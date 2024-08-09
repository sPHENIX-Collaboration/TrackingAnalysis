// This is a macro that plots the BCO QA information produced as a part of the
// Fun4AllStreamingInputManager into summary plots showing fraction of tagged BCOs as a function of packet number
const int ntpcebdcs = 24;
const int ninttebdcs = 8;
const int ninttfees = 14;
const int ntpcpackets = 2;
const int nmvtxfelix = 12;
const int nmvtxfees = 12;


// options to run only your favorite detector
bool tpc = true;
bool intt = true;
bool mvtx = true;

/// Input filename should look like "HIST_XXXXX.root" where XXXXX is a runnumber
void PlotRatios(std::string filename)
{
  
  TFile *file = TFile::Open(filename.c_str());
  //TFile *file = TFile::Open("/sphenix/data/data02/sphnxpro/streamhist/run_00048400_00048500/HIST_DST_STREAMING_EVENT_run2pp_new_2024p002-48473-0000.root");
  auto pos = filename.find("HIST_");
  auto runnumstr = filename.substr(pos+5,5);
  int runnumber = std::stoi(runnumstr);
  std::cout << "runnumber is " << runnumber << std::endl;
  ostringstream name;
  if(intt)
    {
  TH1* inttrefgl1 = (TH1*)file->Get("h_InttPoolQA_RefGL1BCO");
  const int nInttGL1BCOs = inttrefgl1->GetEntries();
  TH1* inttallpackets = (TH1*)file->Get("h_InttPoolQA_TagBCOAllServers");
  TH1 *inttallpacketsallfees = (TH1*)file->Get("h_InttPoolQA_TagBCOAllServersAllFees");
  TH1* inttTagEbdc[ninttebdcs+1];
  TH1* inttTagEbdcFee[ninttebdcs][ninttfees];
  int nInttTaggedBCOs[ninttebdcs+1];
  int nInttTaggedBCOsFee[ninttebdcs][ninttfees];
  float nInttTaggedBCOsServers[ninttebdcs+1];
  for(int i=0; i<ninttebdcs; i++)
    {
      
      name.str("");
      name << "h_InttPoolQA_TagBCO_server" << i;
      inttTagEbdc[i] = (TH1*)file->Get(name.str().c_str());
      nInttTaggedBCOs[i] = inttTagEbdc[i]->GetEntries();
      name.str("");
      name << "h_InttPoolQA_TagBCOAllFees_Server"<<i;
      nInttTaggedBCOsServers[i] = ((TH1*)file->Get(name.str().c_str()))->GetEntries();
      nInttTaggedBCOsServers[i] /= nInttGL1BCOs;
      for(int j=0; j<ninttfees; j++)
	{
	  name.str("");
	  name << "h_InttPoolQA_TagBCO_server"<<i<<"_fee"<<j;
	  inttTagEbdcFee[i][j] = (TH1*)file->Get(name.str().c_str());
	  nInttTaggedBCOsFee[i][j] = inttTagEbdcFee[i][j]->GetEntries();

	}
    }

  nInttTaggedBCOs[8] = inttallpackets->GetEntries();
  float inttebdc[ninttebdcs+1];
  float inttebdc_fee[ninttebdcs][ninttfees];
  float inttebdcfrac[ninttebdcs+1];
  float inttebdcfeefrac[ninttebdcs][ninttfees];
  TGraph *inttebdcfeegr[ninttebdcs];
  inttebdcfrac[8] = (float)nInttTaggedBCOs[8] / nInttGL1BCOs;
  inttebdc[8] = 8;
  for(int i = 0; i< ninttebdcs; i++)
    {
      inttebdc[i] = i;
      inttebdcfrac[i] = (float)nInttTaggedBCOs[i] / nInttGL1BCOs;
     
      for(int j=0; j<ninttfees; j++)
	{
	  inttebdc_fee[i][j] = j;
	  inttebdcfeefrac[i][j] = (float)nInttTaggedBCOsFee[i][j] / nInttGL1BCOs;
	 
	}
      inttebdcfeegr[i] = new TGraph(ninttfees, inttebdc_fee[i], inttebdcfeefrac[i]);
    }

  TH1* histoversion = new TH1F("inttsummary",";;Fraction Tagged GL1 BCOs;", 9,0,9);
  TH1 *histoversionfees = new TH1F("inttsummaryfees",";Server; Fraction All FEEs Tagged GL1 BCO",9,0,9);
  histoversionfees->GetYaxis()->SetRangeUser(0.8,1.2);
  for(int i=0; i<ninttebdcs+1; i++)
    {
      histoversion->SetBinContent(i+1, inttebdcfrac[i]);
      histoversionfees->SetBinContent(i+1, nInttTaggedBCOsServers[i]); 
    }
  histoversionfees->SetBinContent(9,(float) inttallpacketsallfees->GetEntries() / nInttGL1BCOs);
  histoversion->SetBinContent(9, inttebdcfrac[8]);
  TCanvas *can = new TCanvas("can","can",200,200,600,600);

  gStyle->SetOptStat(0);
  histoversion->GetYaxis()->SetRangeUser(0,1);
  histoversion->GetXaxis()->SetBinLabel(1,"0");
  histoversion->GetXaxis()->SetBinLabel(2,"1");
  histoversion->GetXaxis()->SetBinLabel(3,"2");
  histoversion->GetXaxis()->SetBinLabel(4,"3");

  histoversion->GetXaxis()->SetBinLabel(5,"4");  
  histoversion->GetXaxis()->SetBinLabel(6,"5");
  histoversion->GetXaxis()->SetBinLabel(7,"6");
  histoversion->GetXaxis()->SetBinLabel(8,"7");
  histoversion->GetXaxis()->SetBinLabel(9,"All");
  
  histoversion->GetXaxis()->SetTitle("Server");
  histoversion->Draw("hist");
  myText(0.22,0.5,kBlack,"#bf{#it{sPHENIX}} internal");
  myText(0.22,0.42,kBlack,"44681, 200 events");

  TCanvas *canfee = new TCanvas("canfee","canfee",200,200,900,900);
  gStyle->SetOptStat(0);
  histoversionfees->GetYaxis()->SetRangeUser(0.8,1.2);
  histoversionfees->GetXaxis()->SetBinLabel(1,"0");
  histoversionfees->GetXaxis()->SetBinLabel(2,"1");
  histoversionfees->GetXaxis()->SetBinLabel(3,"2");
  histoversionfees->GetXaxis()->SetBinLabel(4,"3");
  histoversionfees->GetXaxis()->SetBinLabel(5,"4");
  histoversionfees->GetXaxis()->SetBinLabel(6,"5");
  histoversionfees->GetXaxis()->SetBinLabel(7,"6");
  histoversionfees->GetXaxis()->SetBinLabel(8,"7");
  histoversionfees->GetXaxis()->SetBinLabel(9,"All");
  histoversionfees->Draw("hist");
  myText(0.2,0.88,kBlack,"#bf{#it{sPHENIX}} internal");

  myText(0.2,0.81,kBlack,runnumstr.c_str());
  TCanvas *can2 = new TCanvas("can2","can2",200,200,900,900);
  can2->Divide(2,4);
  for(int i=0; i<ninttebdcs; i++)
    {
      can2->cd(i+1);
      inttebdcfeegr[i]->GetXaxis()->SetTitle("FEE ID");
      inttebdcfeegr[i]->GetYaxis()->SetTitle("Fraction Tagged GL1BCOs");
      inttebdcfeegr[i]->GetYaxis()->SetRangeUser(0,1);
      inttebdcfeegr[i]->Draw("ap");
      name.str("");
      name << "Server " << i;
      myText(0.2,0.25,kBlack,name.str().c_str(),0.07);
    }
    myText(0.2,0.88,kBlack,"#bf{#it{sPHENIX}} internal");

   myText(0.2,0.81,kBlack,runnumstr.c_str());

    }

  /**
   *
   *
   *
   * TPC
   *
   *
   */
  
  if(tpc)
    {
  TH1* tpcrefgl1 = (TH1*)file->Get("h_TpcPoolQA_RefGL1BCO");
  TH1 *tpctagall = (TH1*)file->Get("h_TpcPoolQA_TagBCOAllPackets");

  const int nTpcGL1BCOs = tpcrefgl1->GetEntries();
  TH1* tpcTagEbdc[ntpcebdcs][ntpcpackets];
  int nTpcTagEbdc[ntpcebdcs][ntpcpackets];
  float tpcTagEbdcFrac[ntpcebdcs][ntpcpackets];
  float tpcEbdcPacket[ntpcpackets];
  TGraph *tpcEbdcFracGr[ntpcebdcs];
  for(int i=0; i<ntpcebdcs; i++)
    {
      for(int j=0; j<ntpcpackets; j++)
	{
	  name.str("");
	  name << "h_TpcPoolQA_TagBCO_ebdc"<<i<<"_packet"<<j;
	  tpcTagEbdc[i][j] = (TH1*)file->Get(name.str().c_str());
	  nTpcTagEbdc[i][j] = tpcTagEbdc[i][j]->GetEntries();
	  tpcTagEbdcFrac[i][j] = (float)nTpcTagEbdc[i][j] / nTpcGL1BCOs;
	  tpcEbdcPacket[j] = j;
	  
	}
      tpcEbdcFracGr[i] = new TGraph(ntpcpackets, tpcEbdcPacket, tpcTagEbdcFrac[i]);
    }
  TH1 *tpchistosummary = new TH1F("tpchistosummary",";Packet ID;Fraction Tagged GL1 BCOs", 49,0,49);
  for(int i=0; i<48; i++)
    {
      int ebdc = i / 2;
      tpchistosummary->SetBinContent(i+1, tpcTagEbdcFrac[ebdc][i%2]);
    }
  
  tpchistosummary->SetBinContent(49, tpctagall->GetEntries() / nTpcGL1BCOs);
  
  TCanvas *can3 = new TCanvas("can3","can3",800,800);
  gStyle->SetOptStat(0);
  tpchistosummary->GetYaxis()->SetRangeUser(0,1);
  tpchistosummary->GetXaxis()->SetBinLabel(1, "4000");
  tpchistosummary->GetXaxis()->SetBinLabel(2, "4001");
  tpchistosummary->GetXaxis()->SetBinLabel(3, "4010");
  tpchistosummary->GetXaxis()->SetBinLabel(4, "4011");
  tpchistosummary->GetXaxis()->SetBinLabel(5, "4020");
  tpchistosummary->GetXaxis()->SetBinLabel(6, "4021");
  tpchistosummary->GetXaxis()->SetBinLabel(7, "4030");
  tpchistosummary->GetXaxis()->SetBinLabel(8, "4031");
  tpchistosummary->GetXaxis()->SetBinLabel(9, "4040");
  tpchistosummary->GetXaxis()->SetBinLabel(10, "4041");
  tpchistosummary->GetXaxis()->SetBinLabel(11, "4050");
  
  tpchistosummary->GetXaxis()->SetBinLabel(12, "4051");
  tpchistosummary->GetXaxis()->SetBinLabel(13, "4060");
  tpchistosummary->GetXaxis()->SetBinLabel(14, "4061");
  tpchistosummary->GetXaxis()->SetBinLabel(15, "4070");
  tpchistosummary->GetXaxis()->SetBinLabel(16, "4071");
  tpchistosummary->GetXaxis()->SetBinLabel(17, "4080");
  tpchistosummary->GetXaxis()->SetBinLabel(18, "4081");
  tpchistosummary->GetXaxis()->SetBinLabel(19, "4090");
  tpchistosummary->GetXaxis()->SetBinLabel(20, "4091");
  tpchistosummary->GetXaxis()->SetBinLabel(21, "4100");
  tpchistosummary->GetXaxis()->SetBinLabel(22, "4101");
  tpchistosummary->GetXaxis()->SetBinLabel(23, "4110");
  
  tpchistosummary->GetXaxis()->SetBinLabel(24, "4111");
  tpchistosummary->GetXaxis()->SetBinLabel(25, "4120");
  tpchistosummary->GetXaxis()->SetBinLabel(26, "4121");
  tpchistosummary->GetXaxis()->SetBinLabel(27, "4130");
  tpchistosummary->GetXaxis()->SetBinLabel(28, "4131");
  
  tpchistosummary->GetXaxis()->SetBinLabel(29, "4140");
  tpchistosummary->GetXaxis()->SetBinLabel(30, "4141");
  tpchistosummary->GetXaxis()->SetBinLabel(31, "4150");
  tpchistosummary->GetXaxis()->SetBinLabel(32, "4151");
  tpchistosummary->GetXaxis()->SetBinLabel(33, "4160");
  tpchistosummary->GetXaxis()->SetBinLabel(34, "4161");
  
  tpchistosummary->GetXaxis()->SetBinLabel(35, "4170");
  tpchistosummary->GetXaxis()->SetBinLabel(36, "4171");
  tpchistosummary->GetXaxis()->SetBinLabel(37, "4180");
  tpchistosummary->GetXaxis()->SetBinLabel(38, "4181");
  tpchistosummary->GetXaxis()->SetBinLabel(39, "4190");
  tpchistosummary->GetXaxis()->SetBinLabel(40, "4191");
  tpchistosummary->GetXaxis()->SetBinLabel(41, "4200");
  tpchistosummary->GetXaxis()->SetBinLabel(42, "4201");
  tpchistosummary->GetXaxis()->SetBinLabel(43, "4210");
  tpchistosummary->GetXaxis()->SetBinLabel(44, "4211");
  tpchistosummary->GetXaxis()->SetBinLabel(45, "4220");
  tpchistosummary->GetXaxis()->SetBinLabel(46, "4221");
  tpchistosummary->GetXaxis()->SetBinLabel(47, "4230");
  tpchistosummary->GetXaxis()->SetBinLabel(48, "4231");
  tpchistosummary->GetXaxis()->SetBinLabel(49, "All");
  tpchistosummary->GetXaxis()->SetLabelSize(0.03);
  tpchistosummary->GetXaxis()->LabelsOption("v");
  tpchistosummary->Draw("hist");
  myText(0.2,0.88,kBlack,"#bf{#it{sPHENIX}} internal");

  myText(0.2,0.81,kBlack,runnumstr.c_str());
    }

    /**
     *
     *
     * MVTX
     *
     *
     *
     */

  if(mvtx)
    {
  TH1* mvtxrefgl1 = (TH1*)file->Get("h_MvtxPoolQA_RefGL1BCO");
  TH1* mvtxallpackets = (TH1*)file->Get("h_MvtxPoolQA_TagBCOAllFelixs");
  TH1 *mvtxallpacketsallfees = (TH1*)file->Get("h_MvtxPoolQA_TagBCOAllFelixsAllFees");

   const int nMvtxGL1BCOs = mvtxrefgl1->GetEntries();
  TH1* mvtxTagFelix[nmvtxfelix][nmvtxfees];
  int nMvtxTagFelix[nmvtxfelix][nmvtxfees];
  float mvtxTagFelixFrac[nmvtxfelix][nmvtxfees];
  float mvtxTagFelixx[nmvtxfelix];
  float mvtxTagFelixxFrac[nmvtxfelix];
  float mvtxTagAllFelixFrac[nmvtxfelix];
  float mvtxFelixPacket[nmvtxfees];
  TGraph *mvtxFelixFracGr[nmvtxfelix];
  for(int i=0; i<nmvtxfelix; i++)
    {
      name.str("");
      name << "h_MvtxPoolQA_TagBCO_felix"<<i;
      mvtxTagFelixx[i] = ((TH1*)file->Get(name.str().c_str()))->GetEntries();
      mvtxTagFelixxFrac[i] = mvtxTagFelixx[i] / nMvtxGL1BCOs;

      name.str("");
      name << "h_MvtxPoolQA_TagBCOAllFees_Felix"<<i;
      mvtxTagAllFelixFrac[i]  = (((TH1*)file->Get(name.str().c_str()))->GetEntries()) / nMvtxGL1BCOs;
     
      /* 
      for(int j=0; j<nmvtxfees; j++)
	{
	  name.str("");
	  name << "h_MvtxPoolQA_TagBCO_felix"<<i<<"_fee"<<j;
	  mvtxTagFelix[i][j] = (TH1*)file->Get(name.str().c_str());
	  nMvtxTagFelix[i][j] = mvtxTagFelix[i][j]->GetEntries();
	  
	  mvtxTagFelixFrac[i][j] = (float)nMvtxTagFelix[i][j] / nMvtxGL1BCOs;
	  mvtxFelixPacket[j] = j;
	  
	}
      mvtxFelixFracGr[i] = new TGraph(nmvtxfees, mvtxFelixPacket, mvtxTagFelixFrac[i]);
      */
    }

  TH1F* mvtxsummaryhisto = new TH1F("mvtxsummaryhisto",";Felix.Endpoint;Fraction Tagged GL1 BCOs",13,0,13);
  for(int i=0; i<nmvtxfelix; i++)
    {
      mvtxsummaryhisto->SetBinContent(i+1, mvtxTagFelixxFrac[i]);
    }
  mvtxsummaryhisto->SetBinContent(13, mvtxallpackets->GetEntries() / nMvtxGL1BCOs);

  

  TH1F* mvtxAllFelixSummary = new TH1F("mvtxfelixsummaryhisto",";Felix.Endpoint; Fraction All FEEs Tagged GL1 BCOs",13,0,13);
  mvtxAllFelixSummary->GetYaxis()->SetRangeUser(0.8,1.2);
  for(int i=0; i<nmvtxfelix; i++)
    {
      mvtxAllFelixSummary->SetBinContent(i+1, mvtxTagAllFelixFrac[i]);
    }
  mvtxAllFelixSummary->SetBinContent(13, mvtxallpacketsallfees->GetEntries() / nMvtxGL1BCOs);

  TCanvas *mvtxfeesumcan = new TCanvas("mvtsfeesumcan","mvtxfeesumcan",200,200,600,600);
  gStyle->SetOptStat(0);
   mvtxAllFelixSummary->GetXaxis()->SetBinLabel(1,"0.0");
  mvtxAllFelixSummary->GetXaxis()->SetBinLabel(2,"0.1");
  mvtxAllFelixSummary->GetXaxis()->SetBinLabel(3,"1.0");
  mvtxAllFelixSummary->GetXaxis()->SetBinLabel(4,"1.1");
  mvtxAllFelixSummary->GetXaxis()->SetBinLabel(5,"2.0");
  mvtxAllFelixSummary->GetXaxis()->SetBinLabel(6,"2.1");
  mvtxAllFelixSummary->GetXaxis()->SetBinLabel(7,"3.0");
  mvtxAllFelixSummary->GetXaxis()->SetBinLabel(8,"3.1");
  mvtxAllFelixSummary->GetXaxis()->SetBinLabel(9,"4.0");
  mvtxAllFelixSummary->GetXaxis()->SetBinLabel(10,"4.1");
  mvtxAllFelixSummary->GetXaxis()->SetBinLabel(11,"5.0");
  mvtxAllFelixSummary->GetXaxis()->SetBinLabel(12,"5.1");
  mvtxAllFelixSummary->GetXaxis()->SetBinLabel(13,"All");
  mvtxAllFelixSummary->Draw("hist");
  myText(0.2,0.88,kBlack,"#bf{#it{sPHENIX}} internal");

  myText(0.2,0.81,kBlack,runnumstr.c_str());
  

  TCanvas *mvtxsumcan = new TCanvas("mvtxsumcan","mvtxsumcan",200,200,600,600);
  gStyle->SetOptStat(0);
  
  mvtxsummaryhisto->GetYaxis()->SetRangeUser(0,1);

  mvtxsummaryhisto->GetXaxis()->SetBinLabel(1,"0.0");
  mvtxsummaryhisto->GetXaxis()->SetBinLabel(2,"0.1");
  mvtxsummaryhisto->GetXaxis()->SetBinLabel(3,"1.0");
  mvtxsummaryhisto->GetXaxis()->SetBinLabel(4,"1.1");
  mvtxsummaryhisto->GetXaxis()->SetBinLabel(5,"2.0");
  mvtxsummaryhisto->GetXaxis()->SetBinLabel(6,"2.1");
  mvtxsummaryhisto->GetXaxis()->SetBinLabel(7,"3.0");
  mvtxsummaryhisto->GetXaxis()->SetBinLabel(8,"3.1");
  mvtxsummaryhisto->GetXaxis()->SetBinLabel(9,"4.0");
  mvtxsummaryhisto->GetXaxis()->SetBinLabel(10,"4.1");
  mvtxsummaryhisto->GetXaxis()->SetBinLabel(11,"5.0");
  mvtxsummaryhisto->GetXaxis()->SetBinLabel(12,"5.1");
  mvtxsummaryhisto->GetXaxis()->SetBinLabel(13,"All");
  mvtxsummaryhisto->Draw("hist");
  myText(0.2,0.88,kBlack,"#bf{#it{sPHENIX}} internal");
    
  myText(0.2,0.81,kBlack,runnumstr.c_str());  
/*
  TCanvas *mvtxcan = new TCanvas("mvtxcan","mvtxcan",200,200,700,700);
  mvtxcan->Divide(3,4);

  for(int i=0; i<nmvtxfelix; i++)
    {
      mvtxcan->cd(i+1);

      mvtxFelixFracGr[i]->GetYaxis()->SetRangeUser(0,1);
      mvtxFelixFracGr[i]->GetXaxis()->SetTitle("FEE");
      mvtxFelixFracGr[i]->GetYaxis()->SetTitle("Fraction Tagged GTM BCOs");
      mvtxFelixFracGr[i]->Draw("ap");
      name.str("");
      name << "Felix.Endpoint " << i/2 << "." << i%2;
      myText(0.22,0.4,kBlack,name.str().c_str());
    }
  */
    }
  
}
