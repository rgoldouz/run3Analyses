ROOTCFLAGS     = $(shell root-config --cflags)
ROOTLIBS       = $(shell root-config --libs)
ROOTGLIBS      = $(shell root-config --glibs) 
CORLIBS        = lib/libcorrectionlib.so
CORFLAGS       = $(shell correction config --cflags)

CVMA            = /cvmfs/cms.cern.ch/el9_amd64_gcc12/cms/cmssw/CMSSW_15_0_10/src/
BOOST          = /cvmfs/cms.cern.ch/el9_amd64_gcc12/external/boost/1.80.0-fa55cb3940cb89fca310de6848667027/include/
EFT            = ../CMSSW_15_0_10/src/EFTGenReader/EFTHelperUtilities/interface/
FMT            = /cvmfs/cms.cern.ch/el9_amd64_gcc12/external/fmt/10.2.1-e35fd1db5eb3abc8ac0452e8ee427196/include/
INCLUDES       = -I./include -I./RestFrames/include -I$(EFT) -I$(BOOST) -I$(CVMA) -I$(FMT)
RESTLIB        = -L./RestFrames/lib


CXX            = g++ -m64 
CXXFLAGS       = -fPIC -fno-var-tracking -Wno-deprecated -D_GNU_SOURCE -O2  $(INCLUDES) 
CXXFLAGS      += $(ROOTCFLAGS)

LD             = g++ -m64 
LDFLAGS        =


SOFLAGS        = -O -shared  -fPIC #-flat_namespace 
LIBS           = $(ROOTLIBS) 

GLIBS         = $(ROOTGLIBS) -lMinuit -lMinuit2 -lTreePlayer -lGenVector -lTMVA

#SRCS = src/GEScaleSyst.cc src/Utils.cc src/BTagCalibrationStandalone.cc src/RoccoR.cc src/lepton_candidate.cc src/Z_candidate.cc src/jet_candidate.cc src/PU_reWeighting.cc src/sumOfWeights.cc src/sumOfWeightsSignal.cc src/LumiMask.cc src/genLevelAnalysis.cc src/JigsawRecTZFCNC.cc src/JigsawRecTHFCNC.cc src/initiateMyAnalysis.cc src/trigger.cc src/objectSelection.cc src/MyAnalysis.cc
SRCS = src/GEScaleSyst.cc src/Utils.cc src/RoccoR.cc src/lepton_candidate.cc src/Z_candidate.cc src/jet_candidate.cc src/PU_reWeighting.cc src/sumOfWeights.cc src/sumOfWeightsSignal.cc src/LumiMask.cc src/genLevelAnalysis.cc src/JigsawRecTZFCNC.cc src/JigsawRecTHFCNC.cc src/initiateMyAnalysis.cc src/trigger.cc src/objectSelection.cc src/eventSelection.cc src/MyAnalysis.cc
OBJS =  $(patsubst %.C,%.o,$(SRCS:.cc=.o))

LIB=lib/libmain.so


.SUFFIXES: .cc,.C,.hh,.h

# Rules ====================================
all: $(LIB)  bin/RunAll

lib : $(LIB)
$(LIB): $(OBJS)
	@echo "Creating library $(LIB)"
	mkdir -p lib
	$(PYPATH)
#	$(LD) $(LDFLAGS) $(GLIBS) $(SOFLAGS) $(OBJS) -L./lib -L./RestFrames/lib -lRestFrames -lJetMETCorrectionsModules  -lCondFormatsSerialization -lcorrectionlib -lEFTGenReaderEFTHelperUtilities  -lboost_serialization   -o $(LIB)
	$(LD) $(LDFLAGS) $(GLIBS) $(SOFLAGS) $(OBJS) -o $(LIB)
	@echo "$(LIB) successfully compiled!"
bin/RunAll : src/main.C  $(LIB)
	mkdir -p bin
	$(CXX) $(CXXFLAGS) -ldl -L./lib $(RESTLIB) -lRestFrames -lJetMETCorrectionsModules -lCondFormatsSerialization -lcorrectionlib -lEFTGenReaderEFTHelperUtilities -lboost_serialization -lmain src/main.C $(GLIBS) -o $@

clean:
	$(RM) $(OBJS)	
	$(RM) $(LIB)
	$(RM) bin/RunAll

purge:
	$(RM) $(OBJS)

deps: $(SRCS)
	makedepend $(INCLUDES) $^

