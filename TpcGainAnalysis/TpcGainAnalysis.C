#include "TH1D.h"
#include "TH2D.h"
#include "TCut.h"
#include "TCanvas.h"
#include "TChain.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TFile.h"

#include "iostream"


void TpcGainAnalysis()
{
  gStyle->SetOptStat(0);

  double maxadc = 1000.0;

  TFile *fout = new TFile("temp_gain_analysis_file.root","recreate");

  /*
    region 0 = layer 7-22
    region 1 = layer 23-38
    region 3 = layer 39-54
   */

  // cuts
  TCut trcut("m_pt > 1.0 && quality < 30 && m_ntpc > 25");
  TCut nscut[2] = {"clusgz < 0", "clusgz > 0"};
  TCut regioncut[3] = {
    "cluslayer < 23 && cluslayer > 6",
    "cluslayer > 22 && cluslayer < 39",
    "cluslayer > 38"  };
  TCut phicut[12] = {
    "atan2(clusgy,clusgx) > -15.0 * 3.14159 /180.0 &&  atan2(clusgy,clusgx) < 15.0 * 3.14159 /180.0",
    "atan2(clusgy,clusgx) > 15.0 * 3.14159 /180.0 &&  atan2(clusgy,clusgx) < 45.0 * 3.14159 /180.0",
    "atan2(clusgy,clusgx) > 45.0 * 3.14159 /180.0 &&  atan2(clusgy,clusgx) < 75.0 * 3.14159 /180.0",
    "atan2(clusgy,clusgx) > 75.0 * 3.14159 /180.0 &&  atan2(clusgy,clusgx) < 105.0 * 3.14159 /180.0",
    "atan2(clusgy,clusgx) > 105.0 * 3.14159 /180.0 &&  atan2(clusgy,clusgx) < 135.0 * 3.14159 /180.0",
    "atan2(clusgy,clusgx) > 135.0 * 3.14159 /180.0 &&  atan2(clusgy,clusgx) < 165.0 * 3.14159 /180.0",
    "atan2(clusgy,clusgx) > 165.0 * 3.14159 /180.0 &&  atan2(clusgy,clusgx) < 195.0 * 3.14159 /180.0",
    "atan2(clusgy,clusgx) > -165.0 * 3.14159 /180.0 &&  atan2(clusgy,clusgx) < -135.0 * 3.14159 /180.0",
    "atan2(clusgy,clusgx) > -135.0 * 3.14159 /180.0 &&  atan2(clusgy,clusgx) < -105.0 * 3.14159 /180.0",
    "atan2(clusgy,clusgx) > -105.0 * 3.14159 /180.0 &&  atan2(clusgy,clusgx) < -75.0 * 3.14159 /180.0",
    "atan2(clusgy,clusgx) > -75.0 * 3.14159 /180.0 &&  atan2(clusgy,clusgx) < -45.0 * 3.14159 /180.0",
    "atan2(clusgy,clusgx) > -45.0 * 3.14159 /180.0 &&  atan2(clusgy,clusgx) < -15.0 * 3.14159 /180.0" };

  // read simulation files
  TChain *ntp_sim = new TChain("residualtree");
    for(int i =0;i<80;++i)
    {  
      char name[500];
      sprintf(name,"/sphenix/tg/tg01/hf/frawley/sims/test_gem_gain/avge950_iter1_thresholds10/pythia_pp_%i.root_residuals.root",i);
      ntp_sim->Add(name);
      std::cout << "Adding sims file " << name << std::endl;
    }
  

  // read data files
  TChain *ntp_data = new TChain("residualtree");
  for(int i=0;i<100;++i)
    {
      // typical run from our development data set
      char name[500];
      sprintf(name,"/sphenix/tg/tg01/hf/frawley/data/run41989_seeds/clusters_tracks_41989-%i.root_resid.root",i);
      ntp_data->Add(name);
      sprintf(name,"/sphenix/tg/tg01/hf/frawley/data/run41990_seeds/clusters_tracks_41990-%i.root_resid.root",i);
      ntp_data->Add(name);
     sprintf(name,"/sphenix/tg/tg01/hf/frawley/data/run41991_seeds/clusters_tracks_41991-%i.root_resid.root",i);
      ntp_data->Add(name);
     sprintf(name,"/sphenix/tg/tg01/hf/frawley/data/run41992_seeds/clusters_tracks_41992-%i.root_resid.root",i);
      ntp_data->Add(name);
    }

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
	      TH1D * hist = new TH1D("hist","", 1000, 0, maxadc);
	      ntp_data->Draw("clusAdc>>hist", trcut && nscut[iside] && regioncut[iregion] && phicut[isector] && "cluszsize>1 && clusAdc > 10","goff");
	      hadc_data[iside][iregion][isector] = (TH1D*) hist->Clone();
	      hadc_data[iside][iregion][isector]->SetName(hname);
	      std::cout << "Filled " << hname << std::endl;
	      hadc_data[iside][iregion][isector]->Write();

	      sprintf(hname,"hadc_sim_%i_%i_%i",iside, iregion+1, isector);
	      ntp_sim->Draw("clusAdc>>hist", trcut && nscut[iside] && regioncut[iregion] && phicut[isector] && "cluszsize>1 && clusAdc > 10","goff");
	      hadc_sim[iside][iregion][isector] = (TH1D*) hist->Clone();
	      hadc_sim[iside][iregion][isector]->SetName(hname);
	      std::cout << "Filled " << hname << std::endl;
	      hadc_sim[iside][iregion][isector]->Write();
	      delete hist;

	    }
	}
    }

  TCanvas *cs = new TCanvas("cs","cs", 5,5,1800,1800);
  cs->Divide(6,6);
  for(int isector = 0; isector < 12; ++isector)
    {
      for(int iregion = 0; iregion < 3; ++iregion)
	{
	  int npad = iregion + isector*3 + 1;
	  cs->cd(npad);
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

  TCanvas *cn = new TCanvas("cn","cn", 300,50,1800,1800);
  cn->Divide(6,6);
  for(int isector = 0; isector < 12; ++isector)
    {
      for(int iregion = 0; iregion < 3; ++iregion)
	{
	  int npad = iregion + isector*3 + 1;
	  cn->cd(npad);
	  hadc_sim[1][iregion][isector]->GetXaxis()->SetTitle("cluster ADC");
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
  for(int iside = 0;iside < 2; ++iside)
    {
      for(int isector = 0; isector < 12; ++isector)
	{
	  for(int iregion = 0; iregion < 3; ++iregion)
	    {
	     double md = hadc_data[iside][iregion][isector]->GetMean();
	      double ms = hadc_sim[iside][iregion][isector]->GetMean();
	      std::cout << " data mean " << md << " sim mean " << ms << " ratio " << md/ms << std::endl; 
	    }
	}
    }





  fout->Close();
}




