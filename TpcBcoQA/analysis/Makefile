CXX = g++

ROOTFLAGS = $(shell root-config --cflags --libs)
CXXFLAGS = -I. $(ROOTFLAGS) -I$(ONLINE_MAIN)/include -I$(OFFLINE_MAIN)/include -DOUTPUT_DIR="\"$(OUTPUT_DIR)\""

TARGET = anaroot
SRCS = rootFileMaker.cc
OBJS = $(SRCS:.cc=.o)

ROOTOUT_DIR = rootfiles

all: $(TARGET) clean_objs create_root_dir

create_root_dir:
	@echo "Creating directory: $(OUTPUT_DIR)/$(ROOTOUT_DIR)"
	@mkdir -p $(OUTPUT_DIR)/$(ROOTOUT_DIR)


$(TARGET): $(OBJS)
		$(CXX) -o $(TARGET) $(OBJS) $(CXXFLAGS)

%.o: %.cpp
		$(CXX) -c $< -o $@ $(CXXFLAGS)

clean_objs:
	rm -f $(OBJS)

clean: 
	rm -f $(OBJS) $(TARGET) 

