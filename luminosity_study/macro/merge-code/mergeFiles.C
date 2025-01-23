#include <TFileMerger.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <TFile.h>

void mergeFiles(const std::string fileList="file.list", const std::string outputFile="merged_output.root") {
    TFileMerger merger(false);  // `false` to avoid verbose output
    gSystem->Exec("ls -tr output_*root > file.list");
   
    std::ifstream fileStream(fileList);
    if (!fileStream.is_open()) {
        std::cerr << "Error: Cannot open file list " << fileList << "!" << std::endl;
        return;
    }

    std::string line;
    std::string latestFileline;
    while (std::getline(fileStream, line)) {
       if (line.empty()) continue; // Skip empty lines
        if (!merger.AddFile(line.c_str())) {
            std::cerr << "Error: Unable to add file " << line << std::endl;
        }
	latestFileline = line;
    }
    fileStream.close();


    // Merge all files into the output file
    if (!merger.OutputFile(outputFile.c_str())) {
        std::cerr << "Error: Unable to create output file " << outputFile << std::endl;
        return;
    }

    if (!merger.Merge()) {
        std::cerr << "Error: File merging failed!" << std::endl;
    } else {
        std::cout << "Files merged successfully into " << outputFile << std::endl;
    }

// Reopen the output file to update h_rawgl1scalerBunchID
    TFile *latestFile = TFile::Open(latestFileline.c_str(), "READ");
    TH1D *latestScalerHist1 = nullptr;
    TH1D *latestScalerHist2 = nullptr;
    TH1D *latestScalerHist3 = nullptr;
    TH1D *latestScalerHist4 = nullptr;
    TH1D *latestScalerHist5 = nullptr;

    if (latestFile) {
        latestScalerHist1 = dynamic_cast<TH1D *>(latestFile->Get("h_rawgl1scalerBunchID"));
        latestScalerHist2 = dynamic_cast<TH1D *>(latestFile->Get("h_MBDSNraw_BunchID"));
        latestScalerHist3 = dynamic_cast<TH1D *>(latestFile->Get("h_MBDSNlive_BunchID"));
        latestScalerHist4 = dynamic_cast<TH1D *>(latestFile->Get("h_MBDSNscaled_BunchID"));
        latestScalerHist5 = dynamic_cast<TH1D *>(latestFile->Get("h_gl1p_ZDCCoin_BunchID"));
    }

    TFile *outFile = TFile::Open(outputFile.c_str(), "UPDATE");
    if (!outFile || outFile->IsZombie()) {
        std::cerr << "Error: Unable to open output file for updating " << outputFile << std::endl;
        return;
    }

    if (latestScalerHist1) {
        outFile->Delete("h_rawgl1scalerBunchID;*"); // Remove old versions
	outFile->Delete("h_MBDSNraw_BunchID;*"); 
	outFile->Delete("h_MBDSNlive_BunchID;*"); 
	outFile->Delete("h_MBDSNscaled_BunchID;*"); 
        outFile->Delete("h_gl1p_ZDCCoin_BunchID;*");
        latestScalerHist1->SetDirectory(outFile);   // Associate the histogram with the output file
        latestScalerHist1->Write();                 // Write the latest version
        latestScalerHist2->SetDirectory(outFile);   // Associate the histogram with the output file
        latestScalerHist2->Write();                 // Write the latest version
        latestScalerHist3->SetDirectory(outFile);   // Associate the histogram with the output file
        latestScalerHist3->Write();                 // Write the latest version
        latestScalerHist4->SetDirectory(outFile);   // Associate the histogram with the output file
        latestScalerHist4->Write();                 // Write the latest version
	latestScalerHist5->SetDirectory(outFile);
	latestScalerHist5->Write();
        std::cout << "Updated h_rawgl1scalerBunchID with the latest values." << std::endl;
    }

    
    outFile->Close();
    latestFile->Close();
    delete latestFile; 
    delete outFile;

}
