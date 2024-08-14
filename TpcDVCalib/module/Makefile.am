##############################################
# please add new classes in alphabetical order

AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = \
  -I$(includedir) \
  -I$(OFFLINE_MAIN)/include \
  -isystem$(ROOTSYS)/include

pkginclude_HEADERS = \
  TrackToCalo.h

# List of shared libraries to produce
lib_LTLIBRARIES = \
  libtpc_dv_calib.la

libtpc_dv_calib_la_SOURCES = \
  TrackToCalo.cc

libtpc_dv_calib_la_LDFLAGS = \
  -L$(libdir) \
  -L$(OFFLINE_MAIN)/lib64 \
  -L$(OFFLINE_MAIN)/lib

libtpc_dv_calib_la_LIBADD = \
  -lActsCore \
  -lActsPluginTGeo \
  -lActsExamplesCommon \
  -lActsExamplesDetectorTGeo \
  -lActsExamplesFramework \
  -lSubsysReco \
  -ltrackbase_historic_io \
  -ltrack_io \
  -ltrack_reco \
  -lcalo_io \
  -lglobalvertex_io \
  -lg4detectors_io \
  -lphparameter

################################################
# linking tests

noinst_PROGRAMS = \
  testexternals_tpc_dv_calib

BUILT_SOURCES  = testexternals.cc

testexternals_tpc_dv_calib_SOURCES = testexternals.cc
testexternals_tpc_dv_calib_LDADD = libtpc_dv_calib.la

testexternals.cc:
	echo "//*** this is a generated file. Do not commit, do not edit" > $@
	echo "int main()" >> $@
	echo "{" >> $@
	echo "  return 0;" >> $@
	echo "}" >> $@

##############################################
# please add new classes in alphabetical order

clean-local:
	rm -f $(BUILT_SOURCES)
