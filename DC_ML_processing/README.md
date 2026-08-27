# sPHENIX TPC Distortions from Digital Current - ONNX Inference Pipeline

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

**Last Updated:** 2026-08-26
