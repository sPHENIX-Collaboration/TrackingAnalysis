#include <TFile.h>
#include <TH2D.h>
#include <TString.h>
#include <TCanvas.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <optional>
#include <algorithm>
#include <cmath>
#include <utility>

using std::vector;
using std::string;
using std::optional;
using std::pair;
using std::nullopt;
using std::make_pair;

const int nr = 66;
double r_bins[nr + 1] = {217.83,
  223.83, 229.83, 235.83, 241.83, 247.83, 253.83, 259.83, 265.83, 271.83, 277.83, 283.83, 289.83, 295.83, 301.83, 306.83,
  311.05, 317.92, 323.31, 329.27, 334.63, 340.59, 345.95, 351.91, 357.27, 363.23, 368.59, 374.55, 379.91, 385.87, 391.23, 397.19, 402.49,
  411.53, 421.70, 431.90, 442.11, 452.32, 462.52, 472.73, 482.94, 493.14, 503.35, 513.56, 523.76, 533.97, 544.18, 554.39, 564.59, 574.76,
  583.67, 594.59, 605.57, 616.54, 627.51, 638.48, 649.45, 660.42, 671.39, 682.36, 693.33, 704.30, 715.27, 726.24, 737.21, 748.18, 759.11};

const int nphi = 205;
double phi_bins[nphi + 1] = {
  0., 6.3083 - 2 * M_PI, 6.3401 - 2 * M_PI, 6.372 - 2 * M_PI, 6.4039 - 2 * M_PI, 6.4358 - 2 * M_PI, 6.4676 - 2 * M_PI, 6.4995 - 2 * M_PI, 6.5314 - 2 * M_PI,
  0.2618, 0.2937, 0.3256, 0.3574, 0.3893, 0.4212, 0.453, 0.4849, 0.5168, 0.5487, 0.5805, 0.6124, 0.6443, 0.6762, 0.7081,
  0.7399, 0.7718, 0.7854, 0.8173, 0.8491, 0.881, 0.9129, 0.9448, 0.9767, 1.0085, 1.0404, 1.0723, 1.1041, 1.136, 1.1679,
  1.1998, 1.2317, 1.2635, 1.2954, 1.309, 1.3409, 1.3727, 1.4046, 1.4365, 1.4684, 1.5002, 1.5321, 1.564, 1.5959, 1.6277,
  1.6596, 1.6915, 1.7234, 1.7552, 1.7871, 1.819, 1.8326, 1.8645, 1.8963, 1.9282, 1.9601, 1.992, 2.0238, 2.0557, 2.0876,
  2.1195, 2.1513, 2.1832, 2.2151, 2.247, 2.2788, 2.3107, 2.3426, 2.3562, 2.3881, 2.42, 2.4518, 2.4837, 2.5156, 2.5474,
  2.5793, 2.6112, 2.6431, 2.6749, 2.7068, 2.7387, 2.7706, 2.8024, 2.8343, 2.8662, 2.8798, 2.9117, 2.9436, 2.9754, 3.0073,
  3.0392, 3.0711, 3.1029, 3.1348, 3.1667, 3.1986, 3.2304, 3.2623, 3.2942, 3.326, 3.3579, 3.3898, 3.4034, 3.4353, 3.4671,
  3.499, 3.5309, 3.5628, 3.5946, 3.6265, 3.6584, 3.6903, 3.7221, 3.754, 3.7859, 3.8178, 3.8496, 3.8815, 3.9134, 3.927,
  3.9589, 3.9907, 4.0226, 4.0545, 4.0864, 4.1182, 4.1501, 4.182, 4.2139, 4.2457, 4.2776, 4.3095, 4.3414, 4.3732, 4.4051,
  4.437, 4.4506, 4.4825, 4.5143, 4.5462, 4.5781, 4.61, 4.6418, 4.6737, 4.7056, 4.7375, 4.7693, 4.8012, 4.8331, 4.865,
  4.8968, 4.9287, 4.9606, 4.9742, 5.0061, 5.0379, 5.0698, 5.1017, 5.1336, 5.1654, 5.1973, 5.2292, 5.2611, 5.2929, 5.3248,
  5.3567, 5.3886, 5.4204, 5.4523, 5.4842, 5.4978, 5.5297, 5.5615, 5.5934, 5.6253, 5.6572, 5.689, 5.7209, 5.7528, 5.7847,
  5.8165, 5.8484, 5.8803, 5.9122, 5.944, 5.9759, 6.0078, 6.0214, 6.0533, 6.0851, 6.117, 6.1489, 6.1808, 6.2127, 6.2445,
  6.2764, 2 * M_PI
};

const int nz = 80;
double z_rdo = 1080;

double *z_bins = new double[2 * nz + 1];
for (int z = 0; z <= 2 * nz; z++)
{
  z_bins[z] = -z_rdo + z_rdo / nz * z;
}

// ---------- CSV util ----------
static vector<string> splitCSVLine(const string& line) {
  vector<string> out;
  string cur; bool inQuotes = false;
  for (size_t i = 0; i < line.size(); ++i) {
    char c = line[i];
    if (c == '"') {
      if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') { cur.push_back('"'); ++i; }
      else inQuotes = !inQuotes;
    } else if (c == ',' && !inQuotes) {
      out.push_back(cur); cur.clear();
    } else cur.push_back(c);
  }
  out.push_back(cur);
  return out;
}

// ---------- InputFEE <-> CSV index mapping ----------
static const vector<int> R1 = {2,4,3,13,17,16};                       // csvFeeIndex 0..5 -> inputFEE values
static const vector<int> R2 = {11,12,19,18,0,1,15,14};                // csvFeeIndex 0..7
static const vector<int> R3 = {20,22,21,23,25,24,10,9,8,6,7,5};       // csvFeeIndex 0..11

// helper: reverse map (region, csvFeeIndex) -> inputFEE
static inline int csvToInputFEE(const string& region, int csvIdx) {
  if (region == "R1") { if (csvIdx >= 0 && csvIdx < (int)R1.size()) return R1[csvIdx]; }
  if (region == "R2") { if (csvIdx >= 0 && csvIdx < (int)R2.size()) return R2[csvIdx]; }
  if (region == "R3") { if (csvIdx >= 0 && csvIdx < (int)R3.size()) return R3[csvIdx]; }
  return -1;
}

// ---------- Global lookup: padLookup[inputFEE][channel] = {r, phi} ----------
static vector<vector<optional<pair<double,double>>>> padLookup;
static bool padLookupBuilt = false;

// build once from all three CSVs. csvDir may be "" or "path/to/dir/"
static void buildPadLookup(const string& csvDir = "") {
  if (padLookupBuilt) return;

  const vector<string> regions = {"R1","R2","R3"};
  // We have 26 FEEs (0..25); channels per FEE unknown a priori -> grow as needed
  padLookup.assign(26, {});  // start with 26 FEEs, empty channel vectors

  for (const auto& region : regions) {
    string path = csvDir + region + "_ChannelMapping.csv";
    std::ifstream fin(path);
    if (!fin) {
      std::cerr << "WARNING: cannot open " << path << ", continuing.\n";
      continue;
    }
    string header;
    if (!std::getline(fin, header)) continue;
    auto headers = splitCSVLine(header);

    auto colIndex = [&](const string& name)->int{
      for (int i=0;i<(int)headers.size();++i) if (headers[i]==name) return i;
      std::cerr << "ERROR: column '" << name << "' not found in " << path << "\n";
      return -1;
    };

    int colFee    = colIndex("FEE");
    int colChan   = colIndex("FEE_Chan");
    int colPadR   = colIndex("PadR");
    int colPadPhi = colIndex("PadPhi");
    if (colFee<0 || colChan<0 || colPadR<0 || colPadPhi<0) continue;

    string line;
    while (std::getline(fin, line)) {
      if (line.empty()) continue;
      auto cols = splitCSVLine(line);
      int maxCol = std::max(std::max(colFee,colChan), std::max(colPadR,colPadPhi));
      if ((int)cols.size() <= maxCol) continue;

      // CSV FEE index within that region
      int csvFeeIndex = (int)std::llround(std::stod(cols[colFee]));
      int inputFEE = csvToInputFEE(region, csvFeeIndex);
      if (inputFEE < 0 || inputFEE >= 26) continue;

      int chan   = std::stoi(cols[colChan]);
      double r   = std::stod(cols[colPadR]);
      double phi = std::stod(cols[colPadPhi]);

      if ((int)padLookup[inputFEE].size() <= chan) padLookup[inputFEE].resize(chan+1);
      padLookup[inputFEE][chan] = make_pair(r, phi);
    }
  }

  padLookupBuilt = true;
}

// O(1) lookup using the prebuilt table
static inline optional<pair<double,double>> getPadPositionFast(int inputFEE, int channel) {
  if (!padLookupBuilt) buildPadLookup("");  // build with default dir once
  if (inputFEE < 0 || inputFEE >= (int)padLookup.size()) return nullopt;
  const auto& v = padLookup[inputFEE];
  if (channel < 0 || channel >= (int)v.size()) return nullopt;
  return v[channel];
}

void fee_plot_tpcvolume() {

  // Build lookup ONCE before processing files (change "" to your CSV directory if needed)
  buildPadLookup("");  // e.g. "./mappings/" if files live there

  TH2D *h_dc_phi_r_current_north = new TH2D(Form("h_dc_phi_r_current_north"),"",nphi, phi_bins, nr, r_bins);
  TH2D *h_dc_phi_r_current_south = new TH2D(Form("h_dc_phi_r_current_south"),"",nphi, phi_bins, nr, r_bins);
  TH3D *h_dc_phi_r_z_current = new TH3D(Form("h_dc_phi_r_z_current"),"",nphi, phi_bins, nr, r_bins, 2*nz, z_bins);
  //TH2D *h_dc_x_y_current   = new TH2D(Form("h_dc_x_y_current"),"",100,-750,750,100,-750,750);

  // Retrieve histograms from file
  TH2D *h_channel_gtmbco_current[26];
  TH1D *h_channel_current;

  double pancake_z = 0.;
  const int n_FEEs = 26;
  const int n_z_slices = 80;

  for(int ifile=0; ifile<24; ifile++){
    // Construct filenames
    TString filename0, filename1;
    filename0 = Form("dc_debug_trees/run72440/output_frameindex/output_volume_DST_STREAMING_EVENT_ebdc%02d_0_run3auau_physics_new_nocdbtag-00072440-00000_DigitalCurrentDebugTTree.root",ifile);
    filename1 = Form("dc_debug_trees/run72440/output_frameindex/output_volume_DST_STREAMING_EVENT_ebdc%02d_1_run3auau_physics_new_nocdbtag-00072440-00000_DigitalCurrentDebugTTree.root",ifile);

    // Open ROOT files
    TFile *file0 = TFile::Open(filename0);
    TFile *file1 = TFile::Open(filename1);

    // Precompute phi rotation for this file index
    const double phiShiftBase = (ifile - 12) * M_PI / 6.0;

    //run over 26 FEEs
    for(int i=0; i<n_FEEs; i++){
      // Choose source file per FEE as in your original logic
      if(i<10 || (i>12 && i<16)) h_channel_gtmbco_current[i] = (TH2D*)file1->Get(Form("h_channel_frameindex_FEE%d",i));
      else                       h_channel_gtmbco_current[i] = (TH2D*)file0->Get(Form("h_channel_frameindex_FEE%d",i));

      if (!h_channel_gtmbco_current[i]) continue;

      //run over 80 frame indices
      for(int k=1; k<n_z_slices+1; k++){

        h_channel_current = (TH1D*)h_channel_gtmbco_current[i]->ProjectionX(Form("h_channel_current"),k,k,"");
        if (!h_channel_current) continue;

        const int nchan = h_channel_current->GetNbinsX();
        for(int j=0; j<nchan; j++){
          auto pos = getPadPositionFast(i, j);
          if (!pos) {
            // Uncomment for debugging:
            // std::cout << "No mapping for FEE " << i << " channel " << j << "\n";
            continue;
          }
          double r   = pos->first;
          double phi = pos->second + phiShiftBase;
          if (phi > 2*M_PI) phi -= 2*M_PI;
          if (phi < 0)      phi += 2*M_PI;

          if(ifile<12) pancake_z = k*1075./80.;
          else pancake_z = -1*k*1075./80.;

          //if(ifile<12) h_dc_phi_r_current_north->Fill(phi, r, h_channel_current->GetBinContent(j));
          //else h_dc_phi_r_current_south->Fill(phi, r, h_channel_current->GetBinContent(j));
          h_dc_phi_r_z_current->Fill(phi, r, pancake_z, h_channel_current->GetBinContent(j));
        }
      }
    }

    // Clean up per-file ROOT handles if desired (ROOT will keep them otherwise)
    delete file0; delete file1;
  }

  TFile *output_file = new TFile("dc_r_phi_tpcvolume.root", "RECREATE");
  output_file->cd();
  h_dc_phi_r_z_current->Write();
  output_file->Close();
}
