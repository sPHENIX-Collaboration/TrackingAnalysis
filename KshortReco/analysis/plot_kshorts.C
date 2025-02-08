#include <TFile.h>
#include <TNtuple.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <TCut.h>
#include <TF1.h>
#include <TChain.h>
#include <TStyle.h>
#include <TLegend.h>
#include <TLine.h>
#include <iostream>
#include <fstream>
#include <string>

void plot_kshorts()
{
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(1);

  TCut crosscut("crossing1 == crossing2");

  TCut ptcut("sqrt(px1*px1+py1*py1) > 0.20 && sqrt(px2*px2+py2*py2) > 0.20");
  
  //TCut paircut("cosThetaReco > 0.88 && (pow(projected_pathlength_x,2)+pow(projected_pathlength_y,2)) > 0.2 && abs(projected_pair_dca) < 0.5");
  TCut paircut("cosThetaReco > 0.88 && (pow(projected_pathlength_x,2)+pow(projected_pathlength_y,2)+pow(projected_pathlength_z,2)) > 0.2 && abs(projected_pair_dca) < 0.5");

  TCut chargecut("charge1 != charge2");

  TCut trcut = paircut+crosscut+ptcut+chargecut;

  // likesign charge1==charge2
  // unlikesign charge1!=charge2
  TChain *ntp_likesign = new TChain("ntp_reco_info");
  TChain *ntp_unlikesign = new TChain("ntp_reco_info");

  //int run = 52911;
  int run = 53744;

  ntp_likesign->Add(Form("/sphenix/u/xyu3/workarea/PhotonConv/KsNew/background_shape/KshortReconstruction/likesign/all%d_tony.root",run));
  ntp_unlikesign->Add(Form("/sphenix/u/xyu3/workarea/PhotonConv/KsNew/background_shape/KshortReconstruction/unlikesign/all%d_tony.root",run));

  TCanvas *c1 = new TCanvas("c1","quality",50,50,800,800);
  c1->Divide(2,2);

  c1->cd(1);
  TH1D *hq = new TH1D("hq","quality",100,0,500);
  ntp_unlikesign->Draw("quality1>>hq", trcut);
  hq->Draw();

  c1->cd(2);
  TH1D *hpdca = new TH1D("hpdca","projected pair DCA",800,-1.0,1.0);
  ntp_unlikesign->Draw("projected_pair_dca>>hpdca", "cosThetaReco > 0.9 && projected_pathlength > 0.5" && crosscut);
  hpdca->Draw();

  c1->cd(3);
  TH1D *hcosth = new TH1D("hcosth","cosThetaReco",100,0.5,1);
  ntp_unlikesign->Draw("cosThetaReco>>hcosth", "projected_pathlength > 0.5" && crosscut);
  hcosth->Draw();

  c1->cd(4);
  TH1D *hpath = new TH1D("hpath","projected path length",100,0,10);
  ntp_unlikesign->Draw("projected_pathlength>>hpath", "cosThetaReco > 0.9" && crosscut);
  hpath->Draw();

  c1->Update();
  c1->SaveAs("figure/plot1_quality.pdf");

  TCanvas *c2 = new TCanvas("c2","mass",50,50,400,600);
  c2->Divide(1,2);

  c2->cd(1);
 
  TH1D *hm = new TH1D("hm","invariant mass",60,0.4, 0.6);
  ntp_unlikesign->Draw("invariant_mass>>hm",trcut);
 
  int binlo = hm->FindBin(0.43);
  int binhi = hm->FindBin(0.54);

  TH1D *hmbg = new TH1D("hmbg","mass background",60,0.4, 0.6);
  ntp_likesign->Draw("invariant_mass>>hmbg", paircut && crosscut && ptcut && "charge1==charge2");


  std::cout <<  "fg " <<  hm->Integral(binlo, binhi) << " bgd " << hmbg->Integral(binlo, binhi) << " net signal " <<  hm->Integral(binlo, binhi) -  hmbg->Integral(binlo, binhi) << std::endl;

  hm->GetXaxis()->SetTitle("invariant mass (GeV)");
  hmbg->SetLineColor(kRed);
  hmbg->SetLineWidth(2.0);
  hm->SetLineWidth(2.0);

 
  hm->DrawCopy();  
  hmbg->DrawCopy("same");  

  TLegend *leg = new TLegend(0.55, 0.7, 0.85, 0.85, "", "NDC");
  leg->AddEntry(hm,"Unlike sign","l");
  leg->AddEntry(hmbg,"Like sign","l");
  leg->Draw();
 
  c2->cd(2);
 
  hm->Sumw2();
  hmbg->Sumw2();
  hm->Add(hmbg,-1);
  hm->SetLineColor(kBlack);
  hm->SetMarkerStyle(20);
  hm->SetMarkerSize(0.3);
  hm->SetLineWidth(1.0);
  hm->SetTitle("net invariant mass");
  hm->DrawCopy("p");

  TF1 *f = new TF1("f","[0]+gaus(1)",0.4,0.6);
  f->SetParameters(0.0, 100.0, 0.49, 0.1);
  hm->Fit("f","R");
  f->Draw("same");

  TF1 * fg = new TF1("fg","gaus(0)", 0.4, 0.6);
  fg->SetParameters(f->GetParameter(1), f->GetParameter(2), f->GetParameter(3));
  // multiply value/bin by bins/GeV to  get counts from function integral
  std::cout << " fit signal integral " << fg->Integral(0.43, 0.54) * (60 / 0.2) << std::endl;

  TF1 * fe = new TF1("fe","[0]", 0.4, 0.6);
  fe->SetParameter(0, f->GetParameter(0));
  std::cout << " fit background integral " << fe->Integral(0.43, 0.54) * (60/ 0.2) << std::endl;

  TLine* one = new TLine(0.4, 0.0, 0.6, 0.0);
  one->Draw();

  c2->Update();
  c2->SaveAs("figure/plot2_mass.pdf");

  TCanvas *c3 = new TCanvas("c3","rapidity",250,150,800,400);
  c3->Divide(3,2);

  c3->cd(1);
  TH1D *hrap = new TH1D("hrap","rapidity",100,-1.5, 1.5);
  ntp_unlikesign->Draw("rapidity>>hrap",trcut && crosscut);
  hrap->Draw();  

  c3->cd(2);
  TH2D *hphi = new TH2D("hphi","phi",400,-3.5, 3.5,400, -3.5, 3.5);
  ntp_unlikesign->Draw("phi1:phi2>>hphi",trcut && crosscut);
  hphi->Draw();  

  c3->cd(3);
  TH1D *hcr = new TH1D("hcr","crossing1",600, -200, 400);
  ntp_unlikesign->Draw("crossing1>>hcr",trcut && crosscut);
  hcr->Draw();  

  c3->cd(4);
  TH2D *hpcar = new TH2D("hpcar","DCA vs rapidity",50, -1.5, 1.5, 30, -1.0, 1.0);
  ntp_unlikesign->Draw("projected_pair_dca:eta1>>hpcar",trcut && "crossing1 !=0 && invariant_mass > 0.43 && invariant_mass < 0.55");
  hpcar->Draw("colz");  

  c3->cd(5);
  TH2D *hpcac = new TH2D("hpcac","DCA vs crossing",50, -100, 100,30, -1.0, 1.0);
  ntp_unlikesign->Draw("projected_pair_dca:crossing1>>hpcac",trcut && "crossing1 != 0 && invariant_mass > 0.43 && invariant_mass < 0.55");
  hpcac->Draw("colz");  

  c3->cd(6);
  TH2D *hpcap = new TH2D("hpcap","DCA vs pathlength",50, 0, 4,30, -1.0, 1.0);
  ntp_unlikesign->Draw("projected_pair_dca:projected_pathlength>>hpcap",trcut && "invariant_mass > 0.43 && invariant_mass < 0.55");
  hpcap->Draw("colz");  

  c3->Update();
  c3->SaveAs("figure/plot3_rapidity.pdf");

  TCanvas *c4 = new TCanvas("c4","momentum",150,50,600,600);
  c4->Divide(2,2); 
  c4->cd(1);
  TH2D *hpt = new TH2D("hpt","pT1 vs pT2",100, 0, 2,100, 0, 2.0);
  ntp_unlikesign->Draw("sqrt(px1*px1+py1*py1):sqrt(px2*px2+py2*py2)>>hpt",trcut && "invariant_mass > 0.43 && invariant_mass < 0.55");
  hpt->Draw("colz");  

  c4->cd(2);
  TH1D *hmom = new TH1D("hmom","Ks momentum",100, 0, 3);
  ntp_unlikesign->Draw("sqrt(pow(projected_mom1_x + projected_mom2_x, 2) + pow(projected_mom1_y + projected_mom2_y, 2) + pow(projected_mom1_z + projected_mom2_z, 2))>>hmom",trcut && "invariant_mass > 0.43 && invariant_mass < 0.55");
  hmom->Draw("colz");  

  c4->cd(3);
  TH1D *hpks = new TH1D("hpks","Ks pT",100, 0, 3.0);
  ntp_unlikesign->Draw("invariant_pt>>hpks",trcut && "invariant_mass > 0.43 && invariant_mass < 0.55");
  hpks->Draw();  

  c4->Update();
  c4->SaveAs("figure/plot4_momentum.pdf");



  // get t0 and vd calibrated - how?
  // TrackResiduals
}
