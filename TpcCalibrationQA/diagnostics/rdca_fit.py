"""Interactive, bin-integrated Gaussian + constant-background slice fits.

Input is counts, not a previously accepted profile. Every attempted fit retains
its status/reason; rejected candidate curves may be displayed for diagnosis.
"""

import numpy as np
from scipy.optimize import least_squares
from scipy.special import ndtr, xlogy


def model_counts(theta, edges):
    log_area, mean, log_sigma, background = theta
    sigma = np.exp(log_sigma)
    return np.exp(log_area) * np.diff(ndtr((edges - mean) / sigma)) + background * np.diff(
        edges
    ) / np.median(np.diff(edges))


def fit_slice(
    counts,
    edges,
    variance=None,
    min_counts=100,
    max_deviance_ndf=10.0,
    half_width=None,
    min_sigma_bins=0.15,
    max_nfev=350,
):
    counts = np.asarray(counts, float)
    edges = np.asarray(edges, float)
    result = dict(
        accepted=False,
        reason="unattempted",
        mean_cm=np.nan,
        mean_error_cm=np.nan,
        sigma_cm=np.nan,
        sigma_error_cm=np.nan,
        counts=float(np.sum(counts)),
        deviance_ndf=np.nan,
        fit_low_cm=np.nan,
        fit_high_cm=np.nan,
        theta=None,
        solver_success=False,
        full_mean_cm=np.nan,
        median_cm=np.nan,
    )
    if (
        len(edges) != len(counts) + 1
        or np.any(np.diff(edges) <= 0)
        or np.any(~np.isfinite(counts))
        or np.any(counts < 0)
    ):
        result["reason"] = "invalid_input"
        return result
    centers = (edges[:-1] + edges[1:]) / 2
    total = counts.sum()
    if total > 0:
        result["full_mean_cm"] = float(np.dot(counts, centers) / total)
        result["median_cm"] = float(np.interp(total / 2, np.cumsum(counts), centers))
    if total < min_counts:
        result["reason"] = "low_counts"
        return result
    width = float(np.median(np.diff(edges)))
    peak = int(np.argmax(counts))
    peak_mean = centers[peak]
    baseline = float(np.quantile(counts, 0.25))
    half_height = baseline + 0.5 * (counts[peak] - baseline)
    left = right = peak
    while left > 0 and counts[left] > half_height:
        left -= 1
    while right < len(counts) - 1 and counts[right] > half_height:
        right += 1
    seed_sigma = max(width, (centers[right] - centers[left]) / 2.355)
    half = float(half_width) if half_width is not None else max(4 * width, 3 * seed_sigma)
    if not np.isfinite(half) or half <= 0:
        raise ValueError("half_width must be positive")
    lo = max(0, int(np.searchsorted(edges, peak_mean - half, side="right")) - 1)
    hi = min(len(counts), int(np.searchsorted(edges, peak_mean + half, side="left")))
    n = counts[lo:hi]
    e = edges[lo : hi + 1]
    result.update(fit_low_cm=float(e[0]), fit_high_cm=float(e[-1]))
    if len(n) < 6:
        result["reason"] = "too_few_fit_bins"
        return result
    if n.sum() < min_counts:
        result["reason"] = "low_core_counts"
        return result
    if variance is not None:
        v = np.asarray(variance, float)
        if v.shape != counts.shape or not np.allclose(v[lo:hi], n, rtol=1e-4, atol=1e-5):
            result["reason"] = "weighted_input_not_poisson"
            return result
    sigma_min = min_sigma_bins * width
    sigma_max = float(e[-1] - e[0])
    lower = np.array([np.log(1e-5), e[0], np.log(sigma_min), 0.0])
    upper = np.array(
        [np.log(max(100 * n.sum(), 1e3)), e[-1], np.log(sigma_max), max(10 * max(n), 1.0)]
    )
    initial = np.array(
        [
            np.log(max(n.sum() - baseline * len(n), 1.0)),
            peak_mean,
            np.log(min(seed_sigma, 0.8 * sigma_max)),
            baseline,
        ]
    )
    initial = np.clip(initial, lower + 1e-8, upper - 1e-8)

    def residual(theta):
        expected = np.maximum(model_counts(theta, e), 1e-12)
        deviance = 2 * (expected - n + xlogy(n, n / expected))
        return np.sign(n - expected) * np.sqrt(np.maximum(deviance, 0.0))

    try:
        fit = least_squares(
            residual,
            initial,
            bounds=(lower, upper),
            x_scale="jac",
            max_nfev=max_nfev,
            ftol=1e-8,
            xtol=1e-8,
            gtol=1e-8,
        )
        theta = fit.x
        expected = np.maximum(model_counts(theta, e), 1e-12)
        ndf = len(n) - 4
        result.update(
            theta=theta,
            solver_success=bool(fit.success),
            mean_cm=float(theta[1]),
            sigma_cm=float(np.exp(theta[2])),
            deviance_ndf=float(np.dot(residual(theta), residual(theta)) / ndf),
        )
        # Expected Poisson Fisher information, including an unconstrained local
        # derivative for a background whose optimum can be exactly zero.
        derivatives = []
        for j in range(4):
            step = 1e-5 * max(1.0, abs(theta[j]))
            plus, minus = theta.copy(), theta.copy()
            plus[j] += step
            minus[j] -= step
            derivatives.append((model_counts(plus, e) - model_counts(minus, e)) / (2 * step))
        jacobian = np.column_stack(derivatives)
        scaled = jacobian / np.sqrt(expected[:, None])
        if np.linalg.matrix_rank(scaled) < 4:
            result["reason"] = "singular_covariance"
            return result
        covariance = np.linalg.inv(scaled.T @ scaled)
        result["mean_error_cm"] = float(np.sqrt(max(0.0, covariance[1, 1])))
        result["sigma_error_cm"] = float(np.exp(theta[2]) * np.sqrt(max(0.0, covariance[2, 2])))
        if not fit.success:
            result["reason"] = "solver_failed"
        elif result["sigma_cm"] <= sigma_min * 1.05:
            result["reason"] = "unresolved_width_at_bound"
        elif min(theta[1] - e[0], e[-1] - theta[1]) <= 0.05 * width:
            result["reason"] = "mean_at_fit_boundary"
        elif not (np.isfinite(result["mean_error_cm"]) and result["mean_error_cm"] > 0):
            result["reason"] = "invalid_mean_error"
        elif result["deviance_ndf"] > max_deviance_ndf:
            result["reason"] = "poor_fit_quality"
        else:
            result.update(accepted=True, reason="accepted")
    except (ValueError, FloatingPointError, np.linalg.LinAlgError) as error:
        result["reason"] = "numerical_failure:" + type(error).__name__
    return result


def fit_map(counts, edges, variance=None, **settings):
    return [
        fit_slice(n, edges, None if variance is None else variance[i], **settings)
        for i, n in enumerate(counts)
    ]


def paired_components(positive, negative):
    if len(positive) != len(negative):
        raise ValueError("Charge maps have different coordinate bins")
    valid = np.array(
        [p["accepted"] and n["accepted"] for p, n in zip(positive, negative, strict=True)]
    )
    p = np.array([r["mean_cm"] for r in positive])
    n = np.array([r["mean_cm"] for r in negative])
    error = 0.5 * np.hypot(
        [r["mean_error_cm"] for r in positive], [r["mean_error_cm"] for r in negative]
    )
    even, odd = 0.5 * (p + n), 0.5 * (p - n)
    even[~valid] = odd[~valid] = error[~valid] = np.nan
    return dict(valid=valid, even=even, odd=odd, error=error)
