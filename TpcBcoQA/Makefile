PACKAGE = AnaTpcBco

ROOTFLAGS = $(shell root-config --cflags)
ROOTLIBS = $(shell root-config --glibs)

OUTPUT_DIR = $(shell pwd)/output

CXXFLAGS = -I.  $(ROOTFLAGS) -I$(ONLINE_MAIN)/include -I$(OFFLINE_MAIN)/include -DOUTPUT_DIR="\"$(OUTPUT_DIR)\""
RCFLAGS = -I.  -I$(ONLINE_MAIN)/include -I$(OFFLINE_MAIN)/include

LDFLAGS = -Wl,--no-as-needed  -L$(ONLINE_MAIN)/lib -L$(OFFLINE_MAIN)/lib -lpmonitor -lEvent -lNoRootEvent -lmessage  $(ROOTLIBS) -fPIC


HDRFILES = $(PACKAGE).h
LINKFILE = $(PACKAGE)LinkDef.h

ADDITIONAL_SOURCES = 
ADDITIONAL_LIBS = 

SO = lib$(PACKAGE).so

all: $(OUTPUT_DIR) utilsout.config $(SO) analysis_make

$(SO) : $(PACKAGE).cc $(PACKAGE)_dict.C $(ADDITIONAL_SOURCES) $(LINKFILE) $(OUTPUT_DIR)
	$(CXX) $(CXXFLAGS) -o $@ -shared  $<  $(ADDITIONAL_SOURCES) $(PACKAGE)_dict.C $(LDFLAGS)  $(ADDITIONAL_LIBS)

$(OUTPUT_DIR):
	@echo "Creating output directory: $(OUTPUT_DIR)"
	@mkdir -p $(OUTPUT_DIR)
	@echo "Done..!"

utilsout.config: $(OUTPUT_DIR)
	@echo 'OUTPUT_DIR=$(OUTPUT_DIR)' > utilsout.config
	@echo "Done utilsout.config with dynamic $(OUTPUT_DIR)"

$(PACKAGE)_dict.C : $(HDRFILES) $(LINKFILE)
	rootcint -f $@  -c $(RCFLAGS) $^

analysis_make:
	@echo "Running make in the analysis directory..."
	cd analysis && $(MAKE) OUTPUT_DIR=$(OUTPUT_DIR)

.PHONY: clean analysis_make

clean: 
	rm -f $(SO) $(PACKAGE)_dict.C $(PACKAGE)_dict.h
	cd analysis && $(MAKE) clean
	rm -f utilsout.config

