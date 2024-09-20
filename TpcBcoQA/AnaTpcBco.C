#include "AnaTpcBco.h"
#include <sstream>
#include <string>
#include <iostream>
R__LOAD_LIBRARY(libAnaTpcBco.so)

void AnaTpcBco(const char * infile, bool isTPC)
{
  std::string infilename;
  if(isTPC) infilename = std::string("/sphenix/lustre01/sphnxpro/physics/tpc/physics/") + infile;
  else if(!isTPC) infilename = std::string("/sphenix/lustre01/sphnxpro/physics/GL1/physics/") + infile;
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
