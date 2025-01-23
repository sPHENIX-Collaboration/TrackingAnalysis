AUTOMAKE_OPTIONS = foreign

lib_LTLIBRARIES = \
  libSiliconQA.la

AM_CPPFLAGS = \
  -I$(includedir) \
  -I$(OFFLINE_MAIN)/include \
  -isystem`root-config --incdir`

AM_LDFLAGS = \
  -L$(libdir) \
  -L$(OFFLINE_MAIN)/lib \
  -L$(OFFLINE_MAIN)/lib64 \
  `root-config --libs`

include_HEADERS = \
  SiliconQA.h

libSiliconQA_la_SOURCES = \
  SiliconQA.cc

libSiliconQA_la_LIBADD = \
  -L$(OPT_SPHENIX)/lib \
  -lffamodules \
  -lphool \
  -lcdbobjects

libSiliconQA_la_LDFLAGS = \
  -L$(libdir) \
  -L$(OFFLINE_MAIN)/lib \
  `root-config --libs`


BUILT_SOURCES = testexternals.cc

noinst_PROGRAMS = \
  testexternals

testexternals_SOURCES = testexternals.cc
testexternals_LDADD   = libSiliconQA.la

testexternals.cc:
	echo "//*** this is a generated file. Do not commit, do not edit" > $@
	echo "int main()" >> $@
	echo "{" >> $@
	echo "  return 0;" >> $@
	echo "}" >> $@

clean-local:
	rm -f $(BUILT_SOURCES)
