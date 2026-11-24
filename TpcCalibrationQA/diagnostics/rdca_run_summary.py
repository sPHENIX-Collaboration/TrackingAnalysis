"""Robust run summaries of charge-paired Gaussian location profiles.

The constant of c + a sin(phi) + b cos(phi) summarizes the HALF difference
(positive minus negative)/2 in cm. This is an observable, not a kEff correction.
"""

import numpy as np


def location_fit(phi, odd, error, min_points=8):
    good = np.isfinite(phi) & np.isfinite(odd) & np.isfinite(error) & (error > 0)
    if good.sum() < min_points:
        return None
    x, y, e = phi[good], odd[good], error[good]
    design = np.column_stack([np.ones(len(x)), np.sin(x), np.cos(x)])
    weights = np.ones(len(x))
    for _ in range(25):
        weighted = design * np.sqrt(weights)[:, None] / e[:, None]
        if np.linalg.matrix_rank(weighted) < 3 or np.linalg.cond(weighted) > 1e8:
            return None
        beta = np.linalg.lstsq(weighted, y * np.sqrt(weights) / e, rcond=None)[0]
        residual = (y - design @ beta) / e
        scale = max(1.0, 1.4826 * np.median(np.abs(residual - np.median(residual))))
        updated = np.minimum(1.0, 2.5 * scale / np.maximum(np.abs(residual), 1e-12))
        if np.max(np.abs(updated - weights)) < 1e-5:
            weights = updated
            break
        weights = updated
    weighted = design * np.sqrt(weights)[:, None] / e[:, None]
    beta = np.linalg.lstsq(weighted, y * np.sqrt(weights) / e, rcond=None)[0]
    residual = (y - design @ beta) / e
    reduced = float(np.sum(weights * residual**2) / (len(x) - 3))
    covariance = np.linalg.inv(weighted.T @ weighted) * max(1.0, reduced)
    return dict(
        offset=float(beta[0]),
        sin1=float(beta[1]),
        cos1=float(beta[2]),
        approximate_error=float(np.sqrt(max(0.0, covariance[0, 0]))),
        npoints=len(x),
        downweighted=int(np.count_nonzero(weights < 0.99)),
        reduced_chi2=reduced,
    )


def summarize_profile(phi, odd, error, valid, bootstrap=100, seed=12345):
    phi, odd, error, valid = map(np.asarray, (phi, odd, error, valid))
    mask = valid & np.isfinite(odd) & np.isfinite(error) & (error > 0)
    result = location_fit(phi[mask], odd[mask], error[mask])
    if result is None:
        return dict(status="insufficient_or_degenerate_phi_coverage", npoints=int(mask.sum()))
    result["status"] = "estimated"
    boot = []
    rng = np.random.default_rng(seed)
    x, y, e = phi[mask], odd[mask], error[mask]
    for _ in range(bootstrap):
        index = rng.integers(0, len(x), len(x))
        fitted = location_fit(x[index], y[index], e[index])
        if fitted is not None:
            boot.append(fitted["offset"])
    enough = len(boot) >= max(20, int(0.8 * bootstrap))
    result["bootstrap_success"] = len(boot)
    result["phi_bootstrap_error"] = float(np.std(boot, ddof=1)) if enough else np.nan
    result["phi_bootstrap_low"] = float(np.quantile(boot, 0.16)) if enough else np.nan
    result["phi_bootstrap_high"] = float(np.quantile(boot, 0.84)) if enough else np.nan
    result["odd_half_cm"] = result["offset"]
    result["odd_half_mm"] = 10 * result["offset"]
    result["full_difference_cm"] = 2 * result["offset"]
    result["full_difference_mm"] = 20 * result["offset"]
    return result
