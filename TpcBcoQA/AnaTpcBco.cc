#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <Event/Event.h> 
#include <Event/Eventiterator.h>
#include <Event/fileEventiterator.h>
#include <pmonitor/pmonitor.h>
#include <map>
#include "AnaTpcBco.h"
#include <algorithm>
#include <fstream>

#include <TH1.h>
#include <TH2.h>
#include "TSystem.h"
#include <TGraph.h>

#include <cstdlib>
#include <iomanip>

int init_done = 0;
int offset=0;
Packet **plist{nullptr};
std::ostringstream outfilename;
std::string servernamestr;
std::ostringstream buffer;
std::ofstream outfile;
const std::size_t BUFFER_SIZE_THRESHOLD = 8192;  
long int nevent =0;
std::vector<DataEntry> dataEntries;

const int NTPCPACKETS = 3;
bool useGL1 = false;
int runnumber;
using namespace std;
 
TH1F *h_accept;

uint64_t gl1currentclock;
uint64_t gtm_bco;

bool compareByClock(const DataEntry &a, const DataEntry &b) {
      return a.gtm_bco < b.gtm_bco;
}

void flush_buffer() {
  if (buffer.tellp() > 0) {
    outfile << buffer.str();
    buffer.str("");        
    buffer.clear();       
  }
}

int pinit()
{
    if (init_done) return 1;
    init_done = 1;
    return 0;
}

int process_event (Event * e)
{
  int evtnr = e->getEvtSequence();
  
  if (e->getEvtType() >= 8) 
  {
    return 0;
  }

  DataEntry entry;
  entry.evtnr = evtnr;

  bool isgl1valid= false;
  std::ostringstream line_data;

  plist = new Packet *[NTPCPACKETS];
  int npackets = e->getPacketList(plist, NTPCPACKETS);
  for (int ipacket = 0; ipacket < npackets; ipacket++) {
    auto &packet = plist[ipacket];
    if(!packet) continue;
    int pnum = packet->getIdentifier();
    if(pnum<=4231 && pnum>=4000){
      bool foundL1 = false;
      uint64_t m_nTaggerInFrame = packet->lValue(0, "N_TAGGER");
      int NR_VALIDFEE = packet->lValue(0,"NR_VALIDFEE");
      entry.pnum = pnum;
      entry.m_nTaggerInFrame = m_nTaggerInFrame;
      entry.NR_VALIDFEE = NR_VALIDFEE;
      for (uint64_t t = 0; t < m_nTaggerInFrame; t++)
      {
        const auto is_lvl1 = static_cast<uint8_t>(packet->lValue(t, "IS_LEVEL1_TRIGGER"));
        const auto is_endat = static_cast<uint8_t>(packet->lValue(t, "IS_ENDAT")); 
        if (is_lvl1 || is_endat)
        {
          gtm_bco = packet->lValue(t, "BCO");
          foundL1 = true;
          entry.t = t;
          entry.gtm_bco = gtm_bco;
          dataEntries.push_back(entry);
        }
      }
    }
    
    if(pnum==14001){
      uint64_t gl1bco = packet->lValue(0,"BCO") & 0xFFFFFFFFFFU;
      isgl1valid = true;
      useGL1 = true;
      line_data << evtnr << " " << gl1bco << std::endl; 
    }
    delete packet;
  }
  delete plist;
  
  if (isgl1valid) {
    outfile << line_data.str();
  }
  
  nevent++;
  if(nevent % 10000 ==0) { std::cout << "event processed : " << evtnr << std::endl;}

  return 0;
}

int pclose(){
  if(!useGL1){
    std::sort(dataEntries.begin(), dataEntries.end(), compareByClock);
    for (const auto &entry : dataEntries) {
      outfile << entry.evtnr << " " << entry.pnum << " "
        << entry.m_nTaggerInFrame << " " << entry.t << " "
        << entry.gtm_bco << " " << entry.NR_VALIDFEE << "\n";
    }
  }
  outfile.close();
  return 0;
}

void getfilename(const char * filename)
{
  if (gSystem->AccessPathName(filename)) {
    cout << "\"" << filename << "\" does not exist" << endl;
    return;
  }
  std::string filestr(filename);
  std::size_t ebdc_server = filestr.find("ebdc");
  std::size_t last_dash = filestr.rfind('-');
  std::string segment_part = filestr.substr(last_dash + 1, 4);
  std::string run_number_part = filestr.substr(last_dash - 5, 5);

  std::size_t gl1_server = filestr.find("gl1daq");
  if (ebdc_server != std::string::npos) {
    std::size_t end = filestr.find("-000" + run_number_part);
    std::string extracted = filestr.substr(ebdc_server, end - ebdc_server);
    servernamestr = extracted;
  }
  else if( gl1_server != std::string::npos){
    std::string extracted = filestr.substr(gl1_server,6);
    servernamestr = extracted;
  }
  else {
    std::cout << "String not found!" << std::endl;
    servernamestr = "nostringfound";
  }

  std::string formatted_filename = std::string(OUTPUT_DIR) + "/output_" + servernamestr + "_RunNumber_" + run_number_part + "_segment" + segment_part + ".txt";
  outfilename << formatted_filename;
  std::cout << "outfilename : " << outfilename.str() << std::endl;
  outfile.open(outfilename.str(), std::ios::trunc);
}

