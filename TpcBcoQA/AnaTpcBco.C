#include "AnaTpcBco.h"
#include <sstream>
#include <string>
#include <iostream>
R__LOAD_LIBRARY(libAnaTpcBco.so)

void AnaTpcBco(const char * infile, std::string runtype, bool isTPC)
{
  std::string infilename;
  if(isTPC) infilename = std::string(Form("/sphenix/lustre01/sphnxpro/physics/tpc/%s/",runtype.c_str())) + infile;
  else if(!isTPC) infilename = std::string(Form("/sphenix/lustre01/sphnxpro/physics/GL1/%s/",runtype.c_str())) + infile;
  const char *filename = infilename.c_str();
    if ( filename != NULL)
    {
      pfileopen(filename);
      getfilename(filename);
      pinit();
      prun();
      pclose();
    }
}
