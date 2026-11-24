import unittest

import numpy as np
from rdca_fit import fit_slice, model_counts, paired_components
from tpcqa_arrays import group_axis, pt_slice, root_array


class FitTests(unittest.TestCase):
    def test_narrow_and_wide_binned_gaussians(self):
        rng = np.random.default_rng(123)
        edges = np.linspace(-10, 10, 101)
        for sigma in (0.08, 0.15, 0.3, 0.7, 1.2):
            counts = rng.poisson(model_counts([np.log(10000), 0.37, np.log(sigma), 3.0], edges))
            result = fit_slice(counts, edges, counts)
            self.assertTrue(result["accepted"], result)
            self.assertLess(abs(result["mean_cm"] - 0.37), max(0.02, 5 * result["mean_error_cm"]))
            self.assertLess(abs(result["sigma_cm"] - sigma), 0.12 * sigma)

    def test_empty_and_weighted_inputs_stay_unavailable(self):
        edges = np.linspace(-1, 1, 21)
        empty = fit_slice(np.zeros(20), edges)
        self.assertFalse(empty["accepted"])
        self.assertEqual(empty["reason"], "low_counts")
        counts = model_counts([np.log(1000), 0.0, np.log(0.2), 1.0], edges)
        result = fit_slice(counts, edges, variance=2 * counts)
        self.assertEqual(result["reason"], "weighted_input_not_poisson")

    def test_bad_shapes_are_flagged_and_candidate_is_preserved(self):
        edges = np.linspace(-2, 2, 81)
        counts = model_counts([np.log(10000), -0.3, np.log(0.15), 1.0], edges) + model_counts(
            [np.log(10000), 0.3, np.log(0.15), 1.0], edges
        )
        result = fit_slice(counts, edges, half_width=1.0, max_deviance_ndf=2.0)
        self.assertFalse(result["accepted"])
        self.assertEqual(result["reason"], "poor_fit_quality")
        self.assertIsNotNone(result["theta"])

    def test_charge_sign_and_invalid_bins(self):
        p = [
            dict(accepted=True, mean_cm=0.2, mean_error_cm=0.02),
            dict(accepted=False, mean_cm=0.8, mean_error_cm=0.02),
        ]
        n = [
            dict(accepted=True, mean_cm=0.1, mean_error_cm=0.01),
            dict(accepted=True, mean_cm=0.1, mean_error_cm=0.01),
        ]
        result = paired_components(p, n)
        self.assertAlmostEqual(result["odd"][0], 0.05)
        self.assertAlmostEqual(result["even"][0], 0.15)
        self.assertTrue(np.isnan(result["odd"][1]))


class ExportTests(unittest.TestCase):
    def test_variable_pt_edges_and_grouping(self):
        pt = np.array([0.0, 0.2, 0.5, 0.7, 1.0, 1.5, 2.0, 4.0])
        self.assertEqual(pt_slice(pt, 0.5, 1.0), slice(3, 5))
        with self.assertRaises(ValueError):
            pt_slice(pt, 0.6, 1.0)
        counts = np.arange(24 * 10).reshape(24, 10)
        grouped, variance, edges = group_axis(counts, counts, np.linspace(-3, 3, 25), 3)
        self.assertEqual(grouped.shape, (8, 10))
        self.assertEqual(grouped.sum(), counts.sum())
        self.assertEqual(len(edges), 9)

    def test_root_xyz_memory_order_and_sumw2(self):
        class Storage(np.ndarray):
            def SetSize(self, size):
                assert size == self.size

        class FakeHist:
            def __init__(self, dtype, sumw2):
                z, y, x = np.indices((6, 5, 4))
                self.data = (x + 10 * y + 100 * z).astype(dtype).ravel().view(Storage)
                self.var = (self.data * 2).astype(float).view(Storage)
                self.sumw2 = sumw2

            def ClassName(self):
                return "TH3F" if self.data.dtype == np.float32 else "TH3D"

            def GetNbinsX(self):
                return 2

            def GetNbinsY(self):
                return 3

            def GetNbinsZ(self):
                return 4

            def GetNcells(self):
                return 120

            def GetArray(self):
                return self.data

            def GetSumw2N(self):
                return 120 if self.sumw2 else 0

            def GetSumw2(self):
                class Errors:
                    def __init__(self, data):
                        self.data = data

                    def GetArray(self):
                        return self.data

                return Errors(self.var)

        for dtype in (np.float32, np.float64):
            for has_sumw2 in (False, True):
                values, variance = root_array(FakeHist(dtype, has_sumw2))
                self.assertEqual(values.shape, (4, 5, 6))
                self.assertEqual(values[1, 2, 3], 321)
                np.testing.assert_array_equal(variance, values * (2 if has_sumw2 else 1))


if __name__ == "__main__":
    unittest.main()
