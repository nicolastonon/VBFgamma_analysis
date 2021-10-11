# $< = name of first dependancy
# $@ = name of target ("main")
# -c asks the compiler not to link. Allows to get a compiled "object file" which does not contain
# -o specify target
# $^ all prerequisites in the rule

#Define variables
CC=g++ -fPIC #Compiler used #fPIC useful to get debug messages
DEBUG = -g
CFLAGS = -c $(DEBUG) #Compilation options
ROOTFLAGS = `root-config --glibs --cflags`

LFLAGS = $(DEBUG) -lTMVA -lPyMVA
# LFLAGS+= -L/home/ntonon/Documents/Programmes/tensorflow/bazel-bin/tensorflow -Wl,--allow-multiple-definition -Wl,--whole-archive -ltensorflow_framework -Wl,--whole-archive -Wl,--no-as-needed -ltensorflow_cc #-lprotobuf

# INCFLAGS = -I./include
# INCFLAGS += -I./Utils
# INCFLAGS += -I/home/ntonon/Documents/Programmes/tensorflow
# INCFLAGS += -I/home/ntonon/Documents/Programmes/tensorflow/bazel-genfiles
# INCFLAGS += -I/home/ntonon/Documents/Programmes/tensorflow/bazel-tensorflow/external/eigen_archive
# INCFLAGS += -I/home/ntonon/Documents/Programmes/tensorflow/bazel-tensorflow/external/com_google_absl
# INCFLAGS += -I/usr/local/include/google/protobuf
# INCFLAGS += -I/usr/local/include/google/tensorflow

SRCS = $(wildcard *.cxx) #Source files are all files with .cxx extension
HDRS = $(wildcard *.h) #Header files are all files with .h extension
OBJS = $(SRCS:.cxx=.o) #Object files are all files with .o extension, which have same names as source files

# DICT = myDict.cxx
# LIB = myLib.so

MY_ANALYSIS = analysis_main.exe #Name of executable file
YIELD = Yield_Table.exe
# ROCS = ROCS/Compare_ROC_curves.exe
# SPLIT = Split_FullSamples.exe

# .PHONY : $(wildcard *.o)  #Force to always recompile object

#Instructions
all: $(MY_ANALYSIS) $(YIELD) #$(ROCS) $(SPLIT)

#Obtain executables from object files
$(MY_ANALYSIS): Utils/Helper.o analysis_main.o VBFgamma_analysis.o #ROCS/ROC_Plotter.o
	@echo "###################################"
	@echo "-- Creating executable ./$(MY_ANALYSIS) --"
	@$(CC) $^ -o $@ $(ROOTFLAGS) $(LFLAGS) $(INCFLAGS)
	@echo "-- Done --"
	@echo "###################################""#"
	@echo ""

$(ROCS): ROCS/ROC_Plotter.o ROCS/Compare_ROC_curves.o Utils/Helper.o
	@echo "###################################"
	@echo "-- Creating executable $(ROCS) --"
	@$(CC) $^ -o $@ $(ROOTFLAGS) $(LFLAGS)
	@echo "-- Done --"
	@echo "###################################"
	@echo ""

$(YIELD): Utils/Yield_Table.o Utils/Helper.o $(LIB)
	@echo "###################################""#"
	@echo "-- Creating executable ./$(YIELD) --"
	@$(CC) $^ -o $@ $(ROOTFLAGS) $(LFLAGS) $(INCFLAGS)
	@echo "-- Done --"
	@echo "###################################""#"
	@echo ""

$(SPLIT): input_ntuples/Split_FullSamples.cxx Utils/Helper.o $(LIB)
	@echo "###################################""#"
	@echo "-- Creating executable ./$(SPLIT) --"
	@$(CC) $^ -o $@ $(ROOTFLAGS) $(LFLAGS) $(INCFLAGS)
	@echo "-- Done --"
	@echo "###################################""#"
	@echo ""

#Obtain objects from source and header files
#NB: only works for .cxx files ! e.g. for .cpp, would need a specific rule
%.o: %.cxx $(HDRS)
	@echo "-- Compiling : "$@" --"
	@$(CC) $(ROOTFLAGS) $(CFLAGS) $(LFLAGS) $(INCFLAGS) $< -o $@
	@echo ""
# @Example :  g++ `root-config --glibs --cflags` -fPIC -c -g Utils/Helper.cxx -o Utils/Helper.o

#Erase all objects and executable
clean:
	@-rm -f *.o *.exe Utils/*.o ROCS/*.o ROCS/*.exe
