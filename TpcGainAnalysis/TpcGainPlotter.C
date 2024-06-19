#include "TH1D.h"
#include "TH2D.h"
#include "TCut.h"
#include "TCanvas.h"
#include "TChain.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TFile.h"

#include "iostream"
#include <fstream>

double histmean(int binlo, int binhi, TH1D *h)
{
  int totbins = h->GetNbinsX();
  if (binhi > totbins) { binhi = totbins; }
  if (binlo < 1) {binlo = 1; }

  double sum = 0.0;
  double w = 0.0;

  for(int i = binlo; i < binhi; ++i)
    {
      sum += ( (double) i )* h->GetBinContent(i);
      w += h->GetBinContent(i);
    }

  double weighted_mean = sum / w;

  return weighted_mean;
}

void TpcGainPlotter()
{
  gStyle->SetOptStat(0);

  //TFile *fin = new TFile("original_gain_analysis_file.root");
  //TFile *fin = new TFile("iter1_gain_analysis_file.root");
  //TFile *fin = new TFile("iter1_thresh8_gain_analysis_file.root");
  //TFile *fin = new TFile("iter1_thresh10_gain_analysis_file.root");
TFile *fin = new TFile("temp_gain_analysis_file.root");

  TH1D *hadc_data[2][3][12];
  TH1D *hadc_sim[2][3][12];

  for(int iside = 0; iside < 2; ++iside)
    {
      
      for(int iregion = 0; iregion <3; ++iregion)
	{

	  for(int isector = 0; isector < 12; ++isector)
	    {
	      char hname[500];

	      sprintf(hname,"hadc_data_%i_%i_%i",iside, iregion+1, isector);
	      fin->GetObject(hname, hadc_data[iside][iregion][isector]);
	      sprintf(hname,"hadc_sim_%i_%i_%i",iside, iregion+1, isector);
	      fin->GetObject(hname, hadc_sim[iside][iregion][isector]);
	    }
	}
    }

  TCanvas *cs = new TCanvas("cs","TPC South", 5,5,1800,1800);
  cs->Divide(6,6);
  for(int isector = 0; isector < 12; ++isector)
    {
      for(int iregion = 0; iregion < 3; ++iregion)
	{
	  int npad = iregion + isector*3 + 1;
	  cs->cd(npad);
	  hadc_sim[0][iregion][isector]->GetXaxis()->SetTitle("cluster ADC");
	  hadc_sim[0][iregion][isector]->GetXaxis()->SetTitleSize(0.048);
	  hadc_sim[0][iregion][isector]->DrawCopy();
	  hadc_data[0][iregion][isector]->SetLineColor(kRed);
	  hadc_data[0][iregion][isector]->DrawCopy("same");

	  char module[500];
	  sprintf(module,"#splitline{side %i region %i}{ sector %i}",0,iregion+1,isector);
	  TLatex *lab = new TLatex(0.4,0.7,module);
	  lab->SetNDC();
	  lab->SetTextSize(0.08);
	  lab->Draw();
	}
    }

  TCanvas *cn = new TCanvas("cn","TPC North", 300,50,1800,1800);
  cn->Divide(6,6);
  for(int isector = 0; isector < 12; ++isector)
    {
      for(int iregion = 0; iregion < 3; ++iregion)
	{
	  int npad = iregion + isector*3 + 1;
	  cn->cd(npad);
	  hadc_sim[1][iregion][isector]->GetXaxis()->SetTitle("cluster ADC");
	  hadc_sim[1][iregion][isector]->GetXaxis()->SetTitleSize(0.048);
	  hadc_sim[1][iregion][isector]->DrawCopy();
	  hadc_data[1][iregion][isector]->SetLineColor(kRed);
	  hadc_data[1][iregion][isector]->DrawCopy("same");

	  char module[500];
	  sprintf(module,"#splitline{side %i region %i}{ sector %i}",1,iregion+1,isector);
	  TLatex *lab = new TLatex(0.4,0.7,module);
	  lab->SetNDC();
	  lab->SetTextSize(0.08);
	  lab->Draw();

	}
    }

  // get the mean cluster ADC values from the histograms and make the data/sim ratios
  std::ofstream fout("tpc_gain_ratios.txt",std::ofstream::out);
  double ratio_average[2] = {0, 0};
  double ratio_count[2] = {0, 0};
  for(int iside = 0;iside < 2; ++iside)
    {
      for(int isector = 0; isector < 12; ++isector)
	{
	  for(int iregion = 0; iregion < 3; ++iregion)
	    {
	      // double md = hadc_data[iside][iregion][isector]->GetMean();
	      // double ms = hadc_sim[iside][iregion][isector]->GetMean();

	      double md = histmean( 1, 500, hadc_data[iside][iregion][isector]);
	      double ms = histmean( 1, 500, hadc_sim[iside][iregion][isector]);

	      std::cout << " iside " << iside << " iregion " << iregion << " isector " << isector << " data mean " << md << " sim mean " << ms << " ratio " << md/ms << std::endl; 

	      fout << iside << "  " << iregion << "  " << isector << "  " << md/ms << std::endl;

	      ratio_average[iside] += md/ms;
	      ratio_count[iside]++;
	    }
	}
      ratio_average[iside] /= ratio_count[iside];
      std::cout << " ratio average for side " << iside << " is " << ratio_average[iside] << std::endl;
    }

}




