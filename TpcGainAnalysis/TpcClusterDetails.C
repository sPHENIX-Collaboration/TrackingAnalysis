#include "TH1D.h"
#include "TH2D.h"
#include "TCut.h"
#include "TCanvas.h"
#include "TChain.h"
#include "TStyle.h"
#include "TLatex.h"

#include "iostream"

// Run this macro on files produced using:
//     macros/TrackingProduction/Fun4All_FieldOnAllTrackers.C
// which will produce the tracks and run the module TrackResiduals on them
// This macro selects clusters on those tracks having pT > 1 GeV/c
// These are close to MIPs
//
// The input files are loaded into a TChain in lines 36-43, edit these to change input files
//

void TpcClusterDetails()
{
  gStyle->SetOptStat(0);

  int NS = 0;
  TCut nscut[2] = {"clusgz < 0", "clusgz > 0"};
  std::string tpcside[2] = {"South","North"};

  double maxmax = 300.0;
  double maxadc = 1000.0;

  //TCut trcut("m_pt > 1.0 && quality < 30 && m_ntpc > 25");  // for Acts fitted tracks
  TCut trcut("m_pt > 1.0  && m_ntpc > 25");                                       // for converted seeds
  TCut regioncut[3] = {
    "cluslayer > 6 && cluslayer < 23",
    "cluslayer > 22 && cluslayer < 39",
    "cluslayer > 38 && cluslayer < 55"  };

  TChain *ntp = new TChain("residualtree");  // tracks and clusters on tracks
  for(int i=0;i<10;++i)
    {
      char name[500];
      /*
      sprintf(name,"/sphenix/tg/tg01/hf/frawley/data/run45818_seeds/clusters_tracks_45818-%i.root_resid.root",i);
      std::cout << "Add file " << name << std::endl;
      ntp->Add(name);
     */
      sprintf(name,"/sphenix/tg/tg01/hf/frawley/data/run45819_seeds/clusters_tracks_45819-%i.root_resid.root",i);
      std::cout << "Add file " << name << std::endl;
      ntp->Add(name); 

      /*
      sprintf(name,"/sphenix/tg/tg01/hf/frawley/data/run45749_seeds/clusters_tracks_45749-%i.root_resid.root",i);
      std::cout << "Add file " << name << std::endl;
      ntp->Add(name);
      */
     	/*
      sprintf(name,"/sphenix/tg/tg01/hf/frawley/data/run41989_tracks/clusters_tracks_41989-%i.root_resid.root",i);
      std::cout << "Add file " << name << std::endl;
      ntp->Add(name);
     	*/
    }

  // Cluster phi size
  //============
  
  TCanvas *c2 = new TCanvas("c2","",50,50,1200,800);
  c2->Divide(3,2);

  c2->cd(1);
  TH1D *hclus1 = new TH1D("hclus1","cluster phisize R1 South", 10,0, 10);
  ntp->Draw("clusphisize>>hclus1", trcut  && nscut[0] && "cluszsize > 1 && cluslayer < 23");
  hclus1->GetXaxis()->SetTitle("cluster phi size R1 South");
  hclus1->DrawCopy();

  c2->cd(4);
  TH1D *hclus21 = new TH1D("hclus21","cluster phisize R1 North", 10,0, 10);
  ntp->Draw("clusphisize>>hclus21", trcut  && nscut[1] && "cluszsize > 1 && cluslayer < 23");
  hclus21->GetXaxis()->SetTitle("cluster phi size R1 North");
  hclus21->DrawCopy();

  c2->cd(2);
  TH1D *hclus2 = new TH1D("hclus2","cluster phisize R2 South", 10,0, 10);
  ntp->Draw("clusphisize>>hclus2", trcut  && nscut[0] && "cluszsize > 1 && cluslayer > 22 && cluslayer < 39");
  hclus2->GetXaxis()->SetTitle("cluster phi size R2");
  hclus2->DrawCopy();

  c2->cd(5);
  TH1D *hclus22 = new TH1D("hclus22","cluster phisize R2 North", 10,0, 10);
  ntp->Draw("clusphisize>>hclus22", trcut  && nscut[1] && "cluszsize > 1 && cluslayer > 22 && cluslayer < 39");
  hclus22->GetXaxis()->SetTitle("cluster phi size R2 North");
  hclus22->DrawCopy();

  c2->cd(3);
  TH1D *hclus3 = new TH1D("hclus3","cluster phisize R3 South", 10,0, 10);
  ntp->Draw("clusphisize>>hclus3", trcut  && nscut[0] && "cluszsize > 1 && cluslayer > 39");
  hclus3->GetXaxis()->SetTitle("cluster phi size R3 South");
  hclus3->DrawCopy();

  c2->cd(6);
  TH1D *hclus23 = new TH1D("hclus23","cluster phisize R3 North", 10,0, 10);
  ntp->Draw("clusphisize>>hclus23", trcut  && nscut[1] && "cluszsize > 1 && cluslayer > 39");
  hclus23->GetXaxis()->SetTitle("cluster phi size R3 North");
  hclus23->DrawCopy();
  
  // Get fraction of clusters by number of pads  
  //=================================  

 double ntot = 0;
 double ntot2 = 0;
 double npads[10] = {0,0,0,0,0,0,0,0,0,0};
 double npads2[10] = {0,0,0,0,0,0,0,0,0,0};

 float frac[10] = {0,0,0,0,0,0,0,0,0,0};
 float frac2[10] = {0,0,0,0,0,0,0,0,0,0};
 std::cout << "Region 1:" << std::endl;
 for (int i=1;i<7;++i)
   {
     npads[i] = (double)  hclus1->GetBinContent(i);
     ntot += npads[i];

     npads2[i] = (double)  hclus21->GetBinContent(i);
     ntot2 += npads2[i];
   }
 for (int i=2;i<7;++i)
   {
     frac[i] = npads[i]/ntot;
     std::cout << " south npads " << i-1 << " fraction  " << frac[i] << std::endl;

     frac2[i] = npads2[i]/ntot2;
     std::cout << " north npads " << i-1 << " fraction  " << frac2[i] << std::endl;

   }
  char region[500];
  sprintf(region,"one pad frac %.3f",frac[2]);
  TLatex *frac1s = new TLatex(0.45,0.5,region);
  c2->cd(1); 
  frac1s->SetNDC();
  frac1s->Draw();

  sprintf(region,"one pad frac %.3f",frac2[2]);
  TLatex *frac1n = new TLatex(0.45,0.5,region);
  c2->cd(4); 
  frac1n->SetNDC();
  frac1n->Draw();

 ntot = 0;
 ntot2 = 0;
 std::cout << "Region 2:" << std::endl;
 for (int i=2;i<7;++i)
   {
     npads[i] = (double)  hclus2->GetBinContent(i);
     ntot += npads[i];

     npads2[i] = (double)  hclus22->GetBinContent(i);
     ntot2 += npads2[i];
   }
  for (int i=2;i<7;++i)
   {
     frac[i] = npads[i]/ntot;
     std::cout << " south npads " << i-1 << " fraction " << frac[i] << std::endl;

     frac2[i] = npads2[i]/ntot2;
     std::cout << " north npads " << i-1 << " fraction " << frac2[i] << std::endl;
   }
  sprintf(region,"one pad frac %.3f",frac[2]);
  TLatex *frac2s = new TLatex(0.45,0.5,region);
  c2->cd(2); 
  frac2s->SetNDC();
  frac2s->Draw();

  sprintf(region,"one pad frac %.3f",frac2[2]);
  TLatex *frac2n = new TLatex(0.45,0.5,region);
  c2->cd(5); 
  frac2n->SetNDC();
  frac2n->Draw();
 
  ntot = 0;
  ntot2 = 0;
  std::cout << "Region 3:" << std::endl;
 for (int i=2;i<7;++i)
   {
     npads[i] = (double)  hclus3->GetBinContent(i);
     ntot += npads[i];

     npads2[i] = (double)  hclus23->GetBinContent(i);
     ntot2 += npads2[i];
   }
  for (int i=2;i<7;++i)
   {
     frac[i] = npads[i]/ntot;
     std::cout << " south npads " << i-1 << " fraction " << frac[i] << std::endl;

     frac2[i] = npads2[i]/ntot2;
     std::cout << " north npads " << i-1 << " fraction " << frac2[i] << std::endl;
   }
  sprintf(region,"one pad frac %.3f",frac[2]);
  TLatex *frac3s = new TLatex(0.45,0.5,region);
  frac3s->SetNDC();
  c2->cd(3); 
  frac3s->Draw();

  sprintf(region,"one pad frac %.3f",frac2[2]);
  TLatex *frac3n = new TLatex(0.45,0.5,region);
  frac3n->SetNDC();
  c2->cd(6); 
  frac3n->Draw();

  // clusMaxAdc
  //==========

  TCanvas *c3 = new TCanvas("c3","",100,100,1200,800);
  c3->Divide(3,2);

  c3->cd(1);
  TH1D *hmax1 = new TH1D("hmax1","cluster max adc R1 South", 1000,0,maxmax);
  ntp->Draw("clusMaxAdc>>hmax1", trcut  && nscut[0] && regioncut[0] && "cluszsize > 1");
  hmax1->GetXaxis()->SetTitle("cluster max adc R1");
  hmax1->DrawCopy();
  double m1 = hmax1->GetMean();

  char mean[500];
  sprintf(mean,"mean %.0f",m1);
  TLatex *lm1 = new TLatex(0.45,0.5,mean);
  lm1->SetNDC();
  lm1->Draw();

  c3->cd(4);
  TH1D *hmax21 = new TH1D("hmax21","cluster max adc R1 North", 1000,0,maxmax);
  ntp->Draw("clusMaxAdc>>hmax21", trcut  && nscut[1] && regioncut[0] && "cluszsize > 1");
  hmax21->GetXaxis()->SetTitle("cluster max adc R1 North");
  hmax21->DrawCopy();
  double m21 = hmax21->GetMean();

  sprintf(mean,"mean %.0f",m21);
  TLatex *lm21 = new TLatex(0.45,0.5,mean);
  lm21->SetNDC();
  lm21->Draw();

  c3->cd(2);
  TH1D *hmax2 = new TH1D("hmax2","cluster max adc R2 South", 1000,0,maxmax);
  ntp->Draw("clusMaxAdc>>hmax2", trcut && nscut[0] && regioncut[1] && "cluszsize > 1");
  hmax2->GetXaxis()->SetTitle("cluster max adc R2");
  hmax2->DrawCopy();
  double m2 = hmax2->GetMean();

  sprintf(mean,"mean %.0f",m2);
  TLatex *lm2 = new TLatex(0.45,0.5,mean);
  lm2->SetNDC();
  lm2->Draw();

  c3->cd(5);
  TH1D *hmax22 = new TH1D("hmax22","cluster max adc R2 North", 1000,0,maxmax);
  ntp->Draw("clusMaxAdc>>hmax22", trcut && nscut[1] && regioncut[1] && "cluszsize > 1");
  hmax22->GetXaxis()->SetTitle("cluster max adc R2 North");
  hmax22->DrawCopy();
  double m22 = hmax22->GetMean();

  sprintf(mean,"mean %.0f",m22);
  TLatex *lm22 = new TLatex(0.45,0.5,mean);
  lm22->SetNDC();
  lm22->Draw();
  
  c3->cd(3);
  TH1D *hmax3 = new TH1D("hmax3","cluster max adc R3 South", 1000,0,maxmax);
  ntp->Draw("clusMaxAdc>>hmax3", trcut && nscut[0] && regioncut[2] && "cluszsize > 1");
  hmax3->GetXaxis()->SetTitle("cluster max adc R3 South");
  hmax3->DrawCopy();
  double m3 = hmax3->GetMean();

  sprintf(mean,"mean %.0f",m3);
  TLatex *lm3 = new TLatex(0.45,0.5,mean);
  lm3->SetNDC();
  lm3->Draw();

  c3->cd(6);
  TH1D *hmax23 = new TH1D("hmax23","cluster max adc R3 North", 1000,0,maxmax);
  ntp->Draw("clusMaxAdc>>hmax23", trcut && nscut[1] && regioncut[2] && "cluszsize > 1");
  hmax23->GetXaxis()->SetTitle("cluster max adc R3 North");
  hmax23->DrawCopy();
  double m23 = hmax23->GetMean();

  sprintf(mean,"mean %.0f",m23);
  TLatex *lm23 = new TLatex(0.45,0.5,mean);
  lm23->SetNDC();
  lm23->Draw();
  
  // Cluster ADC vs phi
  //===============

  TCanvas *c5 = new TCanvas("c5","Integrated clusAdc by phi",200,200,1800,800);
  c5->Divide(3,2);
  
  c5->cd(1);
  gPad->SetLeftMargin(0.12);
  TH2D *hclusphi1 = new TH2D("hclusphi1","Region 1 South",200,-4,4,1000,0,maxadc);
  ntp->Draw("clusAdc:atan2(clusgy,clusgx)>>hclusphi1", trcut && nscut[0] && regioncut[0] && "cluszsize>1 && clusAdc > 10");
  hclusphi1->GetXaxis()->SetTitle("phi (rad)");
  hclusphi1->GetYaxis()->SetTitle("cluster ADC");
  hclusphi1->DrawCopy("colz");  

  c5->cd(4);
  gPad->SetLeftMargin(0.12);
  TH2D *hclusphi21 = new TH2D("hclusphi21","Region 1 North",200,-4,4,1000,0,maxadc);
  ntp->Draw("clusAdc:atan2(clusgy,clusgx)>>hclusphi21", trcut && nscut[1] && regioncut[0] && "cluszsize>1 && clusAdc > 10");
  hclusphi21->GetXaxis()->SetTitle("phi (rad)");
  hclusphi21->GetYaxis()->SetTitle("cluster ADC");
  hclusphi21->DrawCopy("colz");  

  c5->cd(2);
  gPad->SetLeftMargin(0.12);
 TH2D *hclusphi2 = new TH2D("hclusphi2","Region 2 South",200,-4,4,1000,0,maxadc);
  ntp->Draw("clusAdc:atan2(clusgy,clusgx)>>hclusphi2", trcut && nscut[0] && regioncut[1] && "cluszsize>1");
  hclusphi2->GetXaxis()->SetTitle("phi (rad)");
  hclusphi2->GetYaxis()->SetTitle("cluster ADC");
  hclusphi2->DrawCopy("colz");  

  c5->cd(5);
  gPad->SetLeftMargin(0.12);
 TH2D *hclusphi22 = new TH2D("hclusphi22","Region 2 North",200,-4,4,1000,0,maxadc);
  ntp->Draw("clusAdc:atan2(clusgy,clusgx)>>hclusphi22", trcut && nscut[1] && regioncut[1] && "cluszsize>1");
  hclusphi22->GetXaxis()->SetTitle("phi (rad)");
  hclusphi22->GetYaxis()->SetTitle("cluster ADC");
  hclusphi22->DrawCopy("colz");  

  c5->cd(3);
  gPad->SetLeftMargin(0.12);
  TH2D *hclusphi3 = new TH2D("hclusphi3","Region 3 South",200,-4,4,1000,0,maxadc);
  ntp->Draw("clusAdc:atan2(clusgy,clusgx)>>hclusphi3", trcut && nscut[0] && regioncut[2] && "cluszsize>1");
  hclusphi3->GetXaxis()->SetTitle("phi (rad)");
  hclusphi3->GetYaxis()->SetTitle("cluster ADC");
  hclusphi3->DrawCopy("colz");  

  c5->cd(6);
  gPad->SetLeftMargin(0.12);
  TH2D *hclusphi23 = new TH2D("hclusphi23","Region 3 North",200,-4,4,1000,0,maxadc);
  ntp->Draw("clusAdc:atan2(clusgy,clusgx)>>hclusphi23", trcut && nscut[1] && regioncut[2] && "cluszsize>1");
  hclusphi23->GetXaxis()->SetTitle("phi (rad)");
  hclusphi23->GetYaxis()->SetTitle("cluster ADC");
  hclusphi23->DrawCopy("colz");  
}
