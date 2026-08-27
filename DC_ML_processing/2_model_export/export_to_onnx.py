import torch
import torch.nn as nn
import torch.nn.functional as F
import argparse
import os


# ====================================================
# Model definition (must match training architecture)
# ====================================================
class ConvBlock3D(nn.Module):
    def __init__(self, in_ch, out_ch):
        super().__init__()
        self.block = nn.Sequential(
            nn.Conv3d(in_ch, out_ch, kernel_size=3, padding=1, bias=False),
            nn.BatchNorm3d(out_ch),
            nn.ReLU(inplace=True),
            nn.Conv3d(out_ch, out_ch, kernel_size=3, padding=1, bias=False),
            nn.BatchNorm3d(out_ch),
            nn.ReLU(inplace=True),
        )

    def forward(self, x):
        return self.block(x)


class UNet3DLarge(nn.Module):
    def __init__(self, in_channels=1, out_channels=6, base_channels=16):
        super().__init__()

        c1 = base_channels
        c2 = base_channels * 2
        c3 = base_channels * 4
        c4 = base_channels * 8
        c5 = base_channels * 16

        self.enc1 = ConvBlock3D(in_channels, c1)
        self.pool1 = nn.MaxPool3d(2)

        self.enc2 = ConvBlock3D(c1, c2)
        self.pool2 = nn.MaxPool3d(2)

        self.enc3 = ConvBlock3D(c2, c3)
        self.pool3 = nn.MaxPool3d(2)

        self.enc4 = ConvBlock3D(c3, c4)
        self.pool4 = nn.MaxPool3d(2)

        self.bottleneck = ConvBlock3D(c4, c5)

        self.up4 = nn.ConvTranspose3d(c5, c4, kernel_size=2, stride=2)
        self.dec4 = ConvBlock3D(c4 + c4, c4)

        self.up3 = nn.ConvTranspose3d(c4, c3, kernel_size=2, stride=2)
        self.dec3 = ConvBlock3D(c3 + c3, c3)

        self.up2 = nn.ConvTranspose3d(c3, c2, kernel_size=2, stride=2)
        self.dec2 = ConvBlock3D(c2 + c2, c2)

        self.up1 = nn.ConvTranspose3d(c2, c1, kernel_size=2, stride=2)
        self.dec1 = ConvBlock3D(c1 + c1, c1)

        self.out_conv = nn.Conv3d(c1, out_channels, kernel_size=1)

    def match_size(self, x, ref):
        if x.shape[2:] != ref.shape[2:]:
            x = F.interpolate(
                x,
                size=ref.shape[2:],
                mode="trilinear",
                align_corners=False,
            )
        return x

    def forward(self, x):
        e1 = self.enc1(x)
        p1 = self.pool1(e1)

        e2 = self.enc2(p1)
        p2 = self.pool2(e2)

        e3 = self.enc3(p2)
        p3 = self.pool3(e3)

        e4 = self.enc4(p3)
        p4 = self.pool4(e4)

        b = self.bottleneck(p4)

        u4 = self.up4(b)
        u4 = self.match_size(u4, e4)
        d4 = self.dec4(torch.cat([u4, e4], dim=1))

        u3 = self.up3(d4)
        u3 = self.match_size(u3, e3)
        d3 = self.dec3(torch.cat([u3, e3], dim=1))

        u2 = self.up2(d3)
        u2 = self.match_size(u2, e2)
        d2 = self.dec2(torch.cat([u2, e2], dim=1))

        u1 = self.up1(d2)
        u1 = self.match_size(u1, e1)
        d1 = self.dec1(torch.cat([u1, e1], dim=1))

        out = self.out_conv(d1)

        out = F.interpolate(
            out,
            size=(42, 28, 42),
            mode="trilinear",
            align_corners=False,
        )

        return out


def parse_args():
    parser = argparse.ArgumentParser(
        description="Export PyTorch model to ONNX format for C++ inference"
    )
    parser.add_argument(
        "--model-path",
        type=str,
        default="unet3d_weighted_normalized_ddp.pt",
        help="Path to PyTorch model checkpoint",
    )
    parser.add_argument(
        "--output-path",
        type=str,
        default="unet3d_tpc.onnx",
        help="Output path for ONNX model",
    )
    parser.add_argument(
        "--opset-version",
        type=int,
        default=14,
        help="ONNX opset version (default: 14, compatible with most backends)",
    )
    parser.add_argument(
        "--simplify",
        action="store_true",
        help="Simplify the ONNX model (requires onnx-simplifier: pip install onnx-simplifier)",
    )
    return parser.parse_args()


def main():
    args = parse_args()

    # Check if model file exists
    if not os.path.isfile(args.model_path):
        print(f"Error: Model file not found: {args.model_path}")
        return

    print(f"Loading model from: {args.model_path}")

    # Load checkpoint
    device = torch.device("cpu")  # Use CPU for export
    checkpoint = torch.load(args.model_path, map_location=device, weights_only=False)

    base_channels = checkpoint.get("base_channels", 16)
    print(f"Model base channels: {base_channels}")

    # Get normalization parameters
    target_mean = checkpoint["target_mean"]
    target_std = checkpoint["target_std"]
    print(f"Target mean shape: {target_mean.shape}")
    print(f"Target std shape: {target_std.shape}")

    # Create model
    model = UNet3DLarge(
        in_channels=1,
        out_channels=6,
        base_channels=base_channels,
    )

    # Load weights (strict=False allows missing BatchNorm running stats)
    model.load_state_dict(checkpoint["model_state_dict"], strict=False)
    model.eval()

    print(f"Model loaded successfully")

    # Create dummy input with correct shape
    # Input shape: (batch_size, channels, depth, height, width)
    # For TPC: (1, 1, 205, 66, 160)
    dummy_input = torch.randn(1, 1, 205, 66, 160)

    print(f"\nExporting to ONNX...")
    print(f"Input shape: {dummy_input.shape}")
    print(f"Output path: {args.output_path}")

    # Export to ONNX
    torch.onnx.export(
        model,
        dummy_input,
        args.output_path,
        export_params=True,
        opset_version=args.opset_version,
        do_constant_folding=True,
        input_names=["dc_values"],
        output_names=["distortions"],
        dynamic_axes={
            "dc_values": {0: "batch_size"},
            "distortions": {0: "batch_size"},
        },
        verbose=False,
    )

    print(f"✓ Model exported to: {args.output_path}")

    # Verify the exported model
    try:
        import onnx

        onnx_model = onnx.load(args.output_path)
        onnx.checker.check_model(onnx_model)
        print("✓ ONNX model validation passed")

        # Print model info
        print(f"\nModel Information:")
        print(f"  ONNX opset version: {args.opset_version}")
        print(f"  Input: dc_values, shape: [batch_size, 1, 205, 66, 160]")
        print(f"  Output: distortions, shape: [batch_size, 6, 42, 28, 42]")
        print(f"  Output channels: [h_nz, h_pz, r_nz, r_pz, z_nz, z_pz]")

        # Get file size
        file_size_mb = os.path.getsize(args.output_path) / (1024 * 1024)
        print(f"  File size: {file_size_mb:.1f} MB")

        # Simplify if requested
        if args.simplify:
            try:
                from onnxsim import simplify

                print(f"\nSimplifying ONNX model...")
                onnx_model_simplified, check = simplify(onnx_model)

                if check:
                    simplified_path = args.output_path.replace(".onnx", "_simplified.onnx")
                    onnx.save(onnx_model_simplified, simplified_path)
                    simplified_size_mb = os.path.getsize(simplified_path) / (1024 * 1024)
                    print(f"✓ Simplified model saved to: {simplified_path}")
                    print(f"  Simplified size: {simplified_size_mb:.1f} MB")
                else:
                    print("⚠ Simplification validation failed, using original model")
            except ImportError:
                print("⚠ onnx-simplifier not installed. Install with: pip install onnx-simplifier")
            except Exception as e:
                print(f"⚠ Simplification failed: {e}")

    except ImportError:
        print("⚠ onnx package not installed. Install with: pip install onnx")
        print("  Skipping validation (model still exported)")
    except Exception as e:
        print(f"⚠ Validation warning: {e}")

    # Save normalization parameters as a separate file for C++ code
    norm_params_path = args.output_path.replace(".onnx", "_norm_params.txt")
    with open(norm_params_path, "w") as f:
        f.write("# Normalization parameters for TPC distortion prediction\n")
        f.write("# These values should be used to denormalize the model output\n")
        f.write("# Denormalization: distortion = prediction * target_std + target_mean\n\n")
        f.write("# target_mean (6 values for: h_nz, h_pz, r_nz, r_pz, z_nz, z_pz)\n")
        f.write("target_mean = ")
        f.write(", ".join([f"{v:.8e}" for v in target_mean.flatten().tolist()]))
        f.write("\n\n")
        f.write("# target_std (6 values for: h_nz, h_pz, r_nz, r_pz, z_nz, z_pz)\n")
        f.write("target_std = ")
        f.write(", ".join([f"{v:.8e}" for v in target_std.flatten().tolist()]))
        f.write("\n")

    print(f"\n✓ Normalization parameters saved to: {norm_params_path}")

    print("\n" + "=" * 70)
    print("IMPORTANT NOTES FOR C++ INFERENCE:")
    print("=" * 70)
    print("1. Input preprocessing:")
    print("   - Normalize each input sample: x_norm = (x - x.mean()) / (x.std() + 1e-6)")
    print("")
    print("2. Output postprocessing:")
    print("   - Denormalize predictions using target_mean and target_std")
    print("   - distortion[i] = prediction[i] * target_std[i] + target_mean[i]")
    print("")
    print("3. Input/Output shapes:")
    print("   - Input: [1, 1, 205, 66, 160] (batch, channel, depth, height, width)")
    print("   - Output: [1, 6, 42, 28, 42] (batch, channels, depth, height, width)")
    print("")
    print("4. Output channel order:")
    print("   - Channel 0: h_vals_nz (phi-distortion, negative z)")
    print("   - Channel 1: h_vals_pz (phi-distortion, positive z)")
    print("   - Channel 2: r_vals_nz (r-distortion, negative z)")
    print("   - Channel 3: r_vals_pz (r-distortion, positive z)")
    print("   - Channel 4: z_vals_nz (z-distortion, negative z)")
    print("   - Channel 5: z_vals_pz (z-distortion, positive z)")
    print("=" * 70)


if __name__ == "__main__":
    main()
