# Step 3: ONNX Inference in C++/ROOT

This directory contains C++ scripts to perform ML inference on TPC charge density volumes and produce distortion maps.

## Overview

This step takes the 3D TPC volume from Step 1 and the ONNX model from Step 2 to predict 6-component distortion fields. Multiple implementations are provided:

1. **`run_inference.C`** - Automated wrapper script (recommended - easiest to use)
2. **`TPCInference.C`** - Core inference implementation (can be used manually)

## Files

- **`run_inference.C`** - Automated wrapper that handles compilation and execution
- **`TPCInference.C`** - Complete ROOT macro for ONNX inference

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

### Method 1: Automated Wrapper (Recommended)

The easiest way to run inference is using the `run_inference.C` wrapper script:

```bash
cd 3_inference
root -l run_inference.C
```

Or in batch mode (no interactive prompts):
```bash
root -l -b -q run_inference.C
```

**What the wrapper does automatically:**
-  Checks that required files exist (ONNX model and input data)
-  Sets up ONNX Runtime include and library paths
-  Compiles `TPCInference.C` with optimizations (~30 seconds)
-  Runs inference with default file paths:
  - Model: `../2_model_export/unet3d_tpc.onnx`
  - Input: `dc_r_phi_tpcvolume.root`
  - Output: `distortions_output.root`
-  Displays progress messages and helpful error messages
-  Shows visualization instructions when complete

This is the recommended approach for most users as it handles all setup automatically.

---

### Method 2: Manual Execution

For advanced users who need custom file paths or more control:

```bash
cd 3_inference
root -l
```

Load and compile the script:

```cpp
// Load the inference script
.L TPCInference.C+

// Run inference with custom paths
RunInference("../2_model_export/unet3d_tpc.onnx",
             "../dc_r_phi_tpcvolume.root",
             "distortions_output.root")
```

The `+` flag compiles the script with optimization for better performance.

**Note:** Before using this method, ensure ONNX Runtime paths are set:
```bash
export ONNXRUNTIME_DIR=/path/to/onnxruntime
export LD_LIBRARY_PATH=$ONNXRUNTIME_DIR/lib:$LD_LIBRARY_PATH
export CPLUS_INCLUDE_PATH=$ONNXRUNTIME_DIR/include:$CPLUS_INCLUDE_PATH
```

## Input Format

The inference expects charge density data in one of these formats:

### 1. ROOT TH3F/TH3D Histogram (from Step 1)

The ROOT file should contain a 3D histogram named `dc_values`:

```cpp
// Expected histogram structure
TH3F* dc_values
  - X-axis: φ [205 bins, 0 to 2π rad]
  - Y-axis: r [66 bins, 217 to 759 mm]
  - Z-axis: z [160 bins, -1080 to 1080 mm]
  - Dimension: (205, 66, 160) = 2,164,800 elements
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
