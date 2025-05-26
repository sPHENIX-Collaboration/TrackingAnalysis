#ifndef UTILS_H
#define UTILS_H

#include <Riostream.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>

#ifdef OUTPUT_DIR
const std::string outputdirectory = OUTPUT_DIR;
#else
const std::string outputdirectory = "../output/"; 
#endif

const int TotalNumPackets=1;
const int nFiles = 24;
const int nTotalFiles = 48;
const bool WRITE_TPC_SORTED = true;
const std::string runtypetext="Dry Run";

struct TpcDataEntry {
  int event;
  int packet_id;
  int tagger;
  int tagger_id;
  long long clock;
  int nfees;
};

bool compareByClock(const TpcDataEntry &a, const TpcDataEntry &b) {
  return a.clock < b.clock;
}


#endif
