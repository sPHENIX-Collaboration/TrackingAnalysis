#ifndef __ANATPCBCO_H__
#define __ANATPCBCO_H__

#include <pmonitor/pmonitor.h>
#include <Event/Event.h>
#include <Event/EventTypes.h>

struct DataEntry {
  int evtnr;
  int pnum;
  int m_nTaggerInFrame;
  int t;
  long long gtm_bco;
  int NR_VALIDFEE;
};

int process_event (Event *e); //++CINT 
int pclose();
bool compareByClock(const DataEntry &a, const DataEntry &b);

void getfilename(const char * filename);
void flush_buffer();
void checkClocks();
#endif /* __ANATPCBCO_H__ */
