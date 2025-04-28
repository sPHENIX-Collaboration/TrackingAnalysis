const int nmvtxpackets = 12;
const int ninttpackets = 8;

void PlotRatios(const std::string& infile, const int runnumber, const bool MVTX)
{

  TFile *file = TFile::Open(infile.c_str());
  ostringstream runno;
  runno.str("");
  runno << runnumber;
  const bool INTT = !MVTX;
  
  if(MVTX)
    {
      const int mvtxgl1 = ((TH1*)file->Get("h_MvtxPoolQA_RefGL1BCO"))->GetEntries() / (nmvtxpackets / 2.);
      std::cout << "ngl1s " << mvtxgl1 << std::endl;
      auto fees = (TH1*)file->Get("h_MvtxPoolQA_TagStBcoFEEs");

  TH1 *mvtxl1bcodiff[nmvtxpackets];
  TH1 *mvtxstrobebcodiff[nmvtxpackets];
  TH1 *allfeesmvtx[nmvtxpackets];
  TH1 *feesstrobemvtx[nmvtxpackets];
  TH1 *feesll1mvtx[nmvtxpackets];
  ostringstream name;
  for(int i=0; i<nmvtxpackets; i++)
    {
      name.str("");
      name << "h_MvtxPoolQA_GL1LL1BCODiff_packet"<<i;
      mvtxl1bcodiff[i] = (TH1*)file->Get(name.str().c_str());

      name.str("");
      name << "h_MvtxPoolQA_LL1StrobeBCODiff_packet"<<i;
      mvtxstrobebcodiff[i] = (TH1*)file->Get(name.str().c_str());

      name.str("");
      name << "h_MvtxPoolQA_TagBCOAllFees_Felix"<<i;
      allfeesmvtx[i] = (TH1*)file->Get(name.str().c_str());

      name.str("");
      name << "h_MvtxPoolQA_TagStBcoFEEsPacket"<<i;
      feesstrobemvtx[i] = (TH1*)file->Get(name.str().c_str());

      name.str("");
      name << "h_MvtxPoolQA_TagL1BcoFEEsPacket"<<i;
      feesll1mvtx[i] = (TH1*)file->Get(name.str().c_str());
    }

  
  TH1 *feesll1frac[nmvtxpackets];
  for(int i=0; i<nmvtxpackets; i++)
    {
      name.str("");
      name<<"feesl1frac"<<i;
      feesll1frac[i] = new TH1F(name.str().c_str(),";FEEID;GL1 LL1 Tag Frac",12,0,12);
      int feeid = 0;
      for(int j=0; j<feesll1mvtx[i]->GetXaxis()->GetNbins(); j++)
	{
	  if(feesll1mvtx[i]->GetBinContent(j+1) ==0)
	    {
	      continue;
	    }
	  
	  const int numbcosFee = feesll1mvtx[i]->GetBinContent(j+1);
        
	  float frac = (float)numbcosFee / mvtxgl1;
	  feesll1frac[i]->SetBinContent(feeid+1, frac);
	  feeid++;

	}
      feesll1frac[i]->GetYaxis()->SetRangeUser(0,1.3);
      
    }

  TCanvas *can6 = new TCanvas("Fee LL1 BCO Match Summary","Fee LL1 BCO Match Summary",200,200,600,600);

  for(int i=0; i<nmvtxpackets; i++)
    {
      feesll1frac[i]->SetLineColor(i+1);
      feesll1frac[i]->SetMarkerColor(i+1);
      if(i==0)
	{
	  TAxis *x = feesll1frac[i]->GetXaxis();
	  TAxis *y = feesll1frac[i]->GetYaxis();
	  x->SetTitle("FEEID");
	  y->SetTitle("LL1-GL1 Tagged Frac.");
	  y->SetRangeUser(0,1.3);
	  feesll1frac[i]->Draw("hist");

	}
      else
	{
	  feesll1frac[i]->Draw("histsame");
	}
    }
  myText(0.22,0.87,kBlack,"#bf{#it{sPHENIX}} internal");
  myText(0.22,0.81,kBlack,runno.str().c_str());
  myText(0.22,0.69,feesll1frac[0]->GetLineColor(),"Felix.Endpoint 0.0");
  myText(0.22,0.63,feesll1frac[1]->GetLineColor(),"Felix.Endpoint 0.1");
  myText(0.22,0.57,feesll1frac[2]->GetLineColor(),"Felix.Endpoint 1.0");
  myText(0.22,0.51,kBlack,"...");


  
  TCanvas *can4 = new TCanvas("Fee Strobe BCO Match","Fee Strobe BCO Match",200,200,600,600);
  can4->Divide(3,4);
  TH1 *feesstrobefrac[nmvtxpackets];
  for(int i=0; i<nmvtxpackets; i++)
    {
      name.str("");
      name<<"feesstrobefrac"<<i;
      feesstrobefrac[i] = new TH1F(name.str().c_str(),";FEEID;GL1 Strobe Tag Frac",12,0,12);
      int feeid = 0;
      for(int j=0; j<feesstrobemvtx[i]->GetXaxis()->GetNbins(); j++)
	{
	  if(feesstrobemvtx[i]->GetBinContent(j+1) ==0)
	    {
	      continue;
	    }
	  
	  const int numbcosFee = feesstrobemvtx[i]->GetBinContent(j+1);
        
	  float frac = (float)numbcosFee / mvtxgl1;
	  feesstrobefrac[i]->SetBinContent(feeid+1, frac);
	  feeid++;

	}
      can4->cd(i+1);
      feesstrobefrac[i]->GetYaxis()->SetRangeUser(0,1.3);
      feesstrobefrac[i]->Draw();
      if(i==0)
	{
	  myText(0.22,0.85,kBlack,"#bf{#it{sPHENIX}} internal");
	}
      name.str("");
      name << "Felix "<<i/2 << "."<<i%2;
      myText(0.4,0.4,kBlack,name.str().c_str());
    }

  TCanvas *can5 = new TCanvas("Fee Strobe BCO Match Summary","Fee Strobe BCO Match Summary",200,200,600,600);

  for(int i=0; i<nmvtxpackets; i++)
    {
      feesstrobefrac[i]->SetLineColor(i+1);
      feesstrobefrac[i]->SetMarkerColor(i+1);
      if(i==0)
	{
	  TAxis *x = feesstrobefrac[i]->GetXaxis();
	  TAxis *y = feesstrobefrac[i]->GetYaxis();
	  x->SetTitle("FEEID");
	  y->SetTitle("Strobe-GL1 Tagged Frac.");
	  y->SetRangeUser(0,1.3);
	  feesstrobefrac[i]->Draw("hist");

	}
      else
	{
	  feesstrobefrac[i]->Draw("histsame");
	}
    }
  myText(0.22,0.87,kBlack,"#bf{#it{sPHENIX}} internal");
  myText(0.22,0.81,kBlack,runno.str().c_str());
  myText(0.22,0.69,feesstrobefrac[0]->GetLineColor(),"Felix.Endpoint 0.0");
  myText(0.22,0.63,feesstrobefrac[1]->GetLineColor(),"Felix.Endpoint 0.1");
  myText(0.22,0.57,feesstrobefrac[2]->GetLineColor(),"Felix.Endpoint 1.0");
  myText(0.22,0.51,kBlack,"...");


  
  TCanvas *can2 = new TCanvas("can2","can2",200,200,600,600);
  can2->Divide(3,4);
  can2->cd(1);
  int cdcan=1;
  for(int i=0; i<nmvtxpackets; i++)
    {
      if(mvtxstrobebcodiff[i]->GetEntries() ==0)
	{
	  std::cout << "continuing over " << i<<std::endl;
	  continue;
	}

      mvtxstrobebcodiff[i]->GetXaxis()->SetRangeUser(1,100000);
      
      mvtxstrobebcodiff[i]->Draw();
      gPad->SetLogx();
      
      name.str("");
      name << "Felix " << i / 2 << "." << i%2;
      if(cdcan==1)
	myText(0.22,0.87,kBlack,"#bf{#it{sPHENIX}} internal");
      myText(0.22,0.8,kBlack,name.str().c_str());
      cdcan++;
      can2->cd(cdcan);
      
      
    }

  TCanvas *can3 = new TCanvas("can3","can3",200,200,600,600);
  can3->Divide(3,4);
  cdcan = 1;
  can3->cd(cdcan);
  for(int i=0; i<nmvtxpackets; i++)
    {
      if(mvtxl1bcodiff[i]->GetEntries() == 0)
	{
	  continue;
	}
      
      gStyle->SetOptStat(0);
    
      mvtxl1bcodiff[i]->Draw();
      mvtxl1bcodiff[i]->GetXaxis()->SetRangeUser(0,1000);
      mvtxl1bcodiff[i]->GetYaxis()->SetRangeUser(1,mvtxl1bcodiff[i]->GetBinContent(mvtxl1bcodiff[i]->GetMaximumBin())*10);
      gPad->SetLogy();
      
        name.str("");
      name << "Felix " << i / 2 << "." << i%2;
      if(cdcan==1)
	myText(0.22,0.87,kBlack,"#bf{#it{sPHENIX}} internal");
      
      myText(0.22,0.8,kBlack,name.str().c_str());
      
      cdcan++;
      can3->cd(cdcan);
    }
  
  TCanvas *can1 = new TCanvas("can1","can1",200,200,600,600);
  
  TH1 *allfees = new TH1F("mvtxallfees",";Felix.Endpoint;LL1-GL1 Tagged Fraction",12,0,12);
  for(int i=0; i<12; i++)
    {
      if(allfeesmvtx[i]->GetEntries()==0)
	{
	  std::cout << "skipping " << i << std::endl;
	  continue;
	}
     
      const float frac = allfeesmvtx[i]->GetEntries() / mvtxgl1; 

  
      allfees->SetBinContent(i+1,frac);
      ostringstream felix;
      felix.str("");
      felix << i / 2 << "." << i%2;
      allfees->GetXaxis()->SetBinLabel(i+1,felix.str().c_str());
    }
  allfees->GetYaxis()->SetTitleOffset(1.6);
  allfees->GetYaxis()->SetLabelSize(0.045);
  allfees->GetYaxis()->SetRangeUser(0,1.3);
  allfees->Draw();
  
  myText(0.45,0.7,kBlack,"#bf{#it{sPHENIX}} internal");
  myText(0.45,0.63,kBlack,runno.str().c_str());
    }


  if (INTT)
    {
      const int inttgl1 = ((TH1*)file->Get("h_InttPoolQA_RefGL1BCO"))->GetEntries() / ninttpackets;
      ostringstream name;
      float allfeestagged[ninttpackets] = {0};
      float perfee[ninttpackets][14] = {{0}};
      TGraph *grs[ninttpackets];
      std::cout << "intt gl1 " << inttgl1<<std::endl;
      for(int i=0; i<ninttpackets; i++)
	{
	  name.str("");
	  name <<"h_InttPoolQA_TagBCOAllFees_Server"<<i;
	  allfeestagged[i] = ((TH1*)file->Get(name.str().c_str()))->GetEntries();
	  allfeestagged[i] /= inttgl1;
	  float x[14];
	  for(int j=0; j<14; j++)
	    {
	      name.str("");
	      name << "h_InttPoolQA_TagBCO_server"<<i<<"_fee"<<j;
	      perfee[i][j] = ((TH1*)file->Get(name.str().c_str()))->GetEntries();
	
	      perfee[i][j] /= inttgl1;
	      x[j] = j;
	    }
	  name.str("");
	  name<<"perfeegr"<<i;
	  grs[i] = new TGraph(14,x,perfee[i]);
	}

      TH1 *packetallfees = new TH1F("inttpacketallfees",";Server;Frac. GL1 Tagged",8,0,8);
      for(int i=0; i<ninttpackets; i++)
	{
	  packetallfees->SetBinContent(i+1, allfeestagged[i]);
	}


      TCanvas *inttcan1 = new TCanvas("inttcan1","inttcan1",200,200,600,600);
      gStyle->SetOptStat(0);
      packetallfees->GetYaxis()->SetRangeUser(0,1.2);
      
      packetallfees->Draw();
      myText(0.22,0.87,kBlack,"#bf{#it{sPHENIX}} internal");
      myText(0.22,0.8,kBlack,runno.str().c_str());
      TCanvas *inttfeecan = new TCanvas("inttfeecan","inttfeecan",200,200,800,800);
      inttfeecan->Divide(2,4);
      for(int i=0; i<ninttpackets; i++)
	{
	  inttfeecan->cd(i+1);
	  grs[i]->GetXaxis()->SetTitle("FEEID");
	  grs[i]->GetYaxis()->SetTitle("Frac GL1 Tagged");
	  grs[i]->GetYaxis()->SetRangeUser(0,1);
	  grs[i]->Draw("ap");
	  ostringstream felix;
	  felix.str("");
	  felix << "Server " <<i;
	  if(i==0)
	    myText(0.22,0.87,kBlack,"#bf{#it{sPHENIX}} internal",0.07);
	  myText(0.22,0.8,kBlack,felix.str().c_str(),0.07);

	}
      
    }

  
}
