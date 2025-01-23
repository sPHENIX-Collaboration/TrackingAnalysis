#include <SiliconQA.h>

#include <cdbobjects/CDBTTree.h>
#include <ffamodules/CDBInterface.h>

#include <phool/phool.h>
#include <phool/recoConsts.h>

#include <TF1.h>
#include <TH2.h>
#include <TH1F.h>
#include <TGraphErrors.h>
#include <TFile.h>


#include <fstream>
#include <filesystem>
#include <string>


SiliconQA::SiliconQA()
{
}

void SiliconQA::GetQAhtml()
{
  for (const auto &entry : std::filesystem::directory_iterator(_inputbasedir))
  {
    std::string inputfile_hit = entry.path().string();
    
    if (inputfile_hit.find("HIST_DST_TRKR_HIT") == std::string::npos)
    {
    continue;
    }

    // get run number from qa file
    std::string s_runnumber = inputfile_hit.substr(inputfile_hit.find("-000") + 4, 5);
    int run = ::atoi(s_runnumber.c_str());


    // If goldenruns: limit run selection to 51730-52206, 52469-53880
    if (b_goldenruns)
    {
      if (run < 51730 || (run > 52206 && run < 52469) || run > 53880)
      {
        continue;
      }
    }

    // skip if already processed run
    if (processed_runs.find(run) != processed_runs.end()){continue;}

    // add run to processed runs
    processed_runs.insert(run);

    auto pos = inputfile_hit.find("HIST_DST_TRKR_HIT");
    std::string inputfile_clust = inputfile_hit;
    inputfile_clust.replace(pos, std::string("HIST_DST_TRKR_HIT").size(), "HIST_DST_TRKR_CLUSTER");

    if (!std::filesystem::exists(inputfile_clust))
    {
    continue;
    }

    map_inputfile_hit[run] = inputfile_hit;
    map_inputfile_clust[run] = inputfile_clust;
  }

}

void SiliconQA::doQA()
{
  std::vector<int> runlist(processed_runs.begin(), processed_runs.end());
  for (const auto &run : runlist)
  {
    runnumber = run;

    TFile *f_hit = new TFile(map_inputfile_hit[runnumber].c_str(), "READ");
    TFile *f_clust = new TFile(map_inputfile_clust[runnumber].c_str(), "READ");

    // =========================  Do INTT QA  =========================
    // inttQA() returns a tuple with elements:
    // 0: BCO diff 
    // 1: Good channel+chip fraction
    // 2: Good FEE fraction (from RMS of hits in channels/chips)
    auto inttqaresult = inttQA(f_hit);
    if (inttqaresult)
    {
    map_inttQA[runnumber] = *inttqaresult;
    } else 
    {
    continue;
    }
    // ================================================================

    
    // =========================  Do MVTX QA  =========================
    // mvtxQA() returns a tuple with elements:
    // 0: layer 0 good stave+chip fraction
    // 1: layer 1 good stave+chip fraction
    // 2: layer 2 good stave+chip fraction
    // 3: average number of clusters per phi bin
    // 4: modulation amplitude of cosine fit to cluster phi
    // 5: phase offset of cosine fit to cluster phi
    // 6: chi2/NDF of cosine fit to cluster phi
    auto mvtxqaresult = mvtxQA(f_hit, f_clust);
    if (mvtxqaresult)
    {
    map_mvtxQA[runnumber] = *mvtxqaresult;
    } else 
    {
    continue;
    }
    // ================================================================

    // ========================= QA Checks =========================
    std::string statementQA = "";

    // INTT QA
    statementQA += (std::get<0>(map_inttQA[runnumber]) ? 
                        "\u2705 INTT BCO Diff " : "\u274C INTT BCO Diff ");
    statementQA += (std::get<1>(map_inttQA[runnumber]) >= ACCEPTANCE_THRESHOLD ? 
                        "\u2705 INTT Hit Acceptance " : "\u274C INTT Hit Acceptance ");
    statementQA += (std::get<2>(map_inttQA[runnumber]) >= ACCEPTANCE_RMS_THRESHOLD ? 
                        "\u2705 INTT FEE RMS " : "\u274C INTT FEE RMS ");
    // MVTX QA
    statementQA += (std::get<0>(map_mvtxQA[runnumber]) >= layer0hitthresh && 
                        std::get<1>(map_mvtxQA[runnumber]) >= layer1hitthresh &&
                        std::get<2>(map_mvtxQA[runnumber]) >= layer2hitthresh ? 
                        "\u2705 MVTX Hit Acceptance " : "\u274C MVTX Hit Acceptance ");
    statementQA += ((runnumber <= 49961 && std::get<6>(map_mvtxQA[runnumber]) <= scaledchi2ndf_good_b49961) ||
                        (runnumber > 49961 && std::get<6>(map_mvtxQA[runnumber]) <= scaledchi2ndf_good_a49961) ?
                        "\u2705 MVTX chi2/ndf " : "\u274C MVTX chi2/ndf ");
    statementQA += ((runnumber <= 49961 && (std::get<4>(map_mvtxQA[runnumber]) / std::get<3>(map_mvtxQA[runnumber])) >= bovera_low_b49961) ||
                        (runnumber > 49961 && (std::get<4>(map_mvtxQA[runnumber]) / std::get<3>(map_mvtxQA[runnumber])) >= bovera_low_a49961) ?
                        "\u2705 MVTX B/A ratio " : "\u274C MVTX B/A ratio "); 
    statementQA += (std::get<3>(map_mvtxQA[runnumber]) >= avgclus_threshold ?
                        "\u2705 MVTX Run length" : "\u274C MVTX Run length");


    // Group runs by their condition results
    map_allsiliconruns_categories[statementQA].push_back(runnumber);
    if (statementQA == "\u2705 INTT BCO Diff \u2705 INTT Hit Acceptance \u2705 INTT FEE RMS \u2705 MVTX Hit Acceptance \u2705 MVTX chi2/ndf \u2705 MVTX B/A ratio \u2705 MVTX Run length")
    {
    std::get<0>(map_goodsiliconruns[runnumber]) = std::get<1>(map_inttQA[runnumber]);
    std::get<1>(map_goodsiliconruns[run]) = (std::get<0>(map_mvtxQA[run]) + std::get<1>(map_mvtxQA[run]) + std::get<0>(map_mvtxQA[run]))/3;
    }
    
    std::cout << "Run: " << run << "\n" << statementQA << std::endl;

    f_hit->Close();
    f_clust->Close();
    delete f_hit;
    delete f_clust;

  }
}


float SiliconQA::rawHitAcceptance(TH2 *h2)
{
  int totalX = h2->GetNbinsX();
  int totalY = h2->GetNbinsY();
  int emptybins = 0;
  int totbins = 0;
  for (int x = 0; x < totalX; x++)
  {
    for (int y = 0; y < totalY; y++)
    {
      if (h2->GetBinContent(x+1,y+1) == 0)
      {
        emptybins++;
      }
      totbins++;
    }
  }
  float good_bins_eff;
  if (totbins != 0)
  {
    good_bins_eff = 1-emptybins/(float)totbins;
  }
  else
  {
    good_bins_eff = 0.;
  }

  return good_bins_eff;
}


std::tuple<float, float, float, float> SiliconQA::fitClusPhi(TH1F *hm)
{
    // get the average number of clusters per bin
    float avgc = hm->Integral() / hm->GetNbinsX();
    TF1 *f1 = new TF1("f1", "pol0(0)+[1]*cos(x+[2])", -3.14159, 3.14159);
    // set the initial parameters
    f1->SetParameter(0, avgc);
    f1->SetParameter(1, avgc * 0.07);
    f1->SetParameter(2, 9.5);
    f1->SetParLimits(0, avgc*0.5, avgc*1.5);
    f1->SetParLimits(1, avgc*0.001, avgc*0.5);
    f1->SetParLimits(2, 9.3, 9.7);
    f1->SetLineColor(kRed);
    hm->Fit("f1", "R P M Q 0");
    return std::make_tuple(f1->GetParameter(0), f1->GetParameter(1), f1->GetParameter(2), (f1->GetChisquare() / f1->GetParameter(0)) / f1->GetNDF());
}


void SiliconQA::getLTSRegStdDev(TGraphErrors *graph, float fit, float &stddev) 
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

  // for 90% LTS 
  int trimmedSize = N * 0.9;
  double sumSquared = 0;
  for (int i = 0; i < trimmedSize; i++) {
      sumSquared += residuals[i].first * residuals[i].first;
  }

  stddev = std::sqrt(sumSquared / trimmedSize);
}


std::optional<std::tuple<bool, float, float>> SiliconQA::inttQA(TFile* qafile)
{
  
  std::tuple <bool, float, float> tup_intt;

  // get bco diff qa from cdb (must be 23 for INTT streaming mode)
  bool intt_bco_diff_qa = true;

  auto rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER", runnumber);
  rc->set_StringFlag("CDB_GLOBALTAG", "ProdA_2024");
  rc->set_uint64Flag("TIMESTAMP", runnumber);

  std::string intt_bco_calib_dir = CDBInterface::instance()->getUrl("INTT_BCOMAP");
  if (intt_bco_calib_dir.empty())
  {
    std::cout << "No INTT BCOMAP for Run " << runnumber << std::endl;
    intt_bco_diff_qa = false;
  }
  else
  {
    CDBTTree *cdbttree = new CDBTTree(intt_bco_calib_dir);
    cdbttree->LoadCalibrations();
    uint64_t N = cdbttree->GetSingleIntValue("size");
    for (uint64_t n = 0; n < N; ++n)
    {
      int bco_diff = cdbttree->GetIntValue(n, "bco_diff");
      if (!(bco_diff == BCO_DIFF_VALUE || bco_diff == -1))
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
  float good_channel_eff = 0.;
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
      good_channel_eff += rawHitAcceptance(h_InttRawHitQA_intt[i][j])/(8*14);
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
  // if the RMS graphs are empty that means that all raw hit histograms do not exist
  if (gRMS_Chip->GetN() == 0 || gRMS_Channel->GetN() == 0) 
  {
    std::cout << "inttQA histograms are missing." << std::endl;
    return std::nullopt;
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

  // if the copy graphs are empty that means that all FEEs are dead
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
  std::get<1>(tup_intt) = good_channel_eff;
  std::get<2>(tup_intt) = (fees_eff_chip+fees_eff_channel)/2;

  delete gRMS_Chip;
  delete gRMS_Channel;
  delete gRMS_Chip_copy;
  delete gRMS_Channel_copy;

  return tup_intt;
}

std::optional<std::tuple<float, float, float, float, float, float, float>> SiliconQA::mvtxQA(TFile* qafile_hit, TFile *qafile_clust)
{
  std::tuple<float, float, float, float, float, float, float> tup_mvtx;

  TH2F *h_MvtxRawHitQA_nhits_stave_chip_layer0 = dynamic_cast<TH2F *>(qafile_hit->Get("h_MvtxRawHitQA_nhits_stave_chip_layer0"));
  TH2F *h_MvtxRawHitQA_nhits_stave_chip_layer1 = dynamic_cast<TH2F *>(qafile_hit->Get("h_MvtxRawHitQA_nhits_stave_chip_layer1"));
  TH2F *h_MvtxRawHitQA_nhits_stave_chip_layer2 = dynamic_cast<TH2F *>(qafile_hit->Get("h_MvtxRawHitQA_nhits_stave_chip_layer2"));
  TH1F *h_MvtxClusterQA_clusterPhi_incl = dynamic_cast<TH1F *>(qafile_clust->Get("h_MvtxClusterQA_clusterPhi_incl"));
  TH1F *h_MvtxClusterQA_chipOccupancy = dynamic_cast<TH1F *>(qafile_clust->Get("h_MvtxClusterQA_chipOccupancy"));


  if (!h_MvtxRawHitQA_nhits_stave_chip_layer0 || !h_MvtxRawHitQA_nhits_stave_chip_layer1 || 
      !h_MvtxRawHitQA_nhits_stave_chip_layer2 || !h_MvtxClusterQA_clusterPhi_incl ||
      !h_MvtxClusterQA_chipOccupancy)
  {
      std::cerr << "One of the histograms mvtxQA histograms is missing." << std::endl;
      return std::nullopt;

  }

  std::get<0>(tup_mvtx) = rawHitAcceptance(h_MvtxRawHitQA_nhits_stave_chip_layer0);
  std::get<1>(tup_mvtx) = rawHitAcceptance(h_MvtxRawHitQA_nhits_stave_chip_layer1);
  std::get<2>(tup_mvtx) = rawHitAcceptance(h_MvtxRawHitQA_nhits_stave_chip_layer2);

  std::tuple<float, float, float, float> fitresult;
  fitresult = fitClusPhi(h_MvtxClusterQA_clusterPhi_incl);
  std::get<3>(tup_mvtx) = std::get<0>(fitresult);
  std::get<4>(tup_mvtx) = std::get<1>(fitresult);
  std::get<5>(tup_mvtx) = std::get<2>(fitresult);
  std::get<6>(tup_mvtx) = std::get<3>(fitresult);

  return tup_mvtx;

}

void SiliconQA::WriteMarkdown()
{
  // Make markdown
  std::ofstream f_md(_markdownfilename);
  if (!f_md.is_open()) 
  {
    std::cerr << "Error: Could not open the file for writing!" << std::endl;
  }
  else
  {
    for (auto& [statements, runs] : map_allsiliconruns_categories)
    {
      std::sort(runs.begin(), runs.end());
      f_md << "**" << statements << "**\n\n> ";
      for (int run : runs) {
        f_md << run << " ";
      }
      f_md << "\n\n";
    } 
  }
  f_md.close();
}

void SiliconQA::WriteDatFiles()
{
  // Write out .dat file with good runs
  std::ofstream siliconrunqa(_datfilename);
  if (!siliconrunqa.is_open()) 
  {
    std::cerr << "Error: Could not open the file for writing!" << std::endl;
  }
  else
  {
    for (const auto& [runNumber, values] : map_goodsiliconruns)
    {
      siliconrunqa << runNumber << "\n";
    }
  }

  siliconrunqa.close();
  
  // Write out .dat file with good runs ranked by silicon hit acceptance
  std::ofstream siliconrunqa_ranked(_datrankedfilename);
  if (!siliconrunqa_ranked.is_open()) 
  {
    std::cerr << "Error: Could not open the file for writing!" << std::endl;
    return;
  }
  else
  {
    // Rank the runs with the highest acceptance
    std::vector<std::pair<int, std::tuple<float, float>>> rankedGoodSiliconRuns(map_goodsiliconruns.begin(), map_goodsiliconruns.end());

    std::sort(rankedGoodSiliconRuns.begin(), rankedGoodSiliconRuns.end(),
      [](const std::pair<int, std::tuple<float, float>>& a, const std::pair<int, std::tuple<float, float>>& b) {
          return std::get<0>(a.second)*std::get<1>(a.second) > std::get<0>(b.second)*std::get<1>(b.second);
      });

    for (const auto& [runNumber, acceptances] : rankedGoodSiliconRuns) 
    {
      siliconrunqa_ranked << runNumber << " " << std::get<0>(acceptances) << " " << std::get<1>(acceptances) << "\n";
    }
  }
  siliconrunqa_ranked.close();
}