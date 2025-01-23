#include <TFile.h>
#include <TH1I.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TLegend.h>

void AnalyzeLumiFile(int run_number = 51830) {
	// Open the ROOT file
	//TFile *file = TFile::Open("output_streaming.root");
	TFile *file = TFile::Open(Form("../../rootfiles/run%d_output_gl1p_new.root", run_number));
	if (!file || file->IsZombie()) {
		std::cerr << "Error opening file!" << std::endl;
		return;
	}
	std::string outputDir="QAtimewindow";
	//gSystem->Exec(("mkdir -p \"" + outputDir + "\"").c_str());

	TH1I *h_BunchNumber = dynamic_cast<TH1I *>(file->Get("h_BunchNumber"));
	if (!h_BunchNumber) {
		std::cerr << "Error: h_BunchNumber histogram not found in the file!" << std::endl;
		return;
	} 
	int total_entry = h_BunchNumber->GetEntries();
	std::string title = Form("Run %d, %d Events", run_number, total_entry);
	gStyle->SetOptStat(0);
	if (h_BunchNumber) {
		TCanvas *c1 = new TCanvas("c1", "Bunch Number", 800, 600);
		h_BunchNumber->SetXTitle("Bunch Number");
		h_BunchNumber->SetYTitle("Total Occurrence");
		h_BunchNumber->SetTitle(title.c_str());
		h_BunchNumber->Draw("HIST");
		h_BunchNumber->SetMinimum(0); 
		c1->SaveAs("BunchNumber.pdf");
	} else {
		std::cerr << "h_BunchNumber not found!" << std::endl;
	}

	TH1D *h_BunchNumberOccurance = dynamic_cast<TH1D *>(file->Get("h_BunchNumberOccurance"));
	if (h_BunchNumberOccurance) {
		TCanvas *c12 = new TCanvas("c12", "Bunch Number Occurrence", 800, 600);
		h_BunchNumberOccurance->SetXTitle("Bunch Number");
		h_BunchNumberOccurance->SetYTitle("Total Occurrence");
		h_BunchNumberOccurance->SetTitle(title.c_str());
		h_BunchNumberOccurance->Draw("HIST");
		h_BunchNumberOccurance->SetMinimum(0);
		c12->SaveAs("BunchNumberOccurance.pdf");
	} else {
		std::cerr << "h_BunchNumberOccurance not found!" << std::endl;
	}


	// Retrieve and plot h_LumiBCO
	TH1I *h_LumiBCO = dynamic_cast<TH1I *>(file->Get("h_LumiBCO"));
	if (h_LumiBCO) {
		TCanvas *c2 = new TCanvas("c2", "Lumi BCO", 800, 600);
		h_LumiBCO->SetXTitle("BCO Trim Length");
		h_LumiBCO->SetYTitle("Total Occurrence");
		h_LumiBCO->SetTitle(title.c_str());
		h_LumiBCO->Draw("HIST");
		c2->SaveAs("LumiBCO.pdf");
	} else {
		std::cerr << "h_LumiBCO not found!" << std::endl;
	}

	TH1I *h_diffBCO = dynamic_cast<TH1I *>(file->Get("h_diffBCO"));
	if (h_diffBCO) {
		TCanvas *c31 = new TCanvas("c31", "Lumi BCO", 800, 600);
		h_diffBCO->SetXTitle("diff GL1 BCO");
		h_diffBCO->SetYTitle("Total Occurrence");
		h_diffBCO->SetTitle(title.c_str());
		h_diffBCO->Draw("HIST");
		c31->SaveAs("LumidiffBCO.pdf");
	} else {
		std::cerr << "h_diffBCO not found!" << std::endl;
	} 

	//plot the real luminosity
	TH1D *h_MBDSNraw_BunchID = dynamic_cast<TH1D *>(file->Get("h_MBDSNraw_BunchID"));
        TH1D *h_MBDSNlive_BunchID = dynamic_cast<TH1D *>(file->Get("h_MBDSNlive_BunchID"));
        TH1D *h_MBDSNscaled_BunchID = dynamic_cast<TH1D *>(file->Get("h_MBDSNscaled_BunchID"));
	TH1D *h_rawgl1scalerBunchID = dynamic_cast<TH1D *>(file->Get("h_rawgl1scalerBunchID"));
	double gl1_total_entry = h_rawgl1scalerBunchID->GetBinContent(2)/120.;
        //cout<<gl1_total_entry<<endl;
	if (h_MBDSNraw_BunchID) {
		TCanvas *c41 = new TCanvas("41", "Lumi MBDSN", 800, 600);
		c41->SetLeftMargin(0.15);  // Set left margin to 0.15
		c41->SetRightMargin(0.05); // Set right margin to 0.05
		h_MBDSNraw_BunchID->SetXTitle("Bunch Number");
		h_MBDSNraw_BunchID->SetYTitle("MBDSN raw");
		h_MBDSNraw_BunchID->SetTitle(title.c_str());
		h_MBDSNraw_BunchID->Draw("HIST");
		c41->SaveAs("Lumi_MBDSNraw_BunchID.pdf");

		h_MBDSNlive_BunchID->Draw("HIST");
		h_MBDSNlive_BunchID->SetTitle(title.c_str());
		h_MBDSNlive_BunchID->SetXTitle("Bunch Number");
		h_MBDSNlive_BunchID->SetYTitle("MBDSN live");
		c41->SaveAs("Lumi_MBDSNlive_BunchID.pdf");

		h_MBDSNscaled_BunchID->Draw("HIST");
		h_MBDSNscaled_BunchID->SetTitle(title.c_str());
		h_MBDSNscaled_BunchID->SetXTitle("Bunch Number");
		h_MBDSNscaled_BunchID->SetYTitle("MBDSN scaled");
		c41->SaveAs("Lumi_MBDSNscaled_BunchID.pdf");

		TH1D *h_MBDSN_raw_BunchID = new TH1D("h_MBDSN_raw_BunchID", "h_MBDSN_BunchID", 121, -0.5, 120.5);
		TH1D *h_MBDSN_live_BunchID = new TH1D("h_MBDSN_live_BunchID", "h_MBDSN_BunchID", 121, -0.5, 120.5);
		TH1D *h_MBDSN_scaled_BunchID = new TH1D("h_MBDSN_scaled_BunchID", "h_MBDSN_BunchID", 121, -0.5, 120.5);
		double x_sec_MBDSN = 24.07*1e9;//convert to pb from Vernier scan, 26.5 mb
		for(int i = 0;i<120;i++) {
			h_MBDSN_raw_BunchID->SetBinContent(i, h_MBDSNraw_BunchID->GetBinContent(i)/(x_sec_MBDSN * gl1_total_entry));
                        h_MBDSN_live_BunchID->SetBinContent(i, h_MBDSNlive_BunchID->GetBinContent(i)/(x_sec_MBDSN * gl1_total_entry));
                        h_MBDSN_scaled_BunchID->SetBinContent(i, h_MBDSNscaled_BunchID->GetBinContent(i)/(x_sec_MBDSN * gl1_total_entry));
		}
		TCanvas *c42 = new TCanvas("c42", "Lumi MBDSN", 800, 600);
                c42->SetLeftMargin(0.15);  // Set left margin to 0.15
                c42->SetRightMargin(0.05); // Set right margin to 0.05
		h_MBDSN_raw_BunchID->SetTitle(title.c_str());
		h_MBDSN_raw_BunchID->GetXaxis()->SetTitle("Bunch Number");
		h_MBDSN_raw_BunchID->GetYaxis()->SetTitle("f_{raw} 1/Corss Section per Bunch (1/pb)");
		h_MBDSN_raw_BunchID->Draw("HIST");
		c42->SaveAs("Lumi_MBDSNraw_CrossSection_BunchID.pdf");

                h_MBDSN_live_BunchID->SetTitle(title.c_str());
                h_MBDSN_live_BunchID->GetXaxis()->SetTitle("Bunch Number");
                h_MBDSN_live_BunchID->GetYaxis()->SetTitle("f_{live} 1/Corss Section per Bunch (1/pb)");
                h_MBDSN_live_BunchID->Draw("HIST");
                c42->SaveAs("Lumi_MBDSNlive_CrossSection_BunchID.pdf");
                h_MBDSN_scaled_BunchID->SetTitle(title.c_str());
                h_MBDSN_scaled_BunchID->GetXaxis()->SetTitle("Bunch Number");
                h_MBDSN_scaled_BunchID->GetYaxis()->SetTitle("f_{scaled} 1/Corss Section per Bunch (1/pb)");
                h_MBDSN_scaled_BunchID->Draw("HIST");
                c42->SaveAs("Lumi_MBDSNscaled_CrossSection_BunchID.pdf");


		if(h_BunchNumberOccurance){
			TH1D *h_Scaled_MBDSNraw_BunchID = (TH1D *)h_MBDSN_raw_BunchID->Clone("h_Scaled_MBDSNraw_BunchID");
                        TH1D *h_Scaled_MBDSNlive_BunchID = (TH1D *)h_MBDSN_live_BunchID->Clone("h_Scaled_MBDSNlive_BunchID");
                        TH1D *h_Scaled_MBDSNscaled_BunchID = (TH1D *)h_MBDSN_scaled_BunchID->Clone("h_Scaled_MBDSNscaled_BunchID");
			h_Scaled_MBDSNraw_BunchID->Reset();
                        h_Scaled_MBDSNlive_BunchID->Reset();
                        h_Scaled_MBDSNscaled_BunchID->Reset();

			for (int bin = 1; bin <= 111; ++bin) {
				double content_MBDSNraw = h_MBDSN_raw_BunchID->GetBinContent(bin);
				double content_MBDSNlive = h_MBDSN_live_BunchID->GetBinContent(bin);
				double content_MBDSNscaled = h_MBDSN_scaled_BunchID->GetBinContent(bin);
				double content_BunchOccur = h_BunchNumberOccurance->GetBinContent(bin);
				//double total_time = 30*60.;
				h_Scaled_MBDSNraw_BunchID->SetBinContent(bin, content_MBDSNraw * content_BunchOccur);///total_time );
				h_Scaled_MBDSNlive_BunchID->SetBinContent(bin, content_MBDSNlive * content_BunchOccur);///total_time );
				h_Scaled_MBDSNscaled_BunchID->SetBinContent(bin, content_MBDSNscaled * content_BunchOccur);///total_time );
			}

			TCanvas *c43 = new TCanvas("c43", "Lumi MBDSN Occurance", 800, 600);
			c43->SetLeftMargin(0.15);  // Set left margin to 0.15
			c43->SetRightMargin(0.05); // Set right margin to 0.05

			h_Scaled_MBDSNraw_BunchID->SetTitle(title.c_str());
			h_Scaled_MBDSNraw_BunchID->GetXaxis()->SetTitle("Bunch ID");
			h_Scaled_MBDSNraw_BunchID->GetYaxis()->SetTitle("raw Luminosity (pb)^{-1}");//s^{-1}");
			h_Scaled_MBDSNraw_BunchID->Draw("HIST");
			cout<<"Run "<< run_number<<" Entries "<<total_entry<<" Total Lumi raw "<< h_Scaled_MBDSNraw_BunchID->Integral()<<endl;
			c43->SaveAs("Lumi_Lumiraw_BunchID.pdf");
                        h_Scaled_MBDSNlive_BunchID->SetTitle(title.c_str());
                        h_Scaled_MBDSNlive_BunchID->GetXaxis()->SetTitle("Bunch ID");
                        h_Scaled_MBDSNlive_BunchID->GetYaxis()->SetTitle("live Luminosity (pb)^{-1}");//s^{-1}");
                        h_Scaled_MBDSNlive_BunchID->Draw("HIST");
                        //cout<<"Total Lumi live "<< h_Scaled_MBDSNlive_BunchID->Integral()<<endl;
                        c43->SaveAs("Lumi_Lumilive_BunchID.pdf");
                        h_Scaled_MBDSNscaled_BunchID->SetTitle(title.c_str());
                        h_Scaled_MBDSNscaled_BunchID->GetXaxis()->SetTitle("Bunch ID");
                        h_Scaled_MBDSNscaled_BunchID->GetYaxis()->SetTitle("scaled Luminosity (pb)^{-1}");//s^{-1}");
                        h_Scaled_MBDSNscaled_BunchID->Draw("HIST");
                        //cout<<"Total Lumi scaled "<< h_Scaled_MBDSNscaled_BunchID->Integral()<<endl;
                        c43->SaveAs("Lumi_Lumiscaled_BunchID.pdf");


		}
	}
	return;

	// Retrieve BCOWindowTree and plot bco_trim with two datasets
	TTree *BCOWindowTree = dynamic_cast<TTree *>(file->Get("BCOWindowTree"));
	uint64_t bco_trim, lower_bound, upper_bound;
	// Set branch addresses
	BCOWindowTree->SetBranchAddress("bco_trim", &bco_trim);
	BCOWindowTree->SetBranchAddress("lower_bound", &lower_bound);
	BCOWindowTree->SetBranchAddress("upper_bound", &upper_bound);


	uint64_t bco_trim_min = BCOWindowTree->GetMinimum("bco_trim");
	uint64_t bco_trim_max = BCOWindowTree->GetMaximum("bco_trim");

	const int entries_per_plot = 50000; // Number of entries per plot
	Long64_t nEntries = BCOWindowTree->GetEntries();
	int num_plots = (nEntries + entries_per_plot - 1) / entries_per_plot; // Round up for partial ranges

	// Loop through each chunk
	for (int plot_idx = 0; plot_idx < num_plots; ++plot_idx) {
		Long64_t start_entry = plot_idx * entries_per_plot;
		Long64_t end_entry = std::min<Long64_t>((plot_idx + 1) * entries_per_plot, nEntries);

		// Data points for TGraph
		std::vector<int> x_values;
		std::vector<int> y1_values;
		std::vector<int> y2_values;

		// Loop through the entries in the current range
		for (Long64_t i = start_entry; i < end_entry; ++i) {
			BCOWindowTree->GetEntry(i);
			// Collect data points
			x_values.push_back(bco_trim);
			y1_values.push_back(0 - static_cast<int>(bco_trim - lower_bound));
			y2_values.push_back(static_cast<int>(upper_bound - bco_trim));
			//std::cout<<upper_bound - bco_trim<<std::endl; 
		}

		// Create TGraph for the data points
		TGraph *graph_y1 = new TGraph(x_values.size(), x_values.data(), y1_values.data());
		TGraph *graph_y2 = new TGraph(x_values.size(), x_values.data(), y2_values.data());

		// Set graph styles
		graph_y1->SetMarkerStyle(20);  // Closed circle
		graph_y1->SetMarkerSize(0.8);  // Adjust marker size
		graph_y1->SetMarkerColor(kRed);
		graph_y1->SetTitle(Form("BCO Window Analysis (Part %d);GL1 BCO;Time Window", plot_idx + 1));
		graph_y1->SetMaximum(500);
		graph_y1->SetMinimum(-100);


		graph_y2->SetMarkerStyle(21);  // Closed square
		graph_y2->SetMarkerSize(0.8);  // Adjust marker size
		graph_y2->SetMarkerColor(kBlue);

		// Create a canvas and draw the graphs
		TCanvas *canvas = new TCanvas(
				Form("c3_part%d", plot_idx),
				Form("BCO Window Analysis (Part %d)", plot_idx + 1),
				1200, 600
				);

		graph_y1->Draw("AP"); // Draw with axis and line
		graph_y2->Draw("P SAME"); // Overlay the second graph

		// Add legend
		TLegend *legend = new TLegend(0.7, 0.75, 0.98, 0.95);
		legend->AddEntry(graph_y1, "Lower Bound - BCO Trim", "l");
		legend->AddEntry(graph_y2, "Upper Bound - BCO Trim", "l");
		legend->Draw();

		// Save the canvas
		canvas->SaveAs(Form("%s/BCOWindowAnalysisQA_part%d.pdf",outputDir.c_str(), plot_idx + 1));

		// Cleanup
		delete graph_y1;
		delete graph_y2;
		delete canvas;
	}
	/*
	   uint64_t bco_trim_min = BCOWindowTree->GetMinimum("bco_trim");
	   uint64_t bco_trim_max = BCOWindowTree->GetMaximum("bco_trim");

	   const int entries_per_plot = 1000; // Number of entries per plot
	   Long64_t nEntries = BCOWindowTree->GetEntries();
	   int num_plots = (nEntries + entries_per_plot - 1) / entries_per_plot; // Round up for partial ranges

	// Loop through each segment and create separate histograms
	for (int plot_idx = 0; plot_idx < num_plots; ++plot_idx) {
// Define the entry range for this plot
Long64_t start_entry = plot_idx * entries_per_plot;
Long64_t end_entry = std::min<Long64_t>((plot_idx + 1) * entries_per_plot, nEntries);

// Find the local min/max BCO trim in this entry range
uint64_t local_bco_trim_min = UINT64_MAX;
uint64_t local_bco_trim_max = 0;

for (Long64_t i = start_entry; i < end_entry; ++i) {
BCOWindowTree->GetEntry(i);
if (bco_trim < local_bco_trim_min) local_bco_trim_min = bco_trim;
if (bco_trim > local_bco_trim_max) local_bco_trim_max = bco_trim;
}

// Create histograms for this range
auto h_y1 = new TH1I(
Form("h_y1_part%d", plot_idx),
Form("Lower Bound - BCO Trim (Part %d);BCO Trim;Value", plot_idx + 1),
local_bco_trim_max - local_bco_trim_min + 1,
local_bco_trim_min,
local_bco_trim_max + 1
);

auto h_y2 = new TH1I(
Form("h_y2_part%d", plot_idx),
Form("Upper Bound - BCO Trim (Part %d);BCO Trim;Value", plot_idx + 1),
local_bco_trim_max - local_bco_trim_min + 1,
local_bco_trim_min,
local_bco_trim_max + 1
);

// Fill the histograms
for (Long64_t i = start_entry; i < end_entry; ++i) {
BCOWindowTree->GetEntry(i);
h_y1->Fill(bco_trim - local_bco_trim_min, 0 - static_cast<int>(bco_trim - lower_bound));
h_y2->Fill(bco_trim - local_bco_trim_min, upper_bound - bco_trim);
}

// Create a canvas and draw the histograms
TCanvas *canvas = new TCanvas(
Form("c3_part%d", plot_idx),
Form("BCO Window Analysis (Part %d)", plot_idx + 1),
1200, 600
);
h_y1->SetLineColor(kRed);
h_y1->SetLineWidth(2);
h_y1->SetMaximum(600);
h_y1->SetMinimum(-100);
h_y1->Draw("HIST");
h_y2->SetLineColor(kBlue);
h_y2->SetLineWidth(2);
h_y2->Draw("HIST SAME");

// Add legend
TLegend *legend = new TLegend(0.7, 0.75, 0.98, 0.95);
legend->AddEntry(h_y1, "Lower Bound - BCO Trim", "l");
legend->AddEntry(h_y2, "Upper Bound - BCO Trim", "l");
legend->Draw();

// Save the canvas
canvas->SaveAs(Form("BCOWindowAnalysis_part%d.pdf", plot_idx + 1));

// Cleanup
delete h_y1;
delete h_y2;
delete canvas;
}

*/

// Clean up
file->Close();
delete file;

std::cout << "Analysis completed!" << std::endl;
}

