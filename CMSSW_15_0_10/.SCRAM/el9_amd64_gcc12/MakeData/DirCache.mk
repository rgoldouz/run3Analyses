ifeq ($(strip $(EFTGenReader/EFTHelperUtilities)),)
ALL_COMMONRULES += src_EFTGenReader_EFTHelperUtilities_src
src_EFTGenReader_EFTHelperUtilities_src_parent := EFTGenReader/EFTHelperUtilities
src_EFTGenReader_EFTHelperUtilities_src_INIT_FUNC := $$(eval $$(call CommonProductRules,src_EFTGenReader_EFTHelperUtilities_src,src/EFTGenReader/EFTHelperUtilities/src,LIBRARY))
EFTGenReaderEFTHelperUtilities := self/EFTGenReader/EFTHelperUtilities
EFTGenReader/EFTHelperUtilities := EFTGenReaderEFTHelperUtilities
EFTGenReaderEFTHelperUtilities_files := $(patsubst src/EFTGenReader/EFTHelperUtilities/src/%,%,$(wildcard $(foreach dir,src/EFTGenReader/EFTHelperUtilities/src ,$(foreach ext,$(SRC_FILES_SUFFIXES),$(dir)/*.$(ext)))))
EFTGenReaderEFTHelperUtilities_BuildFile    := $(WORKINGDIR)/cache/bf/src/EFTGenReader/EFTHelperUtilities/BuildFile
EFTGenReaderEFTHelperUtilities_LOC_USE := self   CommonTools/Utils DataFormats/FWLite DataFormats/Math root
EFTGenReaderEFTHelperUtilities_LCGDICTS  := x 
EFTGenReaderEFTHelperUtilities_PRE_INIT_FUNC += $$(eval $$(call LCGDict,EFTGenReaderEFTHelperUtilities,src/EFTGenReader/EFTHelperUtilities/src/classes.h,src/EFTGenReader/EFTHelperUtilities/src/classes_def.xml,$(SCRAMSTORENAME_LIB),$(GENREFLEX_ARGS) $(root_EX_FLAGS_GENREFLEX_FAILES_ON_WARNS)))
EFTGenReaderEFTHelperUtilities_EX_LIB   := EFTGenReaderEFTHelperUtilities
EFTGenReaderEFTHelperUtilities_EX_USE   := $(foreach d,$(EFTGenReaderEFTHelperUtilities_LOC_USE),$(if $($(d)_EX_FLAGS_NO_RECURSIVE_EXPORT),,$d))
EFTGenReaderEFTHelperUtilities_PACKAGE := self/src/EFTGenReader/EFTHelperUtilities/src
ALL_PRODS += EFTGenReaderEFTHelperUtilities
EFTGenReaderEFTHelperUtilities_CLASS := LIBRARY
EFTGenReader/EFTHelperUtilities_forbigobj+=EFTGenReaderEFTHelperUtilities
EFTGenReaderEFTHelperUtilities_INIT_FUNC        += $$(eval $$(call Library,EFTGenReaderEFTHelperUtilities,src/EFTGenReader/EFTHelperUtilities/src,src_EFTGenReader_EFTHelperUtilities_src,$(SCRAMSTORENAME_BIN),,$(SCRAMSTORENAME_LIB),$(SCRAMSTORENAME_LOGS),))
endif
