# Step 3: ONNX Inference in C++/ROOT

This directory contains C++ scripts to perform ML inference on TPC charge density volumes and produce distortion maps.

## Overview

This step takes the 3D TPC volume from Step 1 and the ONNX model from Step 2 to predict 6-component distortion fields. Two implementations are provided:

1. **`TPCInference.C`** - ROOT-compatible script (recommended for sPHENIX workflow)
2. **`tpc_inference_example.cpp`** - Standalone C++ example

## Files

- **`TPCInference.C`** - Complete ROOT macro for ONNX inference
- **`tpc_inference_example.cpp`** - Standalone C++ inference example

## Prerequisites

### 1. ONNX Runtime

Download and install ONNX Runtime C++ library:

```bash
# Download from: https://github.com/microsoft/onnxruntime/releases
# For Linux x64 (adjust for your platform):
wget https://github.com/microsoft/onnxruntime/releases/download/v1.17.0/onnxruntime-linux-x64-1.17.0.tgz
tar -xzf onnxruntime-linux-x64-1.17.0.tgz

# Set environment variables
export ONNXRUNTIME_DIR=/path/to/onnxruntime-linux-x64-1.17.0
export LD_LIBRARY_PATH=$ONNXRUNTIME_DIR/lib:$LD_LIBRARY_PATH
export CPLUS_INCLUDE_PATH=$ONNXRUNTIME_DIR/include:$CPLUS_INCLUDE_PATH
```

### 2. ROOT

Ensure ROOT 6.x is installed and configured:

```bash
source /path/to/root/bin/thisroot.sh
```

## Usage with ROOT

### Interactive ROOT Session

```bash
root -l
```

Load and compile the script:

```cpp
// Load the inference script
.L TPCInference.C+

// Run inference with paths relative to onnx directory
RunInference("../2_model_export/unet3d_tpc.onnx",
             "../dc_r_phi_tpcvolume.root",
             "distortions_output.root")
```

The `+` flag compiles the script with optimization for better performance.

## Input Format

The inference expects charge density data in one of these formats:

### 1. ROOT TH3F/TH3D Histogram (from Step 1)

The ROOT file should contain a 3D histogram named `dc_values`:

```cpp
// Expected histogram structure
TH3F* dc_values
  - X-axis: z [160 bins, -1080 to 1080 mm]
  - Y-axis: r [66 bins, 217 to 759 mm]
  - Z-axis: φ [205 bins, 0 to 2π rad]
  - Dimension: (160, 66, 205) = 2,164,800 elements
```

### 2. C++ Vector (for programmatic use)

```cpp
std::vector<float> dc_values(2164800);  // 205×66×160
// Fill with charge density data in order: φ, r, z
```

## Output Format

The inference produces a ROOT file with 6 TH3F histograms representing distortion fields:

| Histogram Name | Description | Units |
|----------------|-------------|-------|
| `h_vals_nz` | φ-direction distortion (z < 0) | cm |
| `h_vals_pz` | φ-direction distortion (z > 0) | cm |
| `r_vals_nz` | Radial distortion (z < 0) | cm |
| `r_vals_pz` | Radial distortion (z > 0) | cm |
| `z_vals_nz` | z-direction distortion (z < 0) | cm |
| `z_vals_pz` | z-direction distortion (z > 0) | cm |

Each histogram has dimensions:
```cpp
TH3F dimensions:
  - X-axis: z [42 bins]
  - Y-axis: r [28 bins]
  - Z-axis: φ [42 bins]
  - Total: 49,392 elements per channel
```

## Processing Pipeline

The inference script automatically handles:

1. **Data Loading:**
   - Reads TH3F histogram from ROOT file
   - Reshapes to model input format `[1, 1, 205, 66, 160]`

2. **Input Normalization:**
   ```cpp
   // Per-sample z-score normalization
   mean = sum(input) / N
   std = sqrt(sum((input - mean)^2) / N)
   input_normalized = (input - mean) / (std + 1e-6)
   ```

3. **ONNX Inference:**
   - Loads ONNX model
   - Creates inference session
   - Runs forward pass
   - Output shape: `[1, 6, 42, 28, 42]`

4. **Output Denormalization:**
   ```cpp
   // Loads normalization parameters from unet3d_tpc_norm_params.txt
   for each channel i:
       distortion[i] = prediction[i] * target_std[i] + target_mean[i]
   ```

5. **Save Results:**
   - Creates 6 TH3F histograms
   - Writes to output ROOT file

## Normalization Parameters

The script automatically loads normalization parameters from:
`../2_model_export/unet3d_tpc_norm_params.txt`

These parameters are essential for correct denormalization:

| Channel | target_mean | target_std | Typical Range |
|---------|-------------|------------|---------------|
| h_vals_nz | -0.00999 | 0.00864 | ±0.01 cm |
| h_vals_pz | -0.00814 | 0.00898 | ±0.01 cm |
| r_vals_nz | 1.297 | 0.589 | 0.5-1.5 cm |
| r_vals_pz | 1.097 | 0.470 | 0.5-1.5 cm |
| z_vals_nz | -0.0398 | 0.0336 | ±0.05 cm |
| z_vals_pz | 0.0223 | 0.0397 | ±0.05 cm |

## Using Inference Results

After generating distortion maps, you can use them in analysis:

```cpp
// Load results
TFile* f = TFile::Open("distortions_output.root");
TH3F* r_dist_nz = (TH3F*)f->Get("r_vals_nz");

// Get distortion at specific point
int z_bin = r_dist_nz->GetXaxis()->FindBin(z_coord);
int r_bin = r_dist_nz->GetYaxis()->FindBin(r_coord);
int phi_bin = r_dist_nz->GetZaxis()->FindBin(phi_coord);

float distortion = r_dist_nz->GetBinContent(z_bin, r_bin, phi_bin);

// Apply correction to measured track position
float corrected_r = measured_r - distortion;
```
