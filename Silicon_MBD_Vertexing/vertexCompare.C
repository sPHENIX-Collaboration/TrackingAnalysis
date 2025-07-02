TGaxis::SetMaxDigits(3);

string plotDir = "./";
string runNumber = "68626";

template <typename T>
string to_string_with_precision(const T a_value, const int n = 0)
{
    ostringstream out;
    out.precision(n);
    out << fixed << a_value;
    return out.str();
}

template <typename T>
void savePlots(T myPlot, string typeCompare, float fitMin = -1.5, float fitMax = 1.5)
{
  TCanvas *c1  = new TCanvas("myCanvas", "myCanvas",800,800);

  myPlot.GetXaxis()->SetNdivisions(505);
  myPlot.GetYaxis()->SetNdivisions(505);

  float labelPosition[4];

  string gauss = "[0]*Gaus(x, [1], [2])";
  TF1* fitFunc = new TF1 ("f", gauss.c_str(), fitMin, fitMax);
  fitFunc->SetLineColor(kRed);
  fitFunc->SetParName(0, "Const"); fitFunc->SetParameter(0, 10); fitFunc->SetParLimits(0, 0, 10000);
  fitFunc->SetParName(1, "Mean");  fitFunc->SetParameter(1, 0.0); fitFunc->SetParLimits(1, -10, 10);
  fitFunc->SetParName(2, "Width"); fitFunc->SetParameter(2, 1);  fitFunc->SetParLimits(2, 1e-3, 10);

  if (strncmp(typeid(myPlot).name(), "4TH2F", 5) == 0)
  {
    myPlot.Draw("COL");
    labelPosition[0] = 0.15;
    labelPosition[1] = 0.90;
    labelPosition[2] = 0.95;
    labelPosition[3] = 1.00;
  }
  else
  {
    myPlot.SetMaximum(1.6*myPlot.GetMaximum());
    myPlot.Sumw2();
    myPlot.Draw("PE1");

    myPlot.Fit(fitFunc, "ER");

    gPad->Update();
    labelPosition[0] = 0.2;
    labelPosition[1] = 0.70;
    labelPosition[2] = 0.55;
    labelPosition[3] = 0.88;
  }

  TPaveText *pt;
  pt = new TPaveText(labelPosition[0], labelPosition[1], labelPosition[2], labelPosition[3], "NDC");
  pt->SetFillColor(0);
  pt->SetFillStyle(0);
  pt->SetTextFont(42);
  TText *pt_LaTex;
  string labelString = "#it{#bf{sPHENIX}} Internal, run " + runNumber;
  pt_LaTex = pt->AddText(labelString.c_str());

  if (strncmp(typeid(myPlot).name(), "4TH1F", 5) == 0)
  {
    double fitMean = fitFunc->GetParameter(1);
    double fitMeanError = fitFunc->GetParError(1);
    double fitWidth = fitFunc->GetParameter(2);
    double fitWidthError = fitFunc->GetParError(2);
    int precision = 2;

    string mean_string = "#mu = " + to_string_with_precision(fitMean, precision) + "#pm" + to_string_with_precision(fitMeanError, precision) + " cm";
    string width_string = "#sigma = " + to_string_with_precision(fitWidth, precision) + "#pm" + to_string_with_precision(fitWidthError, precision) + " cm";

    pt_LaTex = pt->AddText(mean_string.c_str());
    pt_LaTex = pt->AddText(width_string.c_str());
  }

  pt->SetBorderSize(0);
  pt->Draw();
  gPad->Modified();

  string extensions[] = {".pdf", ".png"};
  for (auto extension : extensions)
  {
    string output = plotDir + "vtxCompare_run_" + runNumber  + typeCompare + extension;
    c1->SaveAs(output.c_str());
  }

  delete c1;
}

TH1F makeHisto(int nBins, float min, float max, string xAxisTitle, string unit, int precision)
{
  TH1F myHisto(xAxisTitle.c_str(), xAxisTitle.c_str(), nBins, min, max);

  if (unit != "") xAxisTitle += " [" + unit +  "]";
  myHisto.GetXaxis()->SetTitle(xAxisTitle.c_str());

  float binWidth = (float) (max - min)/nBins;
  string yAxisTitle;
  if (unit != "") yAxisTitle = "Entries / " + to_string_with_precision(binWidth, precision) + " " + unit;
  else yAxisTitle = "Entries";
  myHisto.GetYaxis()->SetTitle(yAxisTitle.c_str());

  return myHisto;
}

TH2F makeHisto(int x_nBins, float x_min, float x_max, string xAxisTitle, string x_unit, int y_nBins, float y_min, float y_max, string yAxisTitle, string y_unit, string title)
{
  TH2F myHisto(title.c_str(), title.c_str(), x_nBins, x_min, x_max, y_nBins, y_min, y_max);

  if (x_unit != "") xAxisTitle += " [" + x_unit +  "]";
  myHisto.GetXaxis()->SetTitle(xAxisTitle.c_str());

  if (y_unit != "") yAxisTitle += " [" + y_unit +  "]";
  myHisto.GetYaxis()->SetTitle(yAxisTitle.c_str());

  return myHisto;
}

void vertexCompare(string inputRunNumber = "68626", string outDir = "./")
{
  runNumber = inputRunNumber;

  plotDir = outDir + "/plots/";
  string fileDir = outDir + "/files/";
  
  string fileName = fileDir + "outputVTX.root";
  TFile* file = new TFile(fileName.c_str());
  TTree* tree = (TTree*)file->Get("VTX");
 
  string cutString = "hasTRK && hasMBD";

  string plotTitle = "vtxMap";
  TH2F vtxMap = makeHisto(100, -15, 15, "Tracker z vertex", "cm", 100, -15, 15, "MBD  z vertex", "cm", plotTitle);
  string branch = "mbdVertex:trackerVertex >> " + plotTitle;
  tree->Draw(branch.c_str(), cutString.c_str());
  savePlots(vtxMap, "_correlation");

  string xAxisTitle = "MBD - Silicon z vertex";
  TH1F vtxDiff = makeHisto(40, -10, 10, xAxisTitle, "cm", 1);
  branch = "mbdVertex - trackerVertex >> " + xAxisTitle;
  tree->Draw(branch.c_str(), cutString.c_str());
  savePlots(vtxDiff, "_diff");

  xAxisTitle = "Silicon z vertex";
  TH1F vtxSil = makeHisto(40, -15, 15, xAxisTitle, "cm", 1);
  branch = "trackerVertex >> " + xAxisTitle;
  tree->Draw(branch.c_str(), "hasTRK");
  savePlots(vtxSil, "_trk", -10, 10);

  xAxisTitle = "MBD z vertex";
  TH1F vtxMBD = makeHisto(40, -15, 15, xAxisTitle, "cm", 1);
  branch = "mbdVertex >> " + xAxisTitle;
  tree->Draw(branch.c_str(), "hasMBD");
  savePlots(vtxMBD, "_mbd", -10, 10);
}
