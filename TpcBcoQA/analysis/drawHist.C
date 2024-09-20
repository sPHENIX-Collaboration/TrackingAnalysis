#define drawHist_cxx
#include "/sphenix/user/jpark4/Utility/commonUtility.h"
#include "/sphenix/user/jpark4/Utility/Style_jaebeom.h"

using namespace std;

void drawHist(int runnumber=52462, float ninterval = 10, int ntotal=1000000)
{
  const int npackets=48;
  gStyle->SetPalette(kRainBow);
  gStyle->SetOptStat(0);
  TFile *f = new TFile(Form("histfiles/hist_%d_interval%.2fk_tot%d.root",runnumber,ninterval,ntotal),"read");

  TH1D* h_event = (TH1D*) f->Get("h_event");
  float ntot = h_event->GetBinContent(1) / 1000000.0;
  float efftot = (float) h_event->GetBinContent(3)/h_event->GetBinContent(1); // 1000.0;
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(1) << ntot << "M";
  std::string nevents_label = oss.str();

  const int nfiles=24;
  Int_t nColors = 100; 
  Double_t stops[] = {0.00, 0.50, 1.00};  
  Double_t red[]   = {1.00, 0.00, 0.00}; 
  Double_t green[] = {0.00, 1.00, 0.00};
  Double_t blue[]  = {0.00, 0.00, 1.00};
  Int_t *colors = new Int_t[nColors];
  TColor::CreateGradientColorTable(3, stops, red, green, blue, nColors);
  for (int i = 0; i < nColors; ++i) {
    colors[i] = TColor::GetColorPalette(i);
  }

  TGraphErrors *g_event[nfiles];
  TGraphErrors *g_eff_event_all = (TGraphErrors*) f->Get("g_eff_event_all");
  for(int i =0;i<nfiles; i++){
    g_event[i] = (TGraphErrors*) f->Get(Form("g_event_server%d",i));
    g_event[i]->SetLineColor(colors[i * nColors / nfiles]);
    g_event[i]->SetLineWidth(2);
    g_event[i]->SetMarkerColor(colors[i * nColors / nfiles]);
    g_event[i]->SetMarkerSize(1.2);
  }

  g_eff_event_all->SetLineWidth(3);
  g_eff_event_all->SetLineColor(kBlack);
  g_eff_event_all->SetMarkerColor(kBlack);

  TLegend *l = new TLegend(0.23,0.2,0.5,0.47);
  SetLegendStyle(l);
  l->SetHeader("Individual servers (00-23)");
  l->AddEntry(g_event[0],"ebdc00","l");
  l->AddEntry(g_event[1],"ebdc01","l");
  l->AddEntry((TObject*)0,"...","");
  l->AddEntry(g_event[22],"ebdc22","l");
  l->AddEntry(g_event[23],"ebdc23","l");

  TLegend *l2 = new TLegend(0.57,0.40,0.80,0.49);
  SetLegendStyle(l2);
  l2->AddEntry(g_eff_event_all,"All ebdc00-23","l");

  TCanvas *c= new TCanvas("c","",700,700);
  c->cd();
  gPad->SetTicks(1,1);
  gPad->SetRightMargin(0.07);
  gPad->SetLeftMargin(0.12);
  gPad->SetTopMargin(0.04);
  gPad->SetBottomMargin(0.112);
  g_event[0]->GetXaxis()->SetTitle(Form("%.2fk event segment",ninterval));
  g_event[0]->GetXaxis()->CenterTitle();
  g_event[0]->GetYaxis()->CenterTitle();
  g_event[0]->GetXaxis()->SetTitleOffset(1.15);
  g_event[0]->GetXaxis()->SetTitleSize(0.04);
  g_event[0]->GetYaxis()->SetTitleSize(0.04);
  g_event[0]->GetXaxis()->SetNdivisions(510);
  g_event[0]->GetYaxis()->SetRangeUser(0.3,1.4);
  g_event[0]->Draw("APL");
  for(int i =1;i<nfiles; i++){
    g_event[i]->Draw("PL same");
  }
  g_eff_event_all->Draw("PL same");
  l->Draw("same");
  l2->Draw("same");
  drawText("#bf{#it{sPHENIX}} Internal",0.56,0.88,1,31);
  drawText("#sqrt{s} = 200 GeV",0.56,0.82,1,27);
  drawText(Form("Run %d %s events",runnumber,nevents_label.c_str()),0.534,0.71,1,21);
  drawText("#bf{TPC-GL1 tagging}",0.21,0.78,1,21);
  float toteff = h_event->GetBinContent(3)/h_event->GetBinContent(1);
  string perc = "%";
  drawText(Form("Total eff. %.1f%s",toteff*100,perc.c_str()),0.21,0.73,kRed,21);
  c->SaveAs(Form("plots/c_eff_vs_event_run%d_interval%.2fk_tot%d.pdf",runnumber,ninterval,ntotal));
}
