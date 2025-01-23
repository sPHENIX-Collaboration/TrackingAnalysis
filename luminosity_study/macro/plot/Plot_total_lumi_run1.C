#include <TCanvas.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TH1F.h>
#include <TLegend.h>
#include <TStyle.h>
#include <TMultiGraph.h>
#include <TGaxis.h>

void Plot_total_lumi_run1() {
	int start_run = 51720;
	int end_run =52218;
//SetRangeUser(start_run , end_run);
    // Example data arrays
    const int nPoints = 145;
double run_number[nPoints] = {
51735, 51736, 51738, 51740, 51741, 51742, 51753, 51754, 51762, 51766, 51768, 51771, 51772, 51777, 51778, 51824, 51825, 51826, 51827, 51828, 51829, 51830, 51831, 51837, 51838, 51839, 51840, 51841, 51842, 51843, 51854, 51858, 51859, 51860, 51865, 51874, 51878, 51881, 51886, 51900, 51901, 51902, 51905, 51906, 51907, 51908, 51909, 51914, 51915, 51921, 51922, 51924, 51925, 51927, 51928, 51929, 51933, 51934, 51940, 51941, 51942, 51943, 51944, 51945, 51946, 51947, 51948, 51949, 51950, 51952, 51953, 51954, 51955, 51958, 51960, 51977, 51978, 51982, 51983, 51985, 51986, 51987, 51988, 51990, 51997, 51998, 51999, 52005, 52015, 52018, 52019, 52022, 52025, 52026, 52027, 52030, 52032, 52034, 52035, 52036, 52037, 52038, 52043, 52044, 52045, 52046, 52047, 52048, 52049, 52050, 52052, 52060, 52062, 52065, 52067, 52068, 52069, 52077, 52078, 52097, 52099, 52101, 52102, 52103, 52104, 52105, 52107, 52108, 52109, 52110, 52111, 52112, 52144, 52145, 52146, 52194, 52198, 52199, 52200, 52201, 52202, 52203, 52204, 52205, 52206
};
double total_events[nPoints] = {
1973253, 4794205, 4059736, 3715918, 18707500, 7419504, 11410177, 13349012, 1138755, 1484907, 6916288, 755566, 2499778, 4564692, 1315200, 2297394, 13595162, 16004454, 18866647, 20142315, 22199950, 23647252, 14075227, 11365973, 1855213, 13890466, 2864844, 1889219, 7184046, 3807972, 7406640, 22207679, 12785954, 9983494, 2710378, 10035632, 5246346, 21715346, 4921646, 26377596, 2638830, 11098877, 6989468, 15477562, 9015629, 14814381, 11451356, 3276624, 14079288, 19449255, 11832758, 13261380, 18456082, 24540827, 28613215, 11870030, 1826434, 1107637, 2397312, 5227253, 18500127, 5556841, 1890528, 10192440, 12951772, 5126989, 5557989, 10309383, 3706955, 4596197, 3706812, 29959742, 7334660, 2782526, 15279015, 3650965, 3029207, 2163418, 3025654, 26424243, 6769519, 10640806, 26583809, 9694792, 2680709, 6449286, 2214828, 7217184, 21091373, 22612701, 8871161, 23476303, 1411683, 6729684, 13390114, 10078349, 4772675, 2185852, 5859290, 3018951, 26896249, 6213139, 6344071, 1666132, 16109364, 12178122, 20383456, 23242907, 25277034, 26935860, 7957518, 5213143, 3870370, 1368506, 9572193, 3266148, 8929891, 20707592, 13751323, 1768169, 2784657, 2396541, 7644214, 4791994, 10484998, 2623195, 23542173, 12997265, 7285934, 3051408, 29840408, 2813253, 1772570, 5771363, 10293430, 9951034, 2678034, 2453641, 6727132, 11136436, 21986981, 9153868, 4213730, 17094952, 17302083
};
double luminosity[nPoints] = {
0.00134796, 0.00300809, 0.00234436, 0.00194972, 0.00921858, 0.00337301, 0.00484289, 0.00547843, 0.000736907, 0.000809118, 0.00338933, 0.000654763, 0.00196897, 0.00315225, 0.000867794, 0.00164158, 0.00869719, 0.00905182, 0.00976358, 0.00973737, 0.0100824, 0.0101402, 0.00581085, 0.00763717, 0.00111582, 0.00787544, 0.00154232, 0.00100422, 0.00375035, 0.00195072, 0.00319768, 0.00910809, 0, 0.00389051, 0.00199457, 0.0062418, 0.00296634, 0.0114657, 0.00241865, 0.00987791, 0.000961258, 0.00399766, 0.00582242, 0.0107874, 0.00574953, 0.00889179, 0.00632817, 0.00274178, 0.00962622, 0.0113807, 0.00629633, 0.00658635, 0.00872251, 0.0109601, 0.0117076, 0.00471764, 0.00153413, 0.000884047, 0.00153792, 0.00325731, 0.0108358, 0.00307372, 0.00103458, 0.0054717, 0.00675474, 0.00245362, 0.00262753, 0.00472664, 0.00167839, 0.00205196, 0.00163838, 0.0129432, 0.00309452, 0.00116117, 0.00628272, 0.00188417, 0.00152421, 0.000947404, 0.00129717, 0.0108256, 0.00266407, 0.00405365, 0.00976889, 0.00348936, 0.00215326, 0.00444107, 0.00140198, 0.00420392, 0.011409, 0.0114924, 0.00432705, 0.0103018, 0.00122628, 0.00522569, 0.00906389, 0.00577801, 0.00246545, 0.00108011, 0.00286572, 0.00145746, 0.012623, 0.00283099, 0.00413683, 0.00100939, 0.00901958, 0.00615714, 0.00941364, 0.0100239, 0.0102087, 0.0103008, 0.00294344, 0.0041123, 0.00259328, 0.00133031, 0.00767269, 0.00233266, 0.0060131, 0.000996844, 0.000578239, 0.00148529, 0.00204994, 0.0016541, 0.00498155, 0.00292385, 0.00611119, 0.0014339, 0.0115695, 0.00611328, 0.00333621, 0.00136719, 0.0130117, 0.00119194, 0.00095828, 0.00303487, 0.00519425, 0.00636493, 0.00243874, 0.00206227, 0.00519043, 0.00777624, 0.0139122, 0.00537675, 0.00236749, 0.00926305, 0.00896115
};

const int nPoints_corrected =1;
double run_number_corrected[nPoints_corrected]={52104};
double total_events_corrected[nPoints_corrected]={17119398};
//    double run_number[nPoints];        // x-axis: Run numbers
//    double luminosity[nPoints];       // y-axis: Luminosity
//    double total_events[nPoints];     // y-axis: Total events
    double lumi_trigger_event[nPoints]; // y-axis: Average lumi per event
    double total_lumi[nPoints];       // y-axis: Accumulated luminosity

	int j_corrected=0;
    // Fill example data (replace with your actual data)
    for (int i = 0; i < nPoints; i++) {
	total_events[i]=total_events[i]/1000000.;
        lumi_trigger_event[i] = luminosity[i] / total_events[i]; // Compute average lumi per event
	if(lumi_trigger_event[i]>0.7e-3) cout<<"High density run "<<run_number[i]<<" lumi density>0.7e-3: "<<lumi_trigger_event[i]<<" Total Events "<<total_events[i]<<endl;
        //missing evt runs
	if(j_corrected<nPoints_corrected && run_number[i]==run_number_corrected[j_corrected]){
		cout<<"Correct for run "<<run_number[i]<<" collected events "<<total_events[i]<<"M, DB events: "<<total_events_corrected[j_corrected]/1000000.<<"M, Collected Lumi "<<luminosity[i]<<endl;
		luminosity[i]=luminosity[i]/total_events[i]*(total_events_corrected[j_corrected]/1000000.);
		cout<<"Corrected Lumi "<<luminosity[i]<<endl;
		j_corrected++;
	}

        total_lumi[i] = (i == 0) ? luminosity[i] : total_lumi[i - 1] + luminosity[i]; // Accumulate lumi

    }
	cout<<"Total Lumi for run 1: "<<total_lumi[nPoints-1]<<endl;
    // Create a canvas
    TCanvas *c1 = new TCanvas("c1", "Run Number Analysis", 900, 800);
    c1->Divide(1, 2); // Split the canvas into two pads

    // Upper Plot: Luminosity and Total Events
    c1->cd(1); // Upper panel
    gPad->SetTopMargin(0.07);
gPad->SetBottomMargin(0.15);
gPad->SetLeftMargin(0.1);
//gPad->SetRightMargin(0.1);


TMultiGraph *mg1 = new TMultiGraph();

// Create the primary graph for luminosity
TGraph *gr_lumi = new TGraph(nPoints, run_number, luminosity);
gr_lumi->SetLineColor(kBlue);
gr_lumi->SetLineWidth(2);
gr_lumi->SetTitle("Luminosity");
mg1->Add(gr_lumi);

// Scale the total_events data for better visualization
Double_t scaled_events[nPoints];
for (int i = 0; i < nPoints; i++) {
    scaled_events[i] = total_events[i] / 30*0.015; // Scale total_events to match 0-20 range
}
TGraph *gr_events = new TGraph(nPoints, run_number, scaled_events);
gr_events->SetLineColor(kRed);
gr_events->SetLineWidth(2);
gr_events->SetTitle("Total Events (scaled to 0-30)");
mg1->Add(gr_events);

// Draw the MultiGraph
mg1->GetXaxis()->SetTitle("Run Number");
mg1->GetYaxis()->SetTitle("Luminosity (pb^{-1})");
mg1->GetXaxis()->SetLabelSize(0.06);
mg1->GetYaxis()->SetLabelSize(0.06);
mg1->GetXaxis()->SetTitleSize(0.06);
mg1->GetYaxis()->SetTitleSize(0.06);
mg1->GetYaxis()->SetTitleOffset(0.9);
// Manually set the y-axis range for luminosity (left axis)
mg1->GetYaxis()->SetRangeUser(0, 0.015);
mg1->GetXaxis()->SetRangeUser(start_run , end_run);
mg1->GetYaxis()->SetMaxDigits(2);
mg1->Draw("AL");

//Double_t xmax = gPad->GetUxmax();
// Add a secondary Y-axis for scaled total events
TGaxis *rightAxis = new TGaxis(
    end_run,0,//mg1->GetXaxis()->GetXmax(), 0,        // Start of axis
    end_run,0.015,//mg1->GetXaxis()->GetXmax(), 0.015,     // End of axis
    0, 30,                                // Data range for total events (0-20)
    510, "+L");                           // Axis style
rightAxis->SetLineColor(kRed);
rightAxis->SetLabelColor(kRed);
rightAxis->SetLabelSize(0.06);
rightAxis->SetTitleSize(0.06);
rightAxis->SetTitleOffset(0.5);
rightAxis->SetTitle("Total Events (M)");
rightAxis->Draw();
/*    TMultiGraph *mg1 = new TMultiGraph();
    TGraph *gr_lumi = new TGraph(nPoints, run_number, luminosity);
    TGraph *gr_events = new TGraph(nPoints, run_number, total_events);

    gr_lumi->SetLineColor(kBlue);
    gr_lumi->SetLineWidth(2);
    gr_lumi->SetTitle("Luminosity");

    gr_events->SetLineColor(kRed);
    gr_events->SetLineWidth(2);
    gr_events->SetTitle("Total Events (scaled by 1e6)");

    mg1->Add(gr_lumi);
    mg1->Add(gr_events);
    mg1->Draw("AL");
    mg1->SetTitle("Upper Panel;Run Number;Luminosity / Total Events");
    mg1->GetXaxis()->SetTitle("Run Number");

    TGaxis *rightAxis = new TGaxis(mg1->GetXaxis()->GetXmax(),
                                   mg1->GetYaxis()->GetXmin(),
                                   mg1->GetXaxis()->GetXmax(),
                                   mg1->GetYaxis()->GetXmax(),
                                   mg1->GetYaxis()->GetXmin(),
                                   mg1->GetYaxis()->GetXmax(), 510, "+L");
    rightAxis->SetLabelColor(kRed);
    rightAxis->SetTitle("Total Events / 1e6");
    rightAxis->Draw();
*/
    // Lower Panel: Averaged Luminosity per Event
    c1->cd(2); // Lower panel
gPad->SetTopMargin(0.07);
gPad->SetBottomMargin(0.14);
gPad->SetLeftMargin(0.1);
gPad->SetRightMargin(0.1);

    TGraph *gr_lumi_per_event = new TGraph(nPoints, run_number, lumi_trigger_event);
    gr_lumi_per_event->SetLineColor(kGreen);
    gr_lumi_per_event->GetXaxis()->SetRangeUser(start_run , end_run);
    gr_lumi_per_event->SetLineWidth(2);
    gr_lumi_per_event->SetTitle(";Run Number;Averaged Lumi (pb^{-1}/M)");
gr_lumi_per_event->GetXaxis()->SetLabelSize(0.06);
gr_lumi_per_event->GetYaxis()->SetLabelSize(0.06);
gr_lumi_per_event->GetXaxis()->SetTitleSize(0.06);
gr_lumi_per_event->GetYaxis()->SetTitleSize(0.06);
gr_lumi_per_event->GetYaxis()->SetTitleOffset(0.7);
gr_lumi_per_event->GetYaxis()->SetMaxDigits(2);
    gr_lumi_per_event->Draw("AL");


    // Second Plot: Accumulated Luminosity
    TCanvas *c2 = new TCanvas("c2", "Accumulated Luminosity", 900, 400);
gPad->SetTopMargin(0.05);
gPad->SetBottomMargin(0.15);
gPad->SetLeftMargin(0.1);
gPad->SetRightMargin(0.1);

    TGraph *gr_total_lumi = new TGraph(nPoints, run_number, total_lumi);
    gr_total_lumi->SetLineColor(kMagenta);
    gr_total_lumi->GetXaxis()->SetRangeUser(start_run , end_run);
    gr_total_lumi->SetLineWidth(2);
    gr_total_lumi->SetTitle(";Run Number;Total Luminosity (pb^{-1})");
gr_total_lumi->GetXaxis()->SetLabelSize(0.06);
gr_total_lumi->GetYaxis()->SetLabelSize(0.06);
gr_total_lumi->GetXaxis()->SetTitleSize(0.06);
gr_total_lumi->GetYaxis()->SetTitleSize(0.06);
gr_total_lumi->GetYaxis()->SetTitleOffset(0.7);
    gr_total_lumi->Draw("AL");


    // Save the plots
    c1->SaveAs("RunNumberAnalysis.pdf");
    c2->SaveAs("AccumulatedLuminosity.pdf");
}

