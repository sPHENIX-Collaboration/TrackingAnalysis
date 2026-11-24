import unittest

import numpy as np
from tpcqa_arrays import charge_balance
from tpcqa_conditions import gem_current_keff, rows_from_payload


class Payload:
    def GetUInt64EntryMap(self):
        return {1: {"gbco": 100}, 2: {"gbco": 200}}

    def GetSingleFloatEntryMap(self):
        return {"Fkeffside0": 1.1, "Fkeffside1": 1.2}

    def GetFloatValue(self, channel, name):
        return float(channel)


class SnapshotTests(unittest.TestCase):
    def test_snapshot_scope_sides_and_applied_unknown(self):
        rows, series = rows_from_payload(
            "run3pp/test", 81558, "test_tag", Payload(), Payload(), "conditions.root", "keff.root"
        )
        self.assertEqual([r["side"] for r in rows], ["south", "north"])
        self.assertEqual([r["keff_base"] for r in rows], [1.1, 1.2])
        self.assertEqual(len(series), 4)
        for row in rows:
            self.assertIsNone(row["keff_applied"])
            self.assertFalse(row["historical_applied_verified"])
            self.assertEqual(row["gem_current_uA"], 1.5)
            self.assertEqual(row["conditions_status"], "complete_snapshot")
            self.assertIn("not_selected_QA_segments", row["current_scope"])

    def test_missing_channel_not_silently_averaged(self):
        class Broken(Payload):
            def GetFloatValue(self, channel, name):
                return np.nan if channel == 2 and name == "S_01_R1_G4_IMon" else 2.0

        rows, _ = rows_from_payload("g", 1, "t", Broken(), None, "c", "")
        self.assertEqual(rows[0]["valid_current_samples"], 1)
        self.assertEqual(rows[0]["conditions_status"], "missing_or_partial_current")
        self.assertIsNone(rows[0]["keff_base"])
        self.assertEqual(rows[1]["conditions_status"], "missing_keff")

    def test_duplicate_bco_rejected(self):
        class Duplicate(Payload):
            def GetUInt64EntryMap(self):
                return {1: {"gbco": 100}, 2: {"gbco": 100}}

        with self.assertRaisesRegex(ValueError, "Duplicate"):
            rows_from_payload("g", 1, "t", Duplicate(), None, "c", "")

    def test_charge_balance_unequal_populations_and_empty(self):
        record = dict(
            group="g",
            run=1,
            charge_counts=[
                dict(side="south", window="p", charge="pos", count=300, variance=300),
                dict(side="south", window="p", charge="neg", count=100, variance=100),
            ],
        )
        row = charge_balance([record])[0]
        self.assertEqual(row["ratio"], 3.0)
        self.assertEqual(row["asymmetry"], 0.5)
        self.assertAlmostEqual(row["ratio_error"], np.sqrt(0.12))
        self.assertAlmostEqual(row["asymmetry_error"], np.sqrt(0.75 / 400))
        record["charge_counts"][1]["count"] = 0
        row = charge_balance([record])[0]
        self.assertIsNone(row["ratio"])
        self.assertEqual(row["status"], "empty_charge_population")


class GemFallbackTests(unittest.TestCase):
    def test_recipe_and_explicit_missing_payload_fallback(self):
        self.assertAlmostEqual(gem_current_keff(3.36138, "south"), 1.55)
        self.assertAlmostEqual(gem_current_keff(3.20357, "north"), 1.55)
        rows, _ = rows_from_payload(
            "run3oo/test", 82688, "t", Payload(), None, "conditions", "", "igem"
        )
        for row in rows:
            self.assertIsNone(row["keff_cdb"])
            self.assertIsNone(row["keff_base"])
            self.assertIsNone(row["keff_applied"])
            self.assertAlmostEqual(row["keff_reference"], gem_current_keff(1.5, row["side"]))
            self.assertEqual(row["keff_kind"], "derived_igem_not_verified_applied")
            self.assertFalse(row["historical_applied_verified"])

    def test_present_invalid_payload_never_becomes_fallback(self):
        class Invalid(Payload):
            def GetSingleFloatEntryMap(self):
                return {"Fkeffside0": np.nan, "Fkeffside1": -1.0}

        rows, _ = rows_from_payload(
            "g", 1, "t", Payload(), Invalid(), "conditions", "present.root", "igem"
        )
        for row in rows:
            self.assertIsNone(row["keff_reference"])
            self.assertEqual(row["keff_kind"], "invalid_cdb_payload_no_fallback")
            self.assertIsNotNone(row["keff_from_igem"])

    def test_current_validation_and_opt_in(self):
        for bad in [0.0, -1.0, np.nan, np.inf, None]:
            self.assertIsNone(gem_current_keff(bad, "south"))
        rows, _ = rows_from_payload("g", 1, "t", Payload(), None, "conditions", "")
        self.assertTrue(all(r["keff_reference"] is None for r in rows))

        class Partial(Payload):
            def GetFloatValue(self, channel, name):
                return np.nan if channel == 1 else 1.0

        rows, _ = rows_from_payload("g", 1, "t", Partial(), None, "conditions", "", "igem")
        self.assertTrue(all(r["keff_from_igem"] is None for r in rows))
