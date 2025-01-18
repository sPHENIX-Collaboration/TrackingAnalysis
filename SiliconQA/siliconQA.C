
#include <ffamodules/CDBInterface.h>

#include <phool/recoConsts.h>
#include <cdbobjects/CDBTTree.h>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>

R__LOAD_LIBRARY(libffamodules.so)
R__LOAD_LIBRARY(libphool.so)
R__LOAD_LIBRARY(libcdbobjects.so)

float ACCEPTANCE_THRESHOLD = 0.875;
float ACCEPTANCE_RMS_THRESHOLD = 0.8125;

void getLTSRegStdDev(TGraphErrors *graph, float fit, float &stddev) 
{
  int N = graph->GetN();
  std::vector<std::pair<double, double>> residuals(N);

  for (int i = 0; i < N; i++) {
      double x, y;
      graph->GetPoint(i, x, y);
      double residual = fabs(y - fit);
      residuals[i].first = residual;
      residuals[i].second = y;
  }

  std::sort(residuals.begin(), residuals.end(), [](const auto &a, const auto &b) {
      return a.first < b.first;
  });

  int trimmedSize = N * 0.9;
  double sumSquared = 0;
  for (int i = 0; i < trimmedSize; i++) {
      sumSquared += residuals[i].first * residuals[i].first;
  }

  stddev = std::sqrt(sumSquared / trimmedSize);
}

// Note that many runs from 53332-53485 are not in lustre (just in hpss) so the QAhtml is missing from here
std::tuple<bool, float, float> inttQA(TFile* qafile, int run)
{
  std::tuple <bool, float, float> tup_intt;

  // get bco diff qa from cdb (must be 23 for INTT streaming mode)
  bool intt_bco_diff_qa = true;

  auto rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER", run);
  rc->set_StringFlag("CDB_GLOBALTAG", "ProdA_2024");
  rc->set_uint64Flag("TIMESTAMP", run);

  std::string intt_bco_calib_dir = CDBInterface::instance()->getUrl("INTT_BCOMAP");
  if (intt_bco_calib_dir.empty())
  {
    std::cout << "No INTT BCOMAP for Run " << run << std::endl;
    intt_bco_diff_qa = false;
  }
  else
  {
    CDBTTree *cdbttree = new CDBTTree(intt_bco_calib_dir);
    cdbttree->LoadCalibrations();
    uint64_t N = cdbttree->GetSingleIntValue("size");
    for (uint64_t n = 0; n < N; ++n)
    {
      int felix_server = cdbttree->GetIntValue(n, "felix_server");
      int felix_channel = cdbttree->GetIntValue(n, "felix_channel");
      int bco_diff = cdbttree->GetIntValue(n, "bco_diff");
      if (!(bco_diff == 23 || bco_diff == -1))
      {
        intt_bco_diff_qa = false;
        break;
      }
    }
  }

  // Get the 2D INTT chip/channel hit distributions
  // X: Chip on FEE, Y: Channel on FEE
  TGraphErrors *gRMS_Chip = new TGraphErrors();
  TGraphErrors *gRMS_Channel = new TGraphErrors();
  int emptychannels = 0;
  int totchannels = 0;
  TH2I *h_InttRawHitQA_intt[8][14];
  char hintt[128];
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 14; j++)
    {
      sprintf(hintt,"h_InttRawHitQA_intt%d_%d",i,j);
      auto hist = (TH2I*)qafile->Get(hintt);
      if (!hist) 
      {
	      continue;
      }

      h_InttRawHitQA_intt[i][j] = (TH2I*)hist->Clone();
      
      // Get number of dead channels/chips
      for (int x = 0; x < h_InttRawHitQA_intt[i][j]->GetNbinsX(); x++)
      {
        for (int y = 0; y < h_InttRawHitQA_intt[i][j]->GetNbinsY(); y++)
        {
          if (h_InttRawHitQA_intt[i][j]->GetBinContent(x+1,y+1) == 0)
          {
            emptychannels++;
          }
          totchannels++;
        }
      }

      // Get FEE chip/channel RMS
      double effectiveEntries = h_InttRawHitQA_intt[i][j]->GetEffectiveEntries();
      double RMSX = h_InttRawHitQA_intt[i][j]->GetRMS(1);
      double errorRMSX = RMSX / sqrt(2 * effectiveEntries);
      double RMSY = h_InttRawHitQA_intt[i][j]->GetRMS(2);
      double errorRMSY = RMSY / sqrt(2 * effectiveEntries);
      gRMS_Chip->SetPoint(14*i+j,14*i+j,RMSX);
      gRMS_Chip->SetPointError(14*i+j,0,errorRMSX);
      gRMS_Channel->SetPoint(14*i+j,14*i+j,RMSY);
      gRMS_Channel->SetPointError(14*i+j,0,errorRMSY);
    }
  }

  float dead_channels;
  if (totchannels != 0)
  {
    dead_channels = emptychannels/(float)totchannels;
  }
  else
  {
    dead_channels = 1.;
  }
  
  // we create graph copies that remove RMS=0 points (before robust LTS regression fit)
  TGraphErrors *gRMS_Chip_copy = new TGraphErrors();
  TGraphErrors *gRMS_Channel_copy = new TGraphErrors();
  for (int i = 0; i < gRMS_Chip->GetN(); i++) 
  {
    double x_chip, y_chip, x_channel, y_channel;
    gRMS_Chip->GetPoint(i, x_chip, y_chip);
    gRMS_Channel->GetPoint(i, x_channel, y_channel);
    if (y_chip != 0) 
    {
      double ex_chip = gRMS_Chip->GetErrorX(i);
      double ey_chip = gRMS_Chip->GetErrorY(i);
      gRMS_Chip_copy->SetPoint(gRMS_Chip_copy->GetN(), x_chip, y_chip);
      gRMS_Chip_copy->SetPointError(gRMS_Chip_copy->GetN() - 1, ex_chip, ey_chip);
    }
    if (y_channel != 0) 
    {
      double ex_channel = gRMS_Channel->GetErrorX(i);
      double ey_channel = gRMS_Channel->GetErrorY(i);
      gRMS_Channel_copy->SetPoint(gRMS_Channel_copy->GetN(), x_channel, y_channel);
      gRMS_Channel_copy->SetPointError(gRMS_Channel_copy->GetN() - 1, ex_channel, ey_channel);
    }
  }

  float fees_eff_chip;
  float fees_eff_channel;

  // if the copy histograms are empty that means that all FEEs are dead
  if (gRMS_Chip_copy->GetN() == 0 || gRMS_Channel_copy->GetN() == 0) 
  {
    fees_eff_chip = 0.;
    fees_eff_channel = 0.;
  }
  else
  {
    //do the robust LTS (90%) regression fit
    TF1 *ffit_Chip = new TF1("ffit_Chip", "pol0", 0, 112);
    TF1 *ffit_Channel = new TF1("ffit_Channel", "pol0", 0, 112);
    gRMS_Chip_copy->Fit(ffit_Chip, "RSCQ rob=0.90");
    gRMS_Channel_copy->Fit(ffit_Channel, "RSCQ rob=0.90");
    float ffit_chip_par = ffit_Chip->GetParameter(0);
    float ffit_channel_par = ffit_Channel->GetParameter(0);

    // Now we get the standard deviation about the best fit
    float stddev_chip;
    float stddev_channel;
    getLTSRegStdDev(gRMS_Chip_copy, ffit_chip_par, stddev_chip);
    getLTSRegStdDev(gRMS_Channel_copy, ffit_channel_par, stddev_channel);

    // Get the percentage of FEEs with good RMS (within 3sigma bands)
    int badFees_chip = 0;
    int badFees_channel = 0;
    int sigma = 3;
    for (int p = 0; p < gRMS_Chip->GetN(); p++)
    {
      double x_chip_eff, y_chip_eff;
      gRMS_Chip->GetPoint(p,x_chip_eff,y_chip_eff);
      if (fabs(y_chip_eff - ffit_chip_par) > sigma * stddev_chip)
      {
        badFees_chip++;
      }
    }
    for (int q = 0; q < gRMS_Channel->GetN(); q++)
    {
      double x_channel_eff, y_channel_eff;
      gRMS_Channel->GetPoint(q,x_channel_eff,y_channel_eff);
      if (fabs(y_channel_eff - ffit_channel_par) > sigma * stddev_channel)
      {
        badFees_channel++;
      }
    }

    fees_eff_chip = 1-((float)badFees_chip)/gRMS_Chip->GetN();
    fees_eff_channel = 1-((float)badFees_channel)/gRMS_Channel->GetN();

    delete ffit_Chip;
    delete ffit_Channel;
  }

  std::get<0>(tup_intt) = intt_bco_diff_qa;
  std::get<1>(tup_intt) = 1-dead_channels;
  std::get<2>(tup_intt) = (fees_eff_chip+fees_eff_channel)/2;

  qafile->Close();
  delete qafile;

  delete gRMS_Chip;
  delete gRMS_Channel;
  delete gRMS_Chip_copy;
  delete gRMS_Channel_copy;

  return tup_intt;
}


void siliconQA()
{

  std::string inputbasdir = "/sphenix/data/data02/sphnxpro/QAhtml/aggregated";

  //============= output files =============
  std::ofstream inttrunqa("goodruns_intt.txt");
  if (!inttrunqa.is_open()) 
  {
    std::cerr << "Error: Could not open the file for writing!" << std::endl;
    return;
  }

  std::ofstream inttrunqa_ranked("goodruns_intt_rankedbyacceptance.txt");
  if (!inttrunqa_ranked.is_open()) 
  {
    std::cerr << "Error: Could not open the file for writing!" << std::endl;
    return;
  }
  //=========================================

  //============= processed runs =============
  std::unordered_set<int> processed_runs;
  //=========================================

  std::map<int, std::tuple<bool, float, float>> map_inttQA;
  for (const auto &entry : std::filesystem::directory_iterator(inputbasdir))
  {
    std::string inputfile = entry.path().string();
      
    if (inputfile.find("HIST_DST_TRKR_HIT") == std::string::npos)
    {
      continue;
    }

    // get run number from qa file
    std::string runnumber = inputfile.substr(inputfile.find("-000") + 4, 5);
    int run = ::atoi(runnumber.c_str());

    // skip if already processed run
    if (processed_runs.find(run) != processed_runs.end()){continue;}

    // add run to processed runs
    processed_runs.insert(run);

    // For now, limit run selection to 51730-52206, 52469-53880
    if (run < 51730 || (run > 52206 && run < 52469) || run > 53880)
    {
      continue;
    }

    TFile *f_hit = new TFile(inputfile.c_str(), "READ");

    // =========================  Do INTT QA  =========================
    // inttQA() returns a tuple with elements:
    // 0: BCO diff 
    // 1: Good channel/chip fraction
    // 2: Good FEE fraction (from RMS of hits in channels/chips)
    map_inttQA[run] = inttQA(f_hit, run);
    // ================================================================
    
    // QA checks
    cout << "Run: " << run << "\t";
    if (std::get<1>(map_inttQA[run]) < ACCEPTANCE_THRESHOLD || std::get<2>(map_inttQA[run]) < ACCEPTANCE_RMS_THRESHOLD)
    {
      std::cout << "BAD (FEE acceptance)" << std::endl;
    }
    else
    {
      if (!std::get<0>(map_inttQA[run]))
      {
        std::cout << "BAD (FEE BCO requirement)" << std::endl;
      }
      else
      {
        std::cout << "GOOD" << std::endl;
        inttrunqa << run << "\n";
      }
    }

    std::cout << "\% of Live Channels/Chips: " << std::get<1>(map_inttQA[run]) << std::endl;
    std::cout << "\% of FEEs with good RMS: " << std::get<2>(map_inttQA[run]) << std::endl;

  }
  
  inttrunqa.close();
  
  // Rank the runs with the highest acceptance
  std::vector<std::pair<int, std::tuple<bool, float, float>>> rankedGoodRuns(map_inttQA.begin(), map_inttQA.end());

  std::sort(rankedGoodRuns.begin(), rankedGoodRuns.end(),
      [](const std::pair<int, std::tuple<bool, float, float>>& a, const std::pair<int, std::tuple<bool, float, float>>& b) {
          return std::get<1>(a.second)*std::get<2>(a.second) > std::get<1>(b.second)*std::get<2>(b.second);
      });

  for (const auto& [key, value] : rankedGoodRuns) 
  {
      inttrunqa_ranked << key << " " << std::get<1>(value) << " " << std::get<2>(value) << "\n";
  }
  inttrunqa_ranked.close();
  

}
