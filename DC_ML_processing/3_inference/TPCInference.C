/**
 * TPC Distortion Prediction using ONNX Runtime in ROOT
 *
 * Compile and run with ROOT:
 *   root -l
 *   .L TPCInference.C+
 *   RunInference("unet3d_tpc.onnx", "input_file.root")
 *
 * Or compile directly:
 *   g++ -o tpc_inference TPCInference.C \
 *       `root-config --cflags --libs` \
 *       -I/path/to/onnxruntime/include \
 *       -L/path/to/onnxruntime/lib \
 *       -lonnxruntime -std=c++17
 */

#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <onnxruntime_cxx_api.h>

#include "TFile.h"
#include "TH3F.h"
#include "TString.h"

// Normalization parameters from training (updated from unet3d_tpc_norm_params.txt)
const float TARGET_MEAN[6] = {
    2.40481482e-03f,  // h_nz
    2.71232845e-03f,  // h_pz
    3.23086157e-02f,  // r_nz
    2.92694028e-02f,  // r_pz
    3.81728797e-03f,  // z_nz
   -3.63350334e-03f   // z_pz
};

const float TARGET_STD[6] = {
    1.11907208e-02f,  // h_nz
    1.24416174e-02f,  // h_pz
    1.07677132e-01f,  // r_nz
    1.16574138e-01f,  // r_pz
    2.51075458e-02f,  // z_nz
    2.69109197e-02f   // z_pz
};

// Dimensions
const int INPUT_D = 205;   // phi bins
const int INPUT_H = 66;    // r bins
const int INPUT_W = 160;   // z bins
const int INPUT_SIZE = INPUT_D * INPUT_H * INPUT_W;

const int OUTPUT_CHANNELS = 6;
const int OUTPUT_D = 42;
const int OUTPUT_H = 28;
const int OUTPUT_W = 42;
const int OUTPUT_SIZE_PER_CHANNEL = OUTPUT_D * OUTPUT_H * OUTPUT_W;
const int OUTPUT_SIZE = OUTPUT_CHANNELS * OUTPUT_SIZE_PER_CHANNEL;


/**
 * Normalize input data (z-score normalization)
 */
void NormalizeInput(std::vector<float>& data) {
    double sum = 0.0;
    for (float val : data) {
        sum += val;
    }
    float mean = sum / data.size();

    double var_sum = 0.0;
    for (float val : data) {
        float diff = val - mean;
        var_sum += diff * diff;
    }
    float std_dev = std::sqrt(var_sum / data.size());

    const float epsilon = 1e-6f;
    for (float& val : data) {
        val = (val - mean) / (std_dev + epsilon);
    }
}

/**
 * Denormalize output predictions
 */
void DenormalizeOutput(std::vector<float>& data) {
    for (int ch = 0; ch < OUTPUT_CHANNELS; ++ch) {
        float mean = TARGET_MEAN[ch];
        float std_dev = TARGET_STD[ch];

        int offset = ch * OUTPUT_SIZE_PER_CHANNEL;
        for (int i = 0; i < OUTPUT_SIZE_PER_CHANNEL; ++i) {
            data[offset + i] = data[offset + i] * std_dev + mean;
        }
    }
}

/**
 * Run inference on charge density data
 */
std::vector<float> Predict(const std::string& model_path, std::vector<float>& dc_values) {
    // Initialize ONNX Runtime
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "TPCInference");
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(4);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

    // Create session
    Ort::Session session(env, model_path.c_str(), session_options);

    // Get input/output names
    Ort::AllocatorWithDefaultOptions allocator;
    auto input_name_ptr = session.GetInputNameAllocated(0, allocator);
    auto output_name_ptr = session.GetOutputNameAllocated(0, allocator);
    std::string input_name = input_name_ptr.get();
    std::string output_name = output_name_ptr.get();

    std::cout << "Model loaded: " << model_path << std::endl;
    std::cout << "  Input: " << input_name << std::endl;
    std::cout << "  Output: " << output_name << std::endl;

    // Normalize input
    NormalizeInput(dc_values);

    // Create input tensor
    std::array<int64_t, 5> input_shape = {1, 1, INPUT_D, INPUT_H, INPUT_W};
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info,
        dc_values.data(),
        dc_values.size(),
        input_shape.data(),
        input_shape.size()
    );

    // Run inference
    const char* input_names[] = {input_name.c_str()};
    const char* output_names[] = {output_name.c_str()};

    auto output_tensors = session.Run(
        Ort::RunOptions{nullptr},
        input_names, &input_tensor, 1,
        output_names, 1
    );

    // Extract output
    float* output_data = output_tensors[0].GetTensorMutableData<float>();
    std::vector<float> predictions(output_data, output_data + OUTPUT_SIZE);

    // Denormalize
    DenormalizeOutput(predictions);

    return predictions;
}

/**
 * Read charge density from ROOT histogram
 */
std::vector<float> ReadFromHistogram(TH3F* hist) {
    std::vector<float> data;
    data.reserve(INPUT_SIZE);

    // Read in order: phi (z-axis), r (y-axis), z (x-axis)
    for (int iz = 1; iz <= hist->GetNbinsZ(); ++iz) {
        for (int iy = 1; iy <= hist->GetNbinsY(); ++iy) {
            for (int ix = 1; ix <= hist->GetNbinsX(); ++ix) {
                data.push_back(hist->GetBinContent(ix, iy, iz));
            }
        }
    }

    return data;
}

/**
 * Save distortion field to ROOT histogram
 */
TH3F* CreateDistortionHistogram(const std::vector<float>& data, int channel,
                                 const char* name, const char* title) {
    TH3F* hist = new TH3F(name, title,
                          OUTPUT_W, 0, OUTPUT_W,  // z bins
                          OUTPUT_H, 0, OUTPUT_H,  // r bins
                          OUTPUT_D, 0, OUTPUT_D); // phi bins

    int offset = channel * OUTPUT_SIZE_PER_CHANNEL;
    int idx = 0;

    for (int iz = 1; iz <= OUTPUT_D; ++iz) {      // phi
        for (int iy = 1; iy <= OUTPUT_H; ++iy) {  // r
            for (int ix = 1; ix <= OUTPUT_W; ++ix) {  // z
                hist->SetBinContent(ix, iy, iz, data[offset + idx]);
                idx++;
            }
        }
    }

    return hist;
}

/**
 * Main inference function
 */
void RunInference(const std::string& model_path,
                  const std::string& input_file = "",
                  const std::string& output_file = "tpc_distortions.root") {

    std::cout << "\n=== TPC Distortion Prediction ===" << std::endl;

    // Read input data
    std::vector<float> dc_values;

    if (!input_file.empty()) {
        std::cout << "Reading input from: " << input_file << std::endl;
        TFile* fin = TFile::Open(input_file.c_str(), "READ");
        if (!fin || fin->IsZombie()) {
            std::cerr << "Error: Cannot open input file" << std::endl;
            return;
        }

        // Try to read histogram (adjust name as needed)
        TH3F* hist = (TH3F*)fin->Get("h_dc_phi_r_z_current_tpcVol0");
        if (!hist) {
            // Try alternative name
            hist = (TH3F*)fin->Get("dc_values");
            if (!hist) {
                std::cerr << "Error: Cannot find histogram 'h_dc_phi_r_z_current_tpcVol0' or 'dc_values'" << std::endl;
                std::cerr << "Available objects:" << std::endl;
                fin->ls();
                fin->Close();
                return;
            }
        }

        dc_values = ReadFromHistogram(hist);
        fin->Close();
    } else {
        // Create dummy data for testing
        std::cout << "Creating dummy input data" << std::endl;
        dc_values.resize(INPUT_SIZE);
        for (int i = 0; i < INPUT_SIZE; ++i) {
            dc_values[i] = (float)rand() / RAND_MAX;
        }
    }

    std::cout << "Input size: " << dc_values.size() << std::endl;

    // Run inference
    std::cout << "\nRunning inference..." << std::endl;
    std::vector<float> predictions = Predict(model_path, dc_values);
    std::cout << "Inference complete!" << std::endl;
    std::cout << "Output size: " << predictions.size() << std::endl;

    // Print statistics
    std::cout << "\nDistortion statistics:" << std::endl;
    const char* channel_names[6] = {
        "h_vals_nz (phi, neg z)",
        "h_vals_pz (phi, pos z)",
        "r_vals_nz (r, neg z)",
        "r_vals_pz (r, pos z)",
        "z_vals_nz (z, neg z)",
        "z_vals_pz (z, pos z)"
    };

    for (int ch = 0; ch < OUTPUT_CHANNELS; ++ch) {
        int offset = ch * OUTPUT_SIZE_PER_CHANNEL;
        auto begin = predictions.begin() + offset;
        auto end = begin + OUTPUT_SIZE_PER_CHANNEL;

        float min_val = *std::min_element(begin, end);
        float max_val = *std::max_element(begin, end);
        float mean_val = std::accumulate(begin, end, 0.0f) / OUTPUT_SIZE_PER_CHANNEL;

        printf("  %s: min=%.6f, max=%.6f, mean=%.6f\n",
               channel_names[ch], min_val, max_val, mean_val);
    }

    // Save to ROOT file
    std::cout << "\nSaving results to: " << output_file << std::endl;
    TFile* fout = TFile::Open(output_file.c_str(), "RECREATE");

    TH3F* h_nz = CreateDistortionHistogram(predictions, 0, "h_vals_nz",
                                           "Phi distortion (negative z);z;r;phi");
    TH3F* h_pz = CreateDistortionHistogram(predictions, 1, "h_vals_pz",
                                           "Phi distortion (positive z);z;r;phi");
    TH3F* r_nz = CreateDistortionHistogram(predictions, 2, "r_vals_nz",
                                           "R distortion (negative z);z;r;phi");
    TH3F* r_pz = CreateDistortionHistogram(predictions, 3, "r_vals_pz",
                                           "R distortion (positive z);z;r;phi");
    TH3F* z_nz = CreateDistortionHistogram(predictions, 4, "z_vals_nz",
                                           "Z distortion (negative z);z;r;phi");
    TH3F* z_pz = CreateDistortionHistogram(predictions, 5, "z_vals_pz",
                                           "Z distortion (positive z);z;r;phi");

    h_nz->Write();
    h_pz->Write();
    r_nz->Write();
    r_pz->Write();
    z_nz->Write();
    z_pz->Write();

    fout->Close();

    std::cout << "\nDone! Results saved to " << output_file << std::endl;
    std::cout << "  6 histograms created: h_vals_nz, h_vals_pz, r_vals_nz, r_vals_pz, z_vals_nz, z_vals_pz" << std::endl;
}

/**
 * Entry point for standalone compilation
 */
#ifndef __CINT__
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <model.onnx> [input.root] [output.root]" << std::endl;
        return 1;
    }

    std::string model_path = argv[1];
    std::string input_file = argc > 2 ? argv[2] : "";
    std::string output_file = argc > 3 ? argv[3] : "tpc_distortions.root";

    RunInference(model_path, input_file, output_file);

    return 0;
}
#endif
