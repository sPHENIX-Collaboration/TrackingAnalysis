"""Small numerical projections of production QA histograms; no fitting."""

import numpy as np


def edges(axis):
    return np.array([axis.GetBinLowEdge(i) for i in range(1, axis.GetNbins() + 2)])


def root_array(hist):
    """ROOT has x as fastest-varying index; return (x,y,z), including flows."""
    if hist.ClassName() not in ("TH3F", "TH3D"):
        raise ValueError(f"Expected TH3F/TH3D, got {hist.ClassName()}")
    shape = (hist.GetNbinsZ() + 2, hist.GetNbinsY() + 2, hist.GetNbinsX() + 2)

    def copy_buffer(storage, dtype):
        # Explicit count avoids the unbounded default length of a PyROOT
        # LowLevelView; no dependence on legacy SetSize methods.
        try:
            flat = np.frombuffer(storage, dtype=dtype, count=hist.GetNcells()).copy()
        except TypeError:
            flat = np.fromiter(
                (storage[i] for i in range(hist.GetNcells())), dtype=dtype, count=hist.GetNcells()
            )
        return flat.reshape(shape).transpose(2, 1, 0)

    dtype = np.float32 if hist.ClassName() == "TH3F" else np.float64
    values = copy_buffer(hist.GetArray(), dtype).astype(float, copy=False)
    if hist.GetSumw2N():
        variance = copy_buffer(hist.GetSumw2().GetArray(), np.float64)
    else:
        variance = np.maximum(values, 0.0)
    if np.any(~np.isfinite(values)) or np.any(values < 0):
        raise ValueError("Nonfinite or negative counts are not supported")
    if np.any(~np.isfinite(variance)) or np.any(variance < 0):
        raise ValueError("Invalid histogram variances")
    return values, variance


def pt_slice(axis_edges, low, high):
    left = np.flatnonzero(np.isclose(axis_edges, low, rtol=0, atol=1e-7))
    right = np.flatnonzero(np.isclose(axis_edges, high, rtol=0, atol=1e-7))
    if len(left) != 1 or len(right) != 1 or right[0] <= left[0]:
        raise ValueError(f"pT {low}:{high} must match stored bin edges {axis_edges}")
    # Indices for arrays INCLUDING underflow/overflow.
    return slice(int(left[0]) + 1, int(right[0]) + 1)


def group_axis(values, variance, coordinate_edges, factor):
    if factor < 1 or values.shape[0] % factor:
        raise ValueError("Coordinate bin count must be divisible by grouping factor")
    shape = (values.shape[0] // factor, factor) + values.shape[1:]
    return (
        values.reshape(shape).sum(axis=1),
        variance.reshape(shape).sum(axis=1),
        coordinate_edges[::factor],
    )


def project_file(root, windows, prefix, phi_group):
    arrays, maps, charges, titles = {}, [], [], []
    for side in ("south", "north"):
        for charge in ("pos", "neg"):
            for axis, factor in (("phi", phi_group),):
                name = f"{prefix}tpc_only_{side}_{charge}_rdca_{axis}_pt"
                h = root.Get(name)
                if not h:
                    raise ValueError(f"Missing required QA histogram: {name}")
                titles.append(
                    (
                        name,
                        str(h.GetTitle()),
                        edges(h.GetXaxis()).tolist(),
                        edges(h.GetYaxis()).tolist(),
                        edges(h.GetZaxis()).tolist(),
                    )
                )
                values, variance = root_array(h)
                coordinate, pt, dca = edges(h.GetXaxis()), edges(h.GetYaxis()), edges(h.GetZaxis())
                for tag, low, high in windows:
                    selected = pt_slice(pt, low, high)
                    projected = values[1:-1, selected, 1:-1].sum(axis=1)
                    errors2 = variance[1:-1, selected, 1:-1].sum(axis=1)
                    projected, errors2, grouped_edges = group_axis(
                        projected, errors2, coordinate, factor
                    )
                    key = f"m{len(maps):03d}"
                    arrays[key + "_counts"] = projected
                    arrays[key + "_variance"] = errors2
                    arrays[key + "_xedges"] = grouped_edges
                    arrays[key + "_dcaedges"] = dca
                    maps.append(
                        {
                            "key": key,
                            "axis": axis,
                            "side": side,
                            "charge": charge,
                            "window": tag,
                            "pt_low": low,
                            "pt_high": high,
                            "source": name,
                            "source_title": str(h.GetTitle()),
                            "x_title": str(h.GetXaxis().GetTitle()),
                            "dca_underflow": float(values[1:-1, selected, 0].sum()),
                            "dca_overflow": float(values[1:-1, selected, -1].sum()),
                        }
                    )
            name = f"{prefix}tpc_only_{side}_{charge}_pt_phi_eta"
            h = root.Get(name)
            if not h:
                raise ValueError(f"Missing required charge histogram: {name}")
            if h:
                titles.append(
                    (
                        name,
                        str(h.GetTitle()),
                        edges(h.GetXaxis()).tolist(),
                        edges(h.GetYaxis()).tolist(),
                        edges(h.GetZaxis()).tolist(),
                    )
                )
                values, variance = root_array(h)
                spectrum = values[1:-1, 1:-1, 1:-1].sum(axis=(0, 1))
                errors2 = variance[1:-1, 1:-1, 1:-1].sum(axis=(0, 1))
                pt = edges(h.GetZaxis())
                for tag, low, high in windows:
                    selected = pt_slice(pt, low, high)
                    charges.append(
                        {
                            "side": side,
                            "charge": charge,
                            "window": tag,
                            "count": float(spectrum[selected.start - 1 : selected.stop - 1].sum()),
                            "variance": float(
                                errors2[selected.start - 1 : selected.stop - 1].sum()
                            ),
                            "acceptance": "stored normal phi/eta bins; eta in [-1.2,1.2]",
                        }
                    )
    return arrays, maps, charges, titles


def charge_balance(records):
    rows = []
    for record in records:
        groups = {}
        for item in record["charge_counts"]:
            groups.setdefault((item["side"], item["window"]), {})[item["charge"]] = item
        for (side, window), charges in sorted(groups.items()):
            p, n = charges.get("pos"), charges.get("neg")
            if p is None or n is None:
                raise ValueError("Missing charge population")
            pos, neg = p["count"], n["count"]
            total = pos + neg
            if min(pos, neg, p["variance"], n["variance"]) < 0:
                raise ValueError("Negative charge counts/variance")
            ratio = pos / neg if neg > 0 else None
            ratio_error = (
                float(np.sqrt(p["variance"] / neg**2 + pos**2 * n["variance"] / neg**4))
                if neg > 0
                else None
            )
            asym = (pos - neg) / total if total > 0 else None
            asym_error = (
                float(2 * np.sqrt(neg**2 * p["variance"] + pos**2 * n["variance"]) / total**2)
                if total > 0
                else None
            )
            rows.append(
                dict(
                    group=record["group"],
                    run=record["run"],
                    side=side,
                    window=window,
                    Npos=pos,
                    Nneg=neg,
                    ratio=ratio,
                    ratio_error=ratio_error,
                    asymmetry=asym,
                    asymmetry_error=asym_error,
                    status="available" if min(pos, neg) > 0 else "empty_charge_population",
                )
            )
    return rows
