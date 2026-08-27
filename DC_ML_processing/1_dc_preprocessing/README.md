# Step 1: Digital Current Preprocessing

This directory contains C++ ROOT scripts to convert raw digital current (DC) data from sPHENIX TPC into 3D charge density volumes ready for ML inference.

## Overview

The preprocessing pipeline reads raw digital current measurements from 24 EBDC (Extended Beam Data Collector) modules and transforms them into a 3D TPC volume representation with coordinates (φ, r, z).

## Files

- **`T_DigitalCurrent.C`** - Main analysis code for DC processing
- **`T_DigitalCurrent.h`** - Header file with class definitions
- **`run_digital_current.C`** - ROOT macro to run DC conversion on multiple files
- **`fee_plot_tpcvolume.C`** - Creates 3D TPC volume histograms from channel data
- **`InfoTPC.h`** - TPC geometry definitions (binning, coordinates)
- **`R1_ChannelMapping.csv`** - Channel mapping for Region 1
- **`R2_ChannelMapping.csv`** - Channel mapping for Region 2
- **`R3_ChannelMapping.csv`** - Channel mapping for Region 3

## Data Flow

```
Raw DC ROOT Files (24 EBDC modules)
        ↓
run_digital_current.C → T_DigitalCurrent.C
        ↓
Channel vs. Frame Index Histograms (TH2D)
        ↓
fee_plot_tpcvolume.C
        ↓
3D TPC Volume (TH3D: φ × r × z)
        ↓
Output: dc_r_phi_tpcvolume.root
```

## Usage in ROOT

### Step 1: Process Raw DC Files

Open ROOT and run the digital current analysis:

```bash
root -l
.L run_digital_current.C
```

Edit the file list in `run_digital_current.C` to point to your raw DC ROOT files:

```cpp
// Example modification in run_digital_current.C
vector<string> files = {
    "/path/to/dc_file_ebdc00.root",
    "/path/to/dc_file_ebdc01.root",
    // ... all 24 EBDC files
};
```

Then run:

```cpp
run_digital_current()
```

**Output:** Creates histograms of channel vs. frame_index for each EBDC module.

### Step 2: Create 3D TPC Volume

After processing the DC files, create the TPC volume:

```bash
root -l
.L fee_plot_tpcvolume.C
```

Run the function with your channel histograms:

```cpp
// Adjust frame indices as needed (default: 80 frames)
fee_plot_tpcvolume()
```

**Output:** `dc_r_phi_tpcvolume.root` containing a TH3D histogram with:
- **X-axis:** φ coordinate (205 bins, 0-2π)
- **Y-axis:** r coordinate (66 bins, 217-759 mm)
- **Z-axis:** z coordinate (160 bins, ±1080 mm)

## TPC Geometry

Defined in `InfoTPC.h`:

```cpp
// Radial bins
static constexpr int n_r_bins = 66;
static constexpr float r_min = 217.83;  // mm
static constexpr float r_max = 759.11;  // mm

// z bins
static constexpr int n_z_bins = 160;
static constexpr float z_min = -1080.0;  // mm
static constexpr float z_max = 1080.0;   // mm

// φ bins
static constexpr int n_phi_bins = 205;
static constexpr float phi_min = 0.0;
static constexpr float phi_max = 2.0 * M_PI;
```

## Channel Mapping

The three CSV files (`R1_ChannelMapping.csv`, etc.) map FEE channels to physical TPC coordinates:

- **Column 1:** Channel ID
- **Column 2:** Pad row
- **Column 3:** Pad column
- **Column 4:** r coordinate (cm)
- **Column 5:** φ coordinate (rad)
- **Column 6:** z coordinate (cm)

These mappings are essential for converting raw channel readings into spatial coordinates.