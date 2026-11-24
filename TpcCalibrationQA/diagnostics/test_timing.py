import unittest

import numpy as np
from scipy.special import ndtr
from tpcqa_timing import drift_metrics, time_spectra


class TimingTests(unittest.TestCase):
    def test_rising_and_falling_edges_and_drift_interval(self):
        edges = np.arange(-0.5, 401.0, 1.0)
        x = (edges[:-1] + edges[1:]) / 2
        counts = 20 + 1000 * ndtr((x - 8.5) / 1.2) * ndtr((250.0 - x) / 2.0)
        result = drift_metrics(edges, counts, "cleaned_ADC_unpacked_time")
        self.assertEqual(result["status"], "estimated")
        self.assertAlmostEqual(result["t0_bins"], 8.5, delta=0.15)
        self.assertAlmostEqual(result["cm_bins"], 250.0, delta=0.15)
        self.assertAlmostEqual(
            result["velocity_cm_us"], 102.325 / ((250 - 8.5) * 0.0568), delta=0.01
        )
        shifted = drift_metrics(
            edges + 40, counts, "production_thresholded_raw_sample_counts", raw_shift=40.0
        )
        self.assertAlmostEqual(shifted["velocity_cm_us"], result["velocity_cm_us"])
        self.assertAlmostEqual(shifted["t0_bins"], result["t0_bins"])
        self.assertGreater(result["cm_width_bins"], 0)

    def test_empty_timing_not_a_measured_velocity(self):
        result = drift_metrics(np.arange(401.0), np.zeros(400), "cleaned_ADC_unpacked_time")
        self.assertEqual(result["status"], "timing_edges_unavailable")
        self.assertTrue(np.isnan(result["velocity_cm_us"]))

    def test_physical_sides_and_partial_sector_handling(self):
        class Axis:
            def GetBinLowEdge(self, i):
                return i - 0.5

        class Hist:
            def __init__(self, value):
                self.value = value

            def GetDimension(self):
                return 1

            def GetNbinsX(self):
                return 10

            def GetXaxis(self):
                return Axis()

            def GetBinContent(self, i):
                return self.value

            def GetBinError(self, i):
                return np.sqrt(self.value)

        class File:
            def __init__(self, missing=False):
                self.missing = missing

            def Get(self, name):
                if not name.startswith("h_TpcRawHitQA_nhits_sample_sec"):
                    return None
                sector = int(name.split("_sec")[1].split("_")[0])
                if self.missing and sector == 12:
                    return None
                return Hist(2 if sector >= 12 else 1)

        arrays, info = time_spectra(File())
        np.testing.assert_array_equal(arrays["timing_south_counts"], 72 * np.ones(10))
        np.testing.assert_array_equal(arrays["timing_north_counts"], 36 * np.ones(10))
        arrays, info = time_spectra(File(True))
        self.assertNotIn("timing_south_counts", arrays)
        self.assertEqual(info[0]["status"], "partial_skipped")
        self.assertIn("timing_north_counts", arrays)
