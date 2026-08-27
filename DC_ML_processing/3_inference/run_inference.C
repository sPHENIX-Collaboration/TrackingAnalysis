/**
 * Run TPC distortion inference from 3_inference directory
 *
 * Usage:
 *   cd onnx/3_inference
 *   root -l run_inference.C
 *
 * Or batch mode:
 *   root -l -b -q run_inference.C
 */

void run_inference() {
    std::cout << "\n";
    std::cout << "====================================================\n";
    std::cout << "  TPC Distortion Prediction - ONNX Inference\n";
    std::cout << "====================================================\n\n";

    // File paths (relative to 3_inference directory)
    const char* model_path = "../2_model_export/unet3d_tpc.onnx";
    const char* input_path = "dc_r_phi_tpcvolume.root";
    const char* output_path = "distortions_output.root";

    std::cout << "Configuration:\n";
    std::cout << "  Model:  " << model_path << "\n";
    std::cout << "  Input:  " << input_path << "\n";
    std::cout << "  Output: " << output_path << "\n\n";

    // Check if files exist
    if (gSystem->AccessPathName(model_path)) {
        std::cerr << "Error: Model file not found: " << model_path << "\n";
        std::cerr << "Please export the model first:\n";
        std::cerr << "  cd ../\n";
        std::cerr << "  python 2_model_export/export_to_onnx.py\n";
        return;
    }

    if (gSystem->AccessPathName(input_path)) {
        std::cerr << "Error: Input file not found: " << input_path << "\n";
        std::cerr << "Please create TPC volume first (see ../1_dc_preprocessing/README.md)\n";
        return;
    }

    // Setup include and library paths for ONNX Runtime
    const char* onnx_dir = gSystem->Getenv("ONNXRUNTIME_DIR");
    if (!onnx_dir) {
        onnx_dir = "../onnxruntime";  // Default location
    }

    TString include_path = Form("-I%s/include", onnx_dir);
    TString lib_path = Form("-L%s/lib -lonnxruntime", onnx_dir);

    gSystem->AddIncludePath(include_path);
    gSystem->AddLinkedLibs(lib_path);

    std::cout << "Using ONNX Runtime from: " << onnx_dir << "\n\n";

    // Load and compile the inference code
    std::cout << "Compiling inference code (this may take ~30 seconds)...\n";
    int compile_result = gROOT->ProcessLine(".L TPCInference.C+");

    if (compile_result != 0) {
        std::cerr << "\nError: Failed to compile TPCInference.C\n";
        std::cerr << "Make sure ONNX Runtime is installed and environment variables are set:\n";
        std::cerr << "  export ONNXRUNTIME_DIR=/path/to/onnxruntime\n";
        std::cerr << "  export LD_LIBRARY_PATH=$ONNXRUNTIME_DIR/lib:$LD_LIBRARY_PATH\n";
        std::cerr << "  export CPLUS_INCLUDE_PATH=$ONNXRUNTIME_DIR/include:$CPLUS_INCLUDE_PATH\n";
        return;
    }

    std::cout << "\nRunning inference...\n";
    std::cout << "----------------------------------------------------\n";

    // Run the inference
    gROOT->ProcessLine(
        Form("RunInference(\"%s\", \"%s\", \"%s\")",
             model_path, input_path, output_path)
    );

    std::cout << "\n====================================================\n";
    std::cout << "  Inference Complete!\n";
    std::cout << "====================================================\n";
    std::cout << "\nOutput saved to: " << output_path << "\n\n";

    std::cout << "To visualize results, in ROOT:\n";
    std::cout << "  TFile* f = TFile::Open(\"" << output_path << "\")\n";
    std::cout << "  r_vals_nz->Draw(\"colz\")\n";
    std::cout << "  h_vals_nz->Draw(\"colz\")\n";
    std::cout << "  z_vals_nz->Draw(\"colz\")\n\n";

    std::cout << "Or open in new ROOT session:\n";
    std::cout << "  root -l " << output_path << "\n\n";
}
