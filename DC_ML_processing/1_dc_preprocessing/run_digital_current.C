// run_digital_current.C
#include "TROOT.h"
#include "TSystem.h"
#include "TString.h"
#include "TStopwatch.h"
#include <cstdio>
#include "T_DigitalCurrent.C"
#include "InfoTPC.h"

// Run like:
//   root -l -b -q 'run_digital_current.C()'
// Or with args:
//   root -l -b -q 'run_digital_current.C("T_DigitalCurrent.C","dc_debug_trees/run72440",0,9,0,1,true)'
void run_digital_current(const char* macro    = "T_DigitalCurrent.C",
                         const char* inputDir = "",
                         int iStart = 0, int iEnd = 23,
                         int jStart = 0, int jEnd = 1,
                         bool compileMacro = true)
{
  std::cout<<"Running digital current processing!"<<std::endl;
  TStopwatch sw; sw.Start();

  // Compile / load the analysis macro once
  /*
  TString loadCmd = compileMacro
                    ? Form(".L %s+", macro)
                    : Form(".L %s",  macro);
  if (gROOT->ProcessLine(loadCmd) != 0) {
    ::Error("run_digital_current", "Failed to load: %s", loadCmd.Data());
    return;
  }
  */

  double *z_bins = new double[2 * TPC::nz + 1]; 
  for (int z = 0; z <= 2 * TPC::nz; z++) {
    z_bins[z] = -TPC::z_rdo + TPC::z_rdo / TPC::nz * z;
  }
  std::vector<TH3D*> *histsOut = new std::vector<TH3D*>();

  // Process all (i, j)

  TFile *output_file = new TFile("dc_r_phi_tpcvolume.root", "RECREATE");

  double max_start_time = 0.0;
  // Need to do a first pass of all the trees and get a common start time. Takes a bit of extra time but the overhead is low for processing many events at once.
  for (int i = iStart; i <= iEnd; ++i) {
    for (int j = jStart; j <= jEnd; ++j) {

      // Matches ebdc00..ebdc09 for i=0..9, and ebdc10.. for >=10 (like %02d)
      TString file = Form("%s/DST_STREAMING_EVENT_ebdc%02d_%d_run3auau_physics_new_nocdbtag-00072440-00000_DigitalCurrentDebugTTree.root", inputDir, i, j);

      // Skip if missing to avoid hard errors
      if (gSystem->AccessPathName(file)) {
        printf("Skipping missing file for start time calculation: %s\n", file.Data());
        continue;
      }
    
      T_DigitalCurrent *t = new T_DigitalCurrent(file.Data(),i);
      double time = t->GetStartTime();
      delete t;
      if(time > max_start_time)
        max_start_time = time;
    }
  }
  printf("Found common start gtm_bco of %11.0lf\n",max_start_time);

  // Now, we loop over the actual trees
  for (int i = iStart; i <= iEnd; ++i) {
    for (int j = jStart; j <= jEnd; ++j) {

      // Matches ebdc00..ebdc09 for i=0..9, and ebdc10.. for >=10 (like %02d)
      TString file = Form("%s/DST_STREAMING_EVENT_ebdc%02d_%d_run3auau_physics_new_nocdbtag-00072440-00000_DigitalCurrentDebugTTree.root", inputDir, i, j);

      // Skip if missing to avoid hard errors
      if (gSystem->AccessPathName(file)) {
        printf("Skipping missing file: %s\n", file.Data());
        continue;
      }

      printf("Processing: %s\n", file.Data());
    
      T_DigitalCurrent *t = new T_DigitalCurrent(file.Data(),i);

      if(histsOut->size() == 0) { // Need to figure out how many histograms to make
        int nTpcVols = t->GetUniqueGTMBCOs()/80; // The number of unique gtm_bco's sets the total number of TPC volumes we need to parse.
        for(int n = 0; n < nTpcVols+1; n++) { // Make a histogram for each one.
          output_file->cd(); 
          histsOut->push_back(new TH3D(TString::Format("h_dc_phi_r_z_current_tpcVol%d", n).Data(), TString::Format("h_dc_phi_r_z_current_tpcVol%d", n).Data(), TPC::nphi, TPC::phi_bins, TPC::nr, TPC::r_bins, 2*TPC::nz, z_bins));
          histsOut->at(n)->Write(); // This piece is VERY important. Unless you write the histogram out when you make it, ROOT will delete it from the data buffer and crash when you switch input files.
        }
        std::cout<<"Initialized readout histograms for "<<nTpcVols<<" TPC volumes"<<std::endl;
      }
      histsOut = t->Loop(histsOut, max_start_time); // Fill the histogram
      delete t; // Re-claim memory for this ebdc processing object
    }
  }

  output_file->cd();
  for(unsigned int i = 0; i < histsOut->size(); i++)
    histsOut->at(i)->Write(); // Make sure final versions of all histograms are written

  output_file->Close();

  sw.Stop();
  printf("\nDone.\n");
  sw.Print();
}
