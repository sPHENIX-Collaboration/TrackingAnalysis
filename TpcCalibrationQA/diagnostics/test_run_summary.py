import unittest

import numpy as np
from rdca_run_summary import summarize_profile


class SummaryTests(unittest.TestCase):
    def test_units_and_robust_offset(self):
        phi = np.linspace(-np.pi, np.pi, 24, endpoint=False)
        odd = 0.06 + 0.12 * np.sin(phi) - 0.03 * np.cos(phi)
        odd[4] += 1.0
        r = summarize_profile(phi, odd, np.full(24, 0.01), np.ones(24, bool), bootstrap=30, seed=77)
        self.assertEqual(r["status"], "estimated")
        self.assertLess(abs(r["odd_half_cm"] - 0.06), 0.005)
        self.assertAlmostEqual(r["full_difference_mm"], 20 * r["odd_half_cm"])
        self.assertAlmostEqual(r["odd_half_mm"], 10 * r["odd_half_cm"])
        self.assertTrue(np.isfinite(r["phi_bootstrap_error"]))
        self.assertGreater(r["downweighted"], 0)

    def test_missing_coverage_stays_unavailable(self):
        phi = np.linspace(-np.pi, np.pi, 24, endpoint=False)
        r = summarize_profile(phi, np.zeros(24), np.ones(24), np.arange(24) < 7)
        self.assertNotEqual(r["status"], "estimated")
        self.assertNotIn("odd_half_cm", r)


if __name__ == "__main__":
    unittest.main()
