#include <iostream>
#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TGraphErrors.h"
#include "utils.h"
#include <fstream>

using namespace std;

void HistAna(int run = 60110, float ninterval = 0.1, int ntotalentries=-1)
{
  std::ifstream file("../utilsout.config");
  if (!file.is_open()) {
    std::cerr << "Error: Could not utilsout.conif" << std::endl;
    return; 
  }
  std::string outdir;
  std::getline(file, outdir);
  outdir = outdir.substr(outdir.find("=") + 1);

  const int nSuffix = 2;

  TFile* f[nTotalFiles];
  TTree* tree[nTotalFiles];
  int event[nTotalFiles];
  bool ispacket[nTotalFiles];
  int nFEE[nTotalFiles];
  long long clock[nTotalFiles];

  for (int i = 0; i < nFiles; i++) {
    for (int sfx = 0; sfx < nSuffix; sfx++) {
      int index = i * nSuffix + sfx;
      std::string ifile = Form("ebdc%02d_%d", i, sfx);
      f[index] = new TFile(Form("%s/rootfiles/output_run%d_%s.root", outdir.c_str(), run, ifile.c_str()));
      if (!f[index] || f[index]->IsZombie()) {
        std::cout << "file failed to open: " << ifile << std::endl;
        delete f[index];
        f[index] = nullptr;
        continue;
      }
      tree[index] = (TTree*) f[index]->Get("outputTree");
      tree[index] -> SetBranchAddress("event",&event[index]);
      tree[index] -> SetBranchAddress("ispacket",&ispacket[index]);
      tree[index] -> SetBranchAddress("nFEE",&nFEE[index]);
      tree[index] -> SetBranchAddress("clock",&clock[index]);
    }
  }

  TFile* wf = new TFile(Form("histfiles/hist_%d_interval%.2fk_tot%d.root",run,ninterval,ntotalentries),"recreate");
  TH1D* h_eff_all = new TH1D("h_eff_all",";;GL1 tagged efficiency",49,0,49);
  TH1D* h_event = new TH1D("h_event",";;",3,0,3);
  TGraphErrors* g_event[nTotalFiles];
  TGraphErrors* g_event_fee[nTotalFiles];
  for(int i=0; i<nTotalFiles;i++){
    g_event[i] = new TGraphErrors();
    g_event[i]->SetName(Form("g_event_server%d",i));
    g_event_fee[i] = new TGraphErrors();
    g_event_fee[i]->SetName(Form("g_event_fee_server%d",i));
  }
  TGraphErrors* g_event_all = new TGraphErrors();
  g_event_all->SetName("g_eff_event_all");

  TGraphErrors* g_event_fee_all = new TGraphErrors();
  g_event_fee_all->SetName("g_eff_event_fee_all");

  int interval_events = ninterval*1000;
  int interval_fee_events = 100;
  int idx = 0;
  int idxfee = 0;

  int nGL1matched[nTotalFiles] = {0};
  int nGL1matchedAll=0;
  int nTotalPass=0;
  int nFEEAvg[nTotalFiles]={0};

  //int nEntries = 1000000;//tree[0]->GetEntries();
  int nEntries = ntotalentries;
  if(ntotalentries<0) nEntries = tree[0]->GetEntries();
  h_event->SetBinContent(1,nEntries);
  h_event->SetBinContent(2,interval_events);
  std::cout << "nEntries : " << nEntries << std::endl;
  bool fillfeeinfo = true;
  for (int i = 0; i < nEntries; i++) {
    if(i>0){
      if(i % interval_fee_events ==0 && fillfeeinfo){
        int nFEEAvgAll =0;
        for(int k=0; k<nTotalFiles; k++){
          float avg_fee = (float) nFEEAvg[k] / interval_fee_events;
          g_event_fee[k]->SetPoint(idxfee,idxfee+0.5,avg_fee);
          g_event_fee[k]->SetPointError(idxfee,0,0);
          nFEEAvgAll += nFEEAvg[k];
          nFEEAvg[k] = 0;
        }
        float avg_fee_all = (float) nFEEAvgAll / interval_fee_events;
        g_event_fee_all->SetPoint(idxfee, idxfee+0.5, avg_fee_all);
        g_event_fee_all->SetPointError(idxfee,0,0);
        idxfee++;
      }
      if( idxfee > 1000) fillfeeinfo = false;
    }

    if(i % interval_events ==0){
      std::cout << "begin interval : " << i << " / " << nEntries << " (" << (float) i/nEntries*100. << "%)" << std::endl;
      if(i>0){
        for(int k=0; k<nTotalFiles; k++){
          float eff_interval = (float) nGL1matched[k] / interval_events;
          g_event[k]->SetPoint(idx,idx+0.5,eff_interval);
          g_event[k]->SetPointError(idx,0,0);
          nGL1matched[k] = 0;
          /*
          if(k==16 && eff_interval<0.3){
            tree[k]->GetEntry(i-interval_events);
            long long clock1 = clock[k];
            tree[k]->GetEntry(i);
            long long clock2 = clock[k];
            std::cout << "low efficiency for ebdc" << k << " in point " << idx << " eff : " << eff_interval << " in events " << i - interval_events << "/" << clock1 << " - " << i << "/" << clock2 << std::endl;
          }*/
        }
        float eff_interval_all = (float) nGL1matchedAll/interval_events;
        g_event_all->SetPoint(idx, idx+0.5, eff_interval_all);
        g_event_all->SetPointError(idx,0,0);
        nGL1matchedAll=0;
        idx++;
      }
    }

    int nServerMatched=0; 
    for (int j = 0; j < nTotalFiles; j++) {
      tree[j]->GetEntry(i);
      if(ispacket[j]){
        nGL1matched[j]++;
        nServerMatched++;
        nFEEAvg[j] = nFEE[j];
      }
    }
    //std::cout << "ok.. nServerMatched : " << nServerMatched << std::endl;
    if(nServerMatched==nTotalFiles){
      nGL1matchedAll++;
      nTotalPass++;
    }
  } 
  std::cout << "nTotalPass / nTot : " << nTotalPass << "/" << nEntries << " = " << (float) nTotalPass/nEntries*100 << "(%)" << std::endl;

  h_event->SetBinContent(3,nTotalPass);
  wf->cd();
  for(int i=0; i<nTotalFiles; i++){
    g_event[i]->Write();
    g_event_fee[i]->Write();
  }
  g_event_all->Write();
  g_event_fee_all->Write();
  h_event->Write();
}
