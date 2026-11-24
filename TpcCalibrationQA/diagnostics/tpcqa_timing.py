"""Small timing projections and explicit midpoint-edge QA; no event reconstruction."""

import numpy as np


def time_spectra(root):
    arrays, metadata = {}, []
    for side, index in [("south", 0), ("north", 1)]:
        key = "timing_" + side
        h = root.Get(f"h_TpcDriftVelocityQA_h_adc_time_side{index}")
        source = "cleaned_ADC_unpacked_time"
        names = [f"h_TpcDriftVelocityQA_h_adc_time_side{index}"]
        if not h:
            name = f"h_PHGarfieldDriftVelocityQA_h_adc_tbin_vs_fee_side{index}"
            h2 = root.Get(name)
            if h2:
                if h2.GetDimension() != 2:
                    raise ValueError("Wrong timing histogram dimension")
                h = h2.ProjectionY(key + "_projection", 1, h2.GetNbinsX(), "e")
                h.SetDirectory(0)
                names = [name]
                source = "standalone_cleaned_ADC_unpacked_time"
        if h:
            if h.GetDimension() != 1:
                raise ValueError("Wrong timing histogram dimension")
            histograms = [h]
        else:
            sectors = range(12, 24) if side == "south" else range(12)
            names = [f"h_TpcRawHitQA_nhits_sample_sec{s}_R{r}" for s in sectors for r in range(3)]
            histograms = [root.Get(name) for name in names]
            present = sum(bool(item) for item in histograms)
            if present != len(names):
                metadata.append(
                    dict(
                        side=side,
                        status="absent" if not present else "partial_skipped",
                        present_histograms=present,
                        required_histograms=len(names),
                    )
                )
                continue
            source = "production_thresholded_raw_sample_counts"
        counts = variance = axis = None
        for item in histograms:
            if item.GetDimension() != 1:
                raise ValueError("Wrong raw timing histogram dimension")
            n = item.GetNbinsX()
            bins = np.array([item.GetXaxis().GetBinLowEdge(i) for i in range(1, n + 2)], float)
            values = np.array([item.GetBinContent(i) for i in range(1, n + 1)], float)
            errors = np.array([item.GetBinError(i) ** 2 for i in range(1, n + 1)], float)
            if not np.isfinite(values).all() or np.any(values < 0) or not np.isfinite(errors).all():
                raise ValueError("Invalid timing contents")
            if counts is None:
                counts, variance, axis = values, errors, bins
            else:
                if not np.array_equal(axis, bins):
                    raise ValueError("Mixed timing axes")
                counts += values
                variance += errors
        if source.startswith("standalone") and axis[0] == 0 and np.allclose(np.diff(axis), 1):
            axis = axis - 0.5  # Hist filled at integer bins into [0,1000).
        arrays.update({key + "_counts": counts, key + "_variance": variance, key + "_xedges": axis})
        metadata.append(
            dict(
                side=side,
                status="complete",
                source=source,
                key=key,
                source_histograms=names,
                axis_edges=axis.tolist(),
            )
        )
    return arrays, metadata


def midpoint_edge(x, counts, left, right, search, expected, direction):
    """50% transition between sideband medians, after a three-bin median filter.

    Width is 10–90% transition width, not a statistical error. Require all three
    crossings in the requested direction and physical order; no nearest-bin fallback.
    """
    x, raw = np.asarray(x, float), np.asarray(counts, float)
    invalid = dict(status="no_resolved_edge", midpoint=np.nan, width_10_90=np.nan)
    if len(x) < 5 or len(x) != len(raw) or not np.isfinite(raw).all():
        return invalid
    y = raw.copy()
    y[1:-1] = np.median(np.vstack([raw[:-2], raw[1:-1], raw[2:]]), axis=0)
    masks = [(x >= a) & (x <= b) for a, b in (left, right)]
    if not all(mask.sum() >= 2 for mask in masks):
        return invalid
    lo, hi = [float(np.median(y[mask])) for mask in masks]
    height = hi - lo
    if direction * height <= 0 or abs(height) < 0.01 * max(abs(lo), abs(hi), 1.0):
        return invalid
    crossings = []
    for fraction in (0.1, 0.5, 0.9):
        target = lo + fraction * height
        good = (x[:-1] >= search[0]) & (x[1:] <= search[1]) & (direction * np.diff(y) > 0)
        good &= (y[:-1] - target) * (y[1:] - target) <= 0
        found = x[:-1][good] + (target - y[:-1][good]) * np.diff(x)[good] / np.diff(y)[good]
        if not len(found):
            return invalid
        crossings.append(float(found[np.argmin(abs(found - expected))]))
    if np.any(np.diff(crossings) < 0):
        return invalid
    return dict(
        status="estimated",
        midpoint=crossings[1],
        width_10_90=abs(crossings[2] - crossings[0]),
        x10=crossings[0],
        x90=crossings[2],
        left_level=lo,
        right_level=hi,
    )


def drift_metrics(
    edges,
    counts,
    source,
    raw_shift=40.0,
    time_bin_ns=56.8,
    distance_cm=102.325,
    t0_windows=((2.5, 5.5), (12.0, 16.0), (5.0, 14.0), 8.0),
    cm_windows=((180.0, 225.0), (300.0, 360.0), (210.0, 290.0), 250.0),
):
    if not (time_bin_ns > 0 and distance_cm > 0 and np.isfinite(raw_shift)):
        raise ValueError("Invalid timing coordinate settings")
    shift = raw_shift if source == "production_thresholded_raw_sample_counts" else 0.0
    x = (np.asarray(edges[:-1]) + np.asarray(edges[1:])) / 2 - shift
    t0 = midpoint_edge(x, counts, *t0_windows, 1)
    cm = midpoint_edge(x, counts, *cm_windows, -1)
    valid = t0["status"] == cm["status"] == "estimated" and cm["midpoint"] > t0["midpoint"]
    drift = (cm["midpoint"] - t0["midpoint"]) * time_bin_ns / 1000 if valid else np.nan
    return dict(
        status="estimated" if valid else "timing_edges_unavailable",
        t0_bins=t0["midpoint"],
        cm_bins=cm["midpoint"],
        t0_width_bins=t0["width_10_90"],
        cm_width_bins=cm["width_10_90"],
        drift_time_us=drift,
        velocity_cm_us=distance_cm / drift if valid else np.nan,
        raw_shift_bins=shift,
        distance_cm=distance_cm,
        time_bin_ns=time_bin_ns,
    )
