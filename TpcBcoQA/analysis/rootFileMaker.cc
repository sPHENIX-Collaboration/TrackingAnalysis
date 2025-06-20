#include <TTree.h>
#include <TFile.h>
#include <TBranch.h>
#include <iostream>
#include "utils.h"
#include <fstream>
#include <vector>
#include <algorithm>
using namespace std;

int main(int argc, const char * argv[]) {
  if (argc != 3){
    std::cerr << "Error need run number argument and ebdc server as arguments" << std::endl;
    return 1;
  }
  std::string runnum = argv[1];
  std::string file_idx = argv[2];
  std::vector<TpcDataEntry> TpcData;
  std::string outdir = outputdirectory;
  std::string filename = outdir.c_str() + std::string("/combined_ebdc") + file_idx.c_str() + std::string("_RunNumber") + runnum.c_str() + std::string(".txt");
  std::cout << "reading file... " << filename.c_str() << std::endl;
  std::ifstream TpcFile(filename.c_str());
  if (!TpcFile.is_open()) {
    std::cerr << "Error: Could not file: " << filename << std::endl;
    return 1; 
  }
  
  TpcDataEntry entry;
  long long clockdiff=0;
  TTree *TpcSortedTree = new TTree("TpcSortedTree", "Sorted");
  TpcSortedTree->Branch("event", &entry.event, "event/I");
  TpcSortedTree->Branch("packet_id", &entry.packet_id, "packet_id/I");
  TpcSortedTree->Branch("tagger", &entry.tagger, "tagger/I");
  TpcSortedTree->Branch("tagger_id", &entry.tagger_id, "tagger_id/I");
  TpcSortedTree->Branch("clock", &entry.clock, "clock/L");
  TpcSortedTree->Branch("clockdiff", &clockdiff, "clockdiff/L");
  TpcSortedTree->Branch("nfees", &entry.nfees, "nfees/I");
  
  long long prevclock=0;

  while (TpcFile >> entry.event >> entry.packet_id >> entry.tagger >> entry.tagger_id >> entry.clock >> entry.nfees) {
    clockdiff = (prevclock!=0) ? entry.clock - prevclock : 0;
    prevclock = entry.clock;
    TpcSortedTree->Fill();
  }
  std::cout << "nentries of TpcData entries : " << TpcSortedTree->GetEntries() << std::endl;

  TFile outputFile(Form("%s/rootfiles/output_run%s_ebdc%s.root",outdir.c_str(),runnum.c_str(),file_idx.c_str()), "RECREATE");
  TTree *outputTree = new TTree("outputTree", "Comparison Results");

  int gl1event;
  long long ref_clock;
  long long ref_clock_diff;
  bool packetfound =false;
  bool isallpacket = false;
  int nFEE = 0;

  outputTree->Branch("event", &gl1event, "event/I");
  outputTree->Branch("clock", &ref_clock, "clock/L");
  outputTree->Branch("clockdiff", &ref_clock_diff, "clockdiff/L");
  outputTree->Branch("ispacket", &packetfound, "ispacket/O");
  outputTree->Branch("nFEE",&nFEE,"nFEE/I");

  std::string refFilename = outdir.c_str() + std::string("/combined_gl1daq_RunNumber") + runnum.c_str() + std::string(".txt");
  std::ifstream refFile(refFilename.c_str());
  if (!refFile.is_open()) {
    std::cerr << "Error: Could not file: " << refFilename << std::endl;
    return 1; 
  }


  long long count=0;
  int lastTargetIndex = 0;
  long long prev_ref_clock = 0;
  while (refFile >> gl1event >> ref_clock) {
    ref_clock_diff = (prev_ref_clock !=0 ) ? ref_clock - prev_ref_clock : 0;
    prev_ref_clock = ref_clock;
    packetfound = false;
    nFEE = 0;

    bool isprint = false;
    if(count %500000==0){
      std::cout << "count : " << count << std::endl;
      isprint=true;
    }

    for (int i = lastTargetIndex; i < TpcSortedTree ->GetEntries(); i++) {
      TpcSortedTree->GetEntry(i);

      if (entry.clock > ref_clock + 5) {
        if(isprint) std::cout << "Tpc bco above the ref. bco " << entry.clock << " - " << ref_clock << " now break"<<std::endl;
        lastTargetIndex = i;
        break;
      }

      if (fabs(entry.clock -ref_clock) <5) {
        packetfound = true;
        nFEE = entry.nfees;
        if(isprint) std::cout << "found all packets! break" << std::endl;
        lastTargetIndex = i;
        break;
      }
    }

    outputTree->Fill();
    count++;
  }

  std::cout << "all done writing out file..." << std::endl;

  if(WRITE_TPC_SORTED) TpcSortedTree->Write();
  outputTree->Write();
  outputFile.Close();

  return 0;
}

