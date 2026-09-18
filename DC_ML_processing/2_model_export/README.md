# Step 2: Model Export to ONNX

This directory contains the Python script to export the trained PyTorch model to ONNX format for C++ inference.

## Overview

This step converts the PyTorch model checkpoint (`unet3d_weighted_normalized_ddp.pt`) into an ONNX model that can be used for inference in C++ with ONNX Runtime. This step only needs to be run once after training or when updating the model.

## Files

- **`export_to_onnx.py`** - Python script to export PyTorch model to ONNX
- **`unet3d_tpc.onnx`** - Exported ONNX model (21.6 MB)
- **`unet3d_tpc_norm_params.txt`** - Normalization parameters (target_mean, target_std)

## Requirements

```bash
pip install torch onnx numpy
```

## Usage

### Export from Parent Directory

The script expects the PyTorch checkpoint in the parent directory:

```bash
cd /path/to/sphenix_tpc
python onnx/2_model_export/export_to_onnx.py
```

This will:
1. Load `unet3d_weighted_normalized_ddp.pt` from the parent directory
2. Export to `onnx/2_model_export/unet3d_tpc.onnx`
3. Save normalization parameters to `onnx/2_model_export/unet3d_tpc_norm_params.txt`

### Custom Model Path

To export from a different location:

```bash
python onnx/2_model_export/export_to_onnx.py --model-path /path/to/model.pt
```

### Custom Output Path

To specify output location:

```bash
python onnx/2_model_export/export_to_onnx.py --output-path /path/to/output.onnx
```

## Model Architecture

**UNet3DLarge:**
- **Type:** 3D U-Net encoder-decoder
- **Levels:** 5 (with skip connections)
- **Base channels:** 16 (scales: 16→32→64→128→256)
- **Parameters:** ~21M
- **Model size:** 21.6 MB (ONNX format)

**Input Specification:**
- **Shape:** `[1, 1, 205, 66, 160]`
- **Format:** `[batch_size, channels, φ, r, z]`
- **Data type:** float32
- **Normalization:** Per-sample z-score normalization (applied during inference)

**Output Specification:**
- **Shape:** `[1, 6, 42, 28, 42]`
- **Format:** `[batch_size, channels, φ, r, z]`
- **Channels:**
  1. `h_vals_nz` - φ-direction distortion (negative z)
  2. `h_vals_pz` - φ-direction distortion (positive z)
  3. `r_vals_nz` - r-direction distortion (negative z)
  4. `r_vals_pz` - r-direction distortion (positive z)
  5. `z_vals_nz` - z-direction distortion (negative z)
  6. `z_vals_pz` - z-direction distortion (positive z)
- **Data type:** float32
- **Denormalization:** Required using target_mean and target_std

## Normalization Parameters

The file `unet3d_tpc_norm_params.txt` contains the normalization parameters used during training:

```
Channel 0 (h_vals_nz): target_mean=-9.995e-03, target_std=8.641e-03
Channel 1 (h_vals_pz): target_mean=-8.143e-03, target_std=8.976e-03
Channel 2 (r_vals_nz): target_mean=1.297e+00, target_std=5.887e-01
Channel 3 (r_vals_pz): target_mean=1.097e+00, target_std=4.695e-01
Channel 4 (z_vals_nz): target_mean=-3.978e-02, target_std=3.358e-02
Channel 5 (z_vals_pz): target_mean=2.225e-02, target_std=3.966e-02
```

**Important:** These parameters are automatically loaded by the inference scripts in Step 3 to denormalize predictions.

## ONNX Export Details

The export script performs the following:

1. **Load PyTorch checkpoint:**
   - Loads model architecture (UNet3DLarge)
   - Loads trained weights
   - Extracts normalization parameters

2. **Create dummy input:**
   - Shape: `[1, 1, 205, 66, 160]`
   - Values: Random tensor for tracing

3. **Export to ONNX:**
   - Uses `torch.onnx.export()`
   - Sets opset version 13 (compatible with ONNX Runtime 1.10+)
   - Enables optimization

4. **Save normalization parameters:**
   - Extracts `target_mean` and `target_std` from checkpoint
   - Saves as text file for C++ inference

## Verifying the Export

After export, verify the model:

```python
import onnx

# Load and check model
model = onnx.load("unet3d_tpc.onnx")
onnx.checker.check_model(model)

# Print model info
print(f"Input shape: {model.graph.input[0].type.tensor_type.shape}")
print(f"Output shape: {model.graph.output[0].type.tensor_type.shape}")
```

Expected output:
```
Input shape: [1, 1, 205, 66, 160]
Output shape: [1, 6, 42, 28, 42]
```

## Testing the ONNX Model

Test inference with ONNX Runtime:

```python
import onnxruntime as ort
import numpy as np

# Load model
session = ort.InferenceSession("unet3d_tpc.onnx")

# Create dummy input
dummy_input = np.random.randn(1, 1, 205, 66, 160).astype(np.float32)

# Normalize input (z-score)
mean = dummy_input.mean()
std = dummy_input.std()
dummy_input = (dummy_input - mean) / (std + 1e-6)

# Run inference
outputs = session.run(None, {"input": dummy_input})

print(f"Output shape: {outputs[0].shape}")  # Should be [1, 6, 42, 28, 42]
print(f"Output range: [{outputs[0].min():.4f}, {outputs[0].max():.4f}]")
```

## Model Updates

When you retrain the model with new data:

1. Save the new checkpoint with target normalization parameters:
   ```python
   torch.save({
       'model_state_dict': model.state_dict(),
       'target_mean': target_mean_tensor,  # Shape: [6]
       'target_std': target_std_tensor     # Shape: [6]
   }, 'unet3d_weighted_normalized_ddp.pt')
   ```

2. Re-export to ONNX:
   ```bash
   python onnx/2_model_export/export_to_onnx.py
   ```

3. The new ONNX model will automatically be used by the inference scripts
