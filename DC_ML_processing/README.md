# sPHENIX TPC Distortion Prediction - ONNX Inference Pipeline

Complete C++/ROOT workflow for predicting TPC distortion fields from digital current measurements using machine learning.

## Overview

This pipeline uses a 3D U-Net neural network to predict TPC distortion maps from charge density distributions. The workflow consists of three main steps, all runnable in CERN ROOT:

```
Raw DC Data → [Step 1] → 3D TPC Volumes → [Step 3] → Distortion Maps
                             ↑
                        [Step 2] ONNX Model
```

## Quick Start

```bash
# Setup environment
export ONNXRUNTIME_DIR=/path/to/onnxruntime
export LD_LIBRARY_PATH=$ONNXRUNTIME_DIR/lib:$LD_LIBRARY_PATH
source /path/to/root/bin/thisroot.sh

# Navigate to onnx directory
cd /path/to/sphenix_tpc/onnx

# Step 1: Convert raw DC data to TPC volumes
root -l
.L 1_dc_preprocessing/run_digital_current.C
run_digital_current()  // Edit file list first
.q

root -l
.L 1_dc_preprocessing/fee_plot_tpcvolume.C
fee_plot_tpcvolume()   // Creates dc_r_phi_tpcvolume.root
.q

# Step 2: Export model to ONNX (one-time setup)
cd ..
python onnx/2_model_export/export_to_onnx.py
cd onnx

# Step 3: Run inference
root -l
.L 3_inference/TPCInference.C+
RunInference("2_model_export/unet3d_tpc.onnx", "dc_r_phi_tpcvolume.root", "distortions_output.root")
```

## Three-Step Workflow

### Step 1: Digital Current Preprocessing (C++ ROOT)

**Purpose:** Convert raw digital current data from 24 EBDC modules into 3D TPC charge density volumes.

**Location:** `1_dc_preprocessing/`

**Input:** Raw DC ROOT files from sPHENIX detector
**Output:** `dc_r_phi_tpcvolume.root` (TH3D: 205×66×160)

**Usage:**
```bash
cd 1_dc_preprocessing
root -l
.L run_digital_current.C
.L fee_plot_tpcvolume.C
```

See [1_dc_preprocessing/README.md](1_dc_preprocessing/README.md) for details.

---

### Step 2: Model Export (Python)

**Purpose:** Export trained PyTorch model to ONNX format for C++ inference.

**Location:** `2_model_export/`

**Input:** `unet3d_weighted_normalized_ddp.pt` (trained model)
**Output:**
- `unet3d_tpc.onnx` (21.6 MB)
- `unet3d_tpc_norm_params.txt`

**Usage:**
```bash
cd ..  # Go to sphenix_tpc directory
python onnx/2_model_export/export_to_onnx.py
```

**Note:** This step only needs to be run once, or when updating the model.

See [2_model_export/README.md](2_model_export/README.md) for details.

---

### Step 3: ONNX Inference (C++ ROOT)

**Purpose:** Run ML inference to predict 6-component distortion fields.

**Location:** `3_inference/`

**Input:**
- `dc_r_phi_tpcvolume.root` (from Step 1)
- `unet3d_tpc.onnx` (from Step 2)

**Output:** `distortions_output.root` (6 × TH3F: 42×28×42)

**Usage:**
```bash
cd 3_inference
root -l
.L TPCInference.C+
RunInference("../2_model_export/unet3d_tpc.onnx", "../dc_r_phi_tpcvolume.root", "distortions_output.root")
```

See [3_inference/README.md](3_inference/README.md) for details.

---

## Directory Structure

```
onnx/
├── 1_dc_preprocessing/          # Step 1: Raw DC → TPC volumes
│   ├── T_DigitalCurrent.C       # Core conversion code
│   ├── T_DigitalCurrent.h
│   ├── run_digital_current.C    # Runner script
│   ├── fee_plot_tpcvolume.C     # 3D volume creation
│   ├── InfoTPC.h                # TPC geometry
│   ├── R1_ChannelMapping.csv    # Channel mappings
│   ├── R2_ChannelMapping.csv
│   ├── R3_ChannelMapping.csv
│   └── README.md
│
├── 2_model_export/              # Step 2: PyTorch → ONNX
│   ├── export_to_onnx.py        # Export script
│   ├── unet3d_tpc.onnx          # ONNX model (21.6 MB)
│   ├── unet3d_tpc_norm_params.txt
│   └── README.md
│
├── 3_inference/                 # Step 3: ONNX inference
│   ├── TPCInference.C           # ROOT-compatible inference
│   ├── tpc_inference_example.cpp # Standalone C++ version
│   └── README.md
│
├── onnxruntime/                 # ONNX Runtime library
│   └── README.md
│
├── docs/                        # Additional documentation
│   ├── WORKFLOW_SUMMARY.md
│   └── ONNX_INFERENCE_README.md
│
├── dc_r_phi_tpcvolume.root     # Example TPC volume (633 MB)
└── README.md                    # This file
```

## Data Flow

```
┌─────────────────────────────────────────────────────────────┐
│  Raw Digital Current Files (24 EBDC modules)                 │
│  Format: ROOT TTree with dc_fee, dc_gtm_bco, dc_current[8]  │
└────────────────────┬────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────┐
│  Step 1: DC Preprocessing (C++ ROOT)                         │
│  Scripts: run_digital_current.C, fee_plot_tpcvolume.C       │
└────────────────────┬────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────┐
│  3D TPC Volume: dc_r_phi_tpcvolume.root                      │
│  Format: TH3D (205 × 66 × 160) - φ × r × z                  │
│  Data: ADC-weighted charge density                           │
└────────────────────┬────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────┐
│  Step 3: ONNX Inference (C++ ROOT)              ┌─────────┐ │
│  Script: TPCInference.C                         │ Step 2  │ │
│  • Load TH3D histogram                          │ ONNX    │─┤
│  • Normalize input (z-score)                    │ Model   │ │
│  • Run ONNX inference                           └─────────┘ │
│  • Denormalize predictions                                  │
└────────────────────┬────────────────────────────────────────┘
                     ↓
┌─────────────────────────────────────────────────────────────┐
│  Distortion Maps: distortions_output.root                    │
│  Format: 6 × TH3F (42 × 28 × 42) - φ × r × z                │
│  Channels:                                                   │
│    • h_vals_nz, h_vals_pz  (φ-direction, ±z)                │
│    • r_vals_nz, r_vals_pz  (radial, ±z)                     │
│    • z_vals_nz, z_vals_pz  (z-direction, ±z)                │
└─────────────────────────────────────────────────────────────┘
```

## Model Information

**Architecture:** UNet3DLarge (3D U-Net)
- **Encoder-Decoder:** 5 levels with skip connections
- **Base channels:** 16 (scales: 16→32→64→128→256)
- **Parameters:** ~21M
- **Model size:** 21.6 MB (ONNX)

**Input:**
- Shape: `[1, 1, 205, 66, 160]` (batch, channels, φ, r, z)
- Data: Charge density (dc_values)
- Preprocessing: Per-sample z-score normalization

**Output:**
- Shape: `[1, 6, 42, 28, 42]` (batch, channels, φ, r, z)
- Channels: [h_nz, h_pz, r_nz, r_pz, z_nz, z_pz]
- Postprocessing: Denormalization using target_mean, target_std

**Performance:**
- **Accuracy:** Mean %RMSE = 0.872% (100 test samples)
- **Inference time:** 2-5 seconds per volume (CPU)
- **Memory usage:** ~1 GB during inference

## Prerequisites

### Software Requirements

1. **ROOT 6.x**
   ```bash
   source /path/to/root/bin/thisroot.sh
   ```

2. **ONNX Runtime C++ API** (for Step 3)
   ```bash
   # Download from: https://github.com/microsoft/onnxruntime/releases
   wget https://github.com/microsoft/onnxruntime/releases/download/v1.17.0/onnxruntime-linux-x64-1.17.0.tgz
   tar -xzf onnxruntime-linux-x64-1.17.0.tgz

   export ONNXRUNTIME_DIR=/path/to/onnxruntime-linux-x64-1.17.0
   export LD_LIBRARY_PATH=$ONNXRUNTIME_DIR/lib:$LD_LIBRARY_PATH
   ```

3. **Python 3.x with PyTorch** (for Step 2, one-time)
   ```bash
   pip install torch onnx numpy
   ```

### System Requirements

- **RAM:** 8 GB minimum (16 GB recommended)
- **Disk space:** 10-50 GB depending on data size
- **CPU:** Multi-core recommended (4-8 threads optimal)
- **GPU:** Optional, for accelerated inference (requires CUDA-enabled ONNX Runtime)

## Running in Batch Mode

For non-interactive execution:

```bash
# Step 1: Preprocessing
root -l -b -q '1_dc_preprocessing/run_digital_current.C'
root -l -b -q '1_dc_preprocessing/fee_plot_tpcvolume.C'

# Step 3: Inference
root -l -b -q '3_inference/TPCInference.C+("2_model_export/unet3d_tpc.onnx", "dc_r_phi_tpcvolume.root", "distortions_output.root")'
```

## TPC Geometry

**Coordinate System:**
- **φ (phi/h):** Azimuthal angle (205 bins, 0-2π rad)
- **r:** Radial distance from beam (66 bins, 217-759 mm)
- **z:** Longitudinal position (160 bins, ±1080 mm)

**Distortion Components:**
- **h_vals:** φ-direction distortion (~0.01 cm)
- **r_vals:** Radial distortion (~0.5-1.5 cm, dominant)
- **z_vals:** z-direction distortion (~0.02-0.05 cm)

**Hemispheres:**
- **nz:** Negative z (z < 0)
- **pz:** Positive z (z > 0)

## Performance Benchmarks

| Step | Operation | Time | Output Size |
|------|-----------|------|-------------|
| 1 | DC → TPC volumes (24 EBDCs) | ~5-10 min | 633 MB |
| 2 | PyTorch → ONNX export | ~10 sec | 21.6 MB |
| 3 | ONNX inference (1 volume) | ~2-5 sec | ~50 MB |

## Using Distortion Maps in Reconstruction

After generating distortion maps, integrate with track reconstruction:

```cpp
// Load distortion histograms
TFile* f = TFile::Open("distortions_output.root");
TH3F* r_dist_nz = (TH3F*)f->Get("r_vals_nz");
TH3F* r_dist_pz = (TH3F*)f->Get("r_vals_pz");
// ... load other components ...

// During hit processing
float hemisphere_sign = (z < 0) ? -1.0 : 1.0;
TH3F* r_dist = (z < 0) ? r_dist_nz : r_dist_pz;

// Get distortion at hit position
float dr = r_dist->Interpolate(z, r, phi);

// Apply correction
float corrected_r = measured_r - dr;
```

## Troubleshooting

### Common Issues

1. **ONNX Runtime library not found**
   ```bash
   export LD_LIBRARY_PATH=$ONNXRUNTIME_DIR/lib:$LD_LIBRARY_PATH
   ```

2. **ROOT ACLiC compilation fails**
   ```cpp
   .L script.C++g  // Force recompilation with debug info
   ```

3. **Missing normalization parameters**
   - Ensure `2_model_export/unet3d_tpc_norm_params.txt` exists
   - Re-run Step 2 if needed

4. **Wrong input histogram name**
   - Verify histogram is named `dc_values`
   - Check with: `root -l file.root` then `.ls`

See individual README files in each directory for detailed troubleshooting.

## Validation

Verify your results at each step:

```bash
# After Step 1
root -l dc_r_phi_tpcvolume.root
dc_values->Draw()  # Should show charge distribution

# After Step 3
root -l distortions_output.root
r_vals_nz->Draw("colz")  # Should show smooth radial distortions
r_vals_nz->GetMean()     # Should be ~1.0-1.5 cm
```

## Advanced Usage

### GPU Acceleration

For faster inference with CUDA:

```cpp
// In TPCInference.C, add:
OrtCUDAProviderOptions cuda_options;
cuda_options.device_id = 0;
session_options.AppendExecutionProvider_CUDA(cuda_options);
```

Requires: ONNX Runtime with CUDA support

### Batch Processing

Process multiple TPC volumes in parallel:

```cpp
// Modify input shape
std::vector<int64_t> input_shape = {BATCH_SIZE, 1, 205, 66, 160};
```

### Model Optimization

Quantize model for faster inference:

```bash
python -m onnxruntime.quantization.quantize_dynamic \
    2_model_export/unet3d_tpc.onnx \
    2_model_export/unet3d_tpc_int8.onnx \
    --per_channel
```

## Documentation

| Document | Description |
|----------|-------------|
| `README.md` | This file - workflow overview |
| `1_dc_preprocessing/README.md` | Step 1 detailed guide |
| `2_model_export/README.md` | Step 2 export instructions |
| `3_inference/README.md` | Step 3 inference guide |
| `docs/WORKFLOW_SUMMARY.md` | Complete workflow summary |
| `docs/ONNX_INFERENCE_README.md` | C++ inference reference |

## Project Context

This pipeline is part of the sPHENIX TPC analysis framework:

- **Parent directory:** Contains Python training/validation scripts
- **Training data:** HDF5 files with ground truth distortions
- **Model:** Trained with DDP on GPU cluster
- **Deployment:** C++/ROOT for integration with sPHENIX reconstruction

See `../CLAUDE.md` for complete project documentation.

## Support & Contributing

- **Issues:** Report bugs or request features on project GitLab
- **Questions:** See individual README files for detailed documentation
- **Updates:** Re-export model (Step 2) when training new versions

## References

- **sPHENIX TPC:** https://www.sphenix.bnl.gov/
- **ONNX:** https://onnx.ai/
- **ONNX Runtime:** https://onnxruntime.ai/
- **ROOT:** https://root.cern/

---

**Repository:** `ssh://git@czgitlab.llnl.gov:7999/heavy-ion-physics/sphenix_tpc.git`

**Last Updated:** 2026-08-26
