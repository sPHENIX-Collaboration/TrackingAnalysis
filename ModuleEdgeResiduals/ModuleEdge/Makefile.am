AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = \
  -I$(includedir) \
  -isystem$(OFFLINE_MAIN)/include \
  -isystem$(ROOTSYS)/include

AM_LDFLAGS = \
  -L$(libdir) \
  -L$(OFFLINE_MAIN)/lib \
  -L$(OFFLINE_MAIN)/lib64

pkginclude_HEADERS = \
  PHModuleEdgeCorresctions.h 

lib_LTLIBRARIES = \
  libModuleEdge.la

libModuleEdge_la_SOURCES = \
  PHModuleEdgeCorresctions.cc 

libModuleEdge_la_LIBADD = \
  -lphool \
  -lfun4all \
  -lg4eval \
  -lg4eval_io \
  -lphg4hit \
  -lg4detectors_io \
  -ltrack_io \
  -ltrackbase_historic_io \
  -ltpc_io \
  -lmvtx_io \
  -lintt_io \
  -lmicromegas_io \
  -lSubsysReco \
  -ltrackeralign

BUILT_SOURCES = testexternals.cc

noinst_PROGRAMS = \
  testexternals

testexternals_SOURCES = testexternals.cc
testexternals_LDADD   = libModuleEdge.la

testexternals.cc:
	echo "//*** this is a generated file. Do not commit, do not edit" > $@
	echo "int main()" >> $@
	echo "{" >> $@
	echo "  return 0;" >> $@
	echo "}" >> $@

clean-local:
	rm -f $(BUILT_SOURCES)
