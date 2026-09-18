//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Mon Jul 28 14:09:02 2025 by ROOT version 6.34.08
// from TTree T_DigitalCurrent/DigitalCurrent Debug TTree
// found on file: DST_STREAMING_EVENT_ebdc23_1_run3auau_physics_new_nocdbtag-00069414-00000_DigitalCurrentDebugTTree.root
//////////////////////////////////////////////////////////

#ifndef T_DigitalCurrent_h
#define T_DigitalCurrent_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TH2D.h>
#include <TH3D.h>
#include <TString.h>
#include <TCanvas.h>
#include "InfoTPC.h"

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

// Header file for the classes stored in the TTree if any.

class T_DigitalCurrent {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   ULong64_t       dc_gtm_bco;
   UInt_t          dc_bx_timestamp_predicted;
   UShort_t        dc_fee;
   UShort_t        dc_pkt_length;
   UShort_t        dc_channel;
   UShort_t        dc_sampa_address;
   UInt_t          dc_bx_timestamp;
   UInt_t          dc_current[8];
   UInt_t          dc_nsamples[8];
   UShort_t        dc_data_crc;
   UShort_t        dc_calc_crc;

   // List of branches
   TBranch        *b_dc;   //!

   T_DigitalCurrent(const char* input_filename, int thisEbdc);
   virtual ~T_DigitalCurrent();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t    GetEntries();
   virtual Int_t GetUniqueGTMBCOs();
   virtual double GetStartTime();
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual std::vector<TH3D*>     *Loop(std::vector<TH3D*> *histsToFill, double start_time = 984.659e9);
   virtual bool     Notify();
   virtual void     Show(Long64_t entry = -1);
   virtual vector<string> splitCSVLine(const string& line);
   virtual inline int csvToInputFEE(const string& region, int csvIdx);
   virtual void buildPadLookup(const string& csvDir = "");
   virtual inline optional<pair<double,double>> getPadPositionFast(int inputFEE, int channel);

   //int n_x, n_y, n_z;
   //const double *bins_x, *bins_y, *bins_z;

   int ebdc = -1;
   //double *z_bins = NULL;

   // ---------- InputFEE <-> CSV index mapping ----------
   const vector<int> R1 = {2,4,3,13,17,16};                       // csvFeeIndex 0..5 -> inputFEE values
   const vector<int> R2 = {11,12,19,18,0,1,15,14};                // csvFeeIndex 0..7
   const vector<int> R3 = {20,22,21,23,25,24,10,9,8,6,7,5};       // csvFeeIndex 0..11

   vector<vector<optional<pair<double,double>>>> padLookup;
   bool padLookupBuilt = false;
};

#endif

#ifdef T_DigitalCurrent_cxx
T_DigitalCurrent::T_DigitalCurrent(const char* input_filename, int thisEbdc) : fChain(0) 
{
    TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject(input_filename);
    if (!f || !f->IsOpen()) {
        f = new TFile(input_filename);
    }
    TTree *tree = nullptr;
    f->GetObject("T_DigitalCurrent", tree);

    ebdc = thisEbdc;

    Init(tree);
}

T_DigitalCurrent::~T_DigitalCurrent()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t T_DigitalCurrent::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}

Long64_t T_DigitalCurrent::GetEntries()
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntries();
}

Long64_t T_DigitalCurrent::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void T_DigitalCurrent::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("dc", &dc_gtm_bco, &b_dc);
   Notify();
}

bool T_DigitalCurrent::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return true;
}

void T_DigitalCurrent::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t T_DigitalCurrent::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}

#endif // #ifdef T_DigitalCurrent_cxx
