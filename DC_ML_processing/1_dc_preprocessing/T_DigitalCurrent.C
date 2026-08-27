#define T_DigitalCurrent_cxx
#include "T_DigitalCurrent.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TRandom.h"
#include "TProfile.h"
#include "TProfile2D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TF1.h"

#include "TH2F.h"
#include "TMath.h"
#include <TNtuple.h>
#include "TChain.h"
#include <TString.h>
#include <TCut.h>
#include "TStopwatch.h"
#include "TEnv.h"
#include "TStopwatch.h"
#include "TObjArray.h"

#include "assert.h"
#include <fstream>
#include "TMath.h"
#include <vector>

int T_DigitalCurrent::GetUniqueGTMBCOs() {
   if (fChain == 0) return 0;

   Long64_t nentries = fChain->GetEntries();
   Long64_t nbytes = 0, nb = 0;

   std::set<ULong64_t> unique_ids;
   // Loop through as many entries as it takes for all 26 FEEs to be active
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
      unique_ids.insert(dc_gtm_bco);
   }
   return unique_ids.size();
}

double T_DigitalCurrent::GetStartTime() {

   if (fChain == 0) return 0.0;

   //define start and end time for run
   //run 72440 hard-coded originally, now loads in values.
   double start_time = 984.659e9;
   double end_time = 984.696e9;

   Long64_t nentries = fChain->GetEntries();
   Long64_t nbytes = 0, nb = 0;

   // Step 1: Load in the starting gtm_bco
   //std::cout<<"First pass: Loading run start and end time from tree info"<<std::endl;
   std::vector<bool> feesActive, feesTotal;
   for(int i = 0; i < 26; i++) {
      feesActive.push_back(false);
      feesTotal.push_back(false);
   }

   int nFeesActive = 0, maxFeesActive = 0;
   bool firstEvent = true;
   ULong64_t last_gtm_bco = 0;
   // Loop through as many entries as it takes for all 26 FEEs to be active
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;
      //std::cout<<"Entry "<<jentry<<std::endl;

      // Check if this is currently the first event by looking for a change in gtm_bco
      if((last_gtm_bco != dc_gtm_bco) && (last_gtm_bco != 0) && firstEvent) { // The gtm_bco has changed and this isnt't the first event
         firstEvent = false;
         if(nFeesActive >= maxFeesActive) {
            start_time = last_gtm_bco-1.0;
            break;
         }
      }
      else
         last_gtm_bco = dc_gtm_bco;

      // Tabulate number of active FEEs
      for(int i = 0; i < 8; i++) { // check 8 samples for activity
         if((dc_current[i] > 0) && !feesActive[dc_fee]) {
            nFeesActive++;
            feesActive[dc_fee] = true;
         }
         if(firstEvent && (!feesTotal[dc_fee])) {
            feesTotal[dc_fee] = true;
            maxFeesActive++;
         }
      }
      if((!firstEvent) && (nFeesActive >= maxFeesActive)) { // All FEEs now active
         start_time = dc_gtm_bco-1.0;
         break;
      }
   }
   //printf("Starting gtm_bco found as: %11.0lf\n", start_time);
   return start_time;
}


std::vector<TH3D*> *T_DigitalCurrent::Loop(std::vector<TH3D*> *histsToFill, double start_time)
{
   int adc = 0;
   double min_adc = 0.;
   double max_adc = 3.e4;

   double end_time = 984.696e9;

   Long64_t nentries = fChain->GetEntries();
   Long64_t nbytes = 0, nb = 0;

   int n_tpc_vol = 0;
   const double phiShiftBase = (ebdc - 12) * M_PI / 6.0;

   int frame_index = 0;
   int start_frame_index = 0;

   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;

      if(jentry%1000000 == 0) cout<<jentry<<" out of "<<nentries<<endl;

      if(dc_data_crc!=dc_calc_crc) continue;
      if(dc_gtm_bco<start_time) continue;
      
      frame_index = int(dc_gtm_bco/9720) - int((start_time+1)/9720); // Get the frame index using the gtm_bco timing info and the starting gtm_bco
      //frame_index = int(dc_gtm_bco/9720) - 101302466; // Removing old hard-coded value

      if(frame_index<start_frame_index) continue; // Exclude frames from before all fees are active
      if(frame_index>start_frame_index+80) { // The start_frame_index has exceeded this TPC volume and we need to reset
         n_tpc_vol++;
         start_frame_index = frame_index;
      }
      if(n_tpc_vol >= histsToFill->size()) { 
         std::cout<<"Abormal termination due to different ebdc readout windows. This can happen if you parse the end of a file and it does not match up with the end of a TPC volume."<<std::endl;
         break; 
      }// Didn't reserve enough space due to mismatch in ebdc readout size. Should terminate.
      frame_index = frame_index-start_frame_index; // Convert the frame index to the index within this integration time

      for(int i=0; i<8; i++) {
         adc = dc_current[i] - 60 * dc_nsamples[i]; // A little confused here but just going with it
         if(adc < min_adc || adc > max_adc) continue; // Cut ADCs outside of the known bounds
         int chan = dc_channel-(7-i);
         
         auto pos = getPadPositionFast(dc_fee, chan);
         if (!pos) {
            // Uncomment for debugging:
            // std::cout << "No mapping for FEE " << i << " channel " << j << "\n";
            continue;
         }
         double r   = pos->first;
         double phi = pos->second + phiShiftBase;
         if (phi > 2*M_PI) phi -= 2*M_PI;
         if (phi < 0)      phi += 2*M_PI;
         
         double pancake_z = (ebdc>=12) ? (frame_index)*1075./80. : -1*(frame_index)*1075./80.;
         histsToFill->at(n_tpc_vol)->Fill(phi, r, pancake_z, adc);
      }
   }
   return histsToFill;
}


// ---------- CSV util ----------
vector<string> T_DigitalCurrent::splitCSVLine(const string& line) {
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

// helper: reverse map (region, csvFeeIndex) -> inputFEE
inline int T_DigitalCurrent::csvToInputFEE(const string& region, int csvIdx) {
  if (region == "R1") { if (csvIdx >= 0 && csvIdx < (int)R1.size()) return R1[csvIdx]; }
  if (region == "R2") { if (csvIdx >= 0 && csvIdx < (int)R2.size()) return R2[csvIdx]; }
  if (region == "R3") { if (csvIdx >= 0 && csvIdx < (int)R3.size()) return R3[csvIdx]; }
  return -1;
}

// ---------- Global lookup: padLookup[inputFEE][channel] = {r, phi} ----------

// build once from all three CSVs. csvDir may be "" or "path/to/dir/"
void T_DigitalCurrent::buildPadLookup(const string& csvDir) {
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
inline optional<pair<double,double>> T_DigitalCurrent::getPadPositionFast(int inputFEE, int channel) {
  if (!padLookupBuilt) buildPadLookup("");  // build with default dir once
  if (inputFEE < 0 || inputFEE >= (int)padLookup.size()) return nullopt;
  const auto& v = padLookup[inputFEE];
  if (channel < 0 || channel >= (int)v.size()) return nullopt;
  return v[channel];
}
