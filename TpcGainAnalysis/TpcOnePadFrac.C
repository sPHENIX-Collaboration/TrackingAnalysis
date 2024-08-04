/*
 * Macro that makes plots of the sector by sector one pad fractions and max
 * cluster ADCs for clusters on tracks that have pT>1 GeV, nTPC>30, and 
 * negative charge. Additional cuts can be added to trcut (e.g. nINTT>0). 
 * 
 *
 */
bool onepadfracs = false;
bool cmaxadcs = true;

void TpcOnePadFrac(const std::string filename)
{

  gStyle->SetOptStat(0);
  gStyle->SetCanvasPreferGL();
  TCut nscut[2] = {"clusside==0","clusside==1"};
  TCut sectorcut[12] = {"clussector==0", "clussector==1", "clussector==2", "clussector==3", "clussector==4", "clussector==5", "clussector==6", "clussector==7", "clussector==8", "clussector==9", "clussector==10", "clussector==11"};
  std::string tpcside[2] = {"South", "North"};
  
  TCut trcut("m_pt > 1.0 && m_ntpc>30 && m_charge<0");
  TCut regioncut[3] = {
    "cluslayer > 6 && cluslayer < 23",
    "cluslayer > 22 && cluslayer < 39",
    "cluslayer > 38 && cluslayer < 55"  };

  TFile *file = TFile::Open(filename.c_str());
  
  TTree *tree = (TTree*)file->Get("residualtree");
  ostringstream name;
  TH1* cphisizegrs[2][12][3] = {{{nullptr}}};
  float ntot[2][12][3] = {{{0}}};
  float ngr1[2][12][3] = {{{0}}};
  float frac[2][12][3] = {{{0}}};

  TH1 *cmaxadc[2][12][3] = {{{nullptr}}};
  float cmaxmeans[2][12][3] = {{{0}}};
  std::cout<< "Drawing"<<std::endl;
  for(int side=0; side<2; side++)
    {
      for(int sector=0; sector<12; sector++)
	{
	  for(int region=0; region<3; region++)
	    {
	      if(onepadfracs)
		{
	      name.str("");
	      name << "clusphisize>>cphisize_"<<side<<"_"<<sector<<"_"<<region<<"(8,0,8)";
	      std::cout << "Drawing " << side << ", " <<sector << ", " << region << std::endl;
	      tree->Draw(name.str().c_str(), nscut[side] && regioncut[region] && trcut && sectorcut[sector] && "cluszsize>=1");
	      name.str("");
	      name << "cphisize_"<<side<<"_"<<sector<<"_"<<region;
	      cphisizegrs[side][sector][region] = (TH1*)gDirectory->Get(name.str().c_str());
	      ntot[side][sector][region] = cphisizegrs[side][sector][region]->GetEntries();
	      ngr1[side][sector][region] = cphisizegrs[side][sector][region]->GetBinContent(2) + cphisizegrs[side][sector][region]->GetBinContent(1);

	      frac[side][sector][region] = ngr1[side][sector][region] / ntot[side][sector][region];
		}
	      if(cmaxadcs)
		{
		  std::cout << "Drawing " << side << ", " <<sector << ", " << region << std::endl;
	      name.str("");
	      name << "clusMaxAdc>>cmaxadc_"<<side<<"_"<<sector<<"_"<<region<<"(150,0,300)";
	      tree->Draw(name.str().c_str(), nscut[side] && regioncut[region] && trcut && sectorcut[sector] && "cluszsize>1");
	      name.str("");
	      name << "cmaxadc_"<<side<<"_"<<sector<<"_"<<region;
	      cmaxadc[side][sector][region] = (TH1*)gDirectory->Get(name.str().c_str());
	      cmaxmeans[side][sector][region] = cmaxadc[side][sector][region]->GetMean();
		}
	    }
	}
    }
  std::cout <<"Canvasing"<<std::endl;
  if(onepadfracs)
    {
  TCanvas *cans[2][3];
  for(int side=0;side<2;side++)
    {
      for(int region=0; region<3; region++)
	{
	  name.str("");
	  name<<"can_"<<side<<"_"<<region;
	  cans[side][region] = new TCanvas(name.str().c_str(),name.str().c_str(),200,200,1000,1000);
	  cans[side][region]->Divide(3,4);
	  cans[side][region]->cd(1);
	  if(side==0){
	    name.str("");
	    name << "#bf{South region " <<region<<"}";
	    myText(0.02,0.02,kBlack,name.str().c_str(),0.07);
	  }
	  if(side==1)
	    {
	      name.str("");
	      name << "#bf{North region " << region<<"}";
	      myText(0.02,0.02,kBlack,name.str().c_str(),0.07);
	    }
	  for(int sec=0;sec<12;sec++)
	    {
	      cans[side][region]->cd(sec+1);
	      cphisizegrs[side][sec][region]->GetXaxis()->SetTitle("Cluster phi size");
	      cphisizegrs[side][sec][region]->Draw("hist");
	      if(sec==0)
		{
		  myText(0.5,0.85,kBlack,"#bf{#it{sPHENIX}} internal");
		}
	      name.str("");
	      name << "one pad frac " << std::setprecision(3) << frac[side][sec][region];
	      myText(0.55,0.78,kBlack,name.str().c_str());
	      name.str("");
	      name << "Sector " << sec;
	      myText(0.55,0.71,kBlack,name.str().c_str());
	    }
	  name.str("");
	  name << "plots/2024/8_4/OPFSide26x26_HV3.5_"<<side<<"Region_"<<region<<".pdf";
	  cans[side][region]->SaveAs(name.str().c_str());
	}
    }
    }

  if(cmaxadcs)
    {
       TCanvas *cans[2][3];
  for(int side=0;side<2;side++)
    {
      for(int region=0; region<3; region++)
	{
	  name.str("");
	  name<<"cancmax_"<<side<<"_"<<region;
	  cans[side][region] = new TCanvas(name.str().c_str(),name.str().c_str(),200,200,1000,1000);
	  cans[side][region]->Divide(3,4);
	  cans[side][region]->cd(10);
	  if(side==0){
	    name.str("");
	    name << "#bf{South region " <<region<<"}";
	    myText(0.02,0.02,kBlack,name.str().c_str(),0.07);
	  }
	  if(side==1)
	    {
	      name.str("");
	      name << "#bf{North region " << region<<"}";
	      myText(0.02,0.02,kBlack,name.str().c_str(),0.07);
	    }
	  for(int sec=0;sec<12;sec++)
	    {
	      cans[side][region]->cd(sec+1);
	      cmaxadc[side][sec][region]->GetXaxis()->SetTitle("Cluster Max ADC");
	      
	      //cmaxadc[side][sec][region]->SetFillColor(kRed);
	      cmaxadc[side][sec][region]->SetLineColor(kRed);
	      cmaxadc[side][sec][region]->SetFillColorAlpha(kRed,0.2);
	      cmaxadc[side][sec][region]->SetFillStyle(1001);
	      cmaxadc[side][sec][region]->Draw("hist");
	      if(side==0)
		{
		  myText(0.5,0.85,kBlack,"#bf{#it{sPHENIX}} internal");
		}
	      name.str("");
	      name << "Mean " << std::setprecision(3) << cmaxmeans[side][sec][region] << "+/-"<<cmaxadc[side][sec][region]->GetMeanError();
	      myText(0.55,0.78,kBlack,name.str().c_str());
	      name.str("");
	      name << "Sector " << sec;
	      myText(0.55,0.71,kBlack,name.str().c_str());
	    }
	  std::cout << "Printing"<<std::endl;
	  name.str("");
	  name << "plots/2024/8_4/CMaxSide26x26_HV3.5_"<<side<<"Region_"<<region<<".pdf";
	  cans[side][region]->SaveAs(name.str().c_str());
	}
    }

    }
  std::cout << "Exiting "<<std::endl;
  gSystem->Exit(0);
}
