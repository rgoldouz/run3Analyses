# Source this script to set the environmental variables for this RestFrames installation
# 
# Users can automatically call this script at their shell login by adding to their
# .cshrc file:
#    . $(restframes-config --prefix)/libexec/setup_RestFrames.csh
# assuming restframes-config is in their PATH, or
#    . PATH_OF_RESTFRAMES/libexec/setup_RestFrames.sh
#
# This script if for csh-like shells, see setup_RestFrames.sh for bash-like shells.

if ($?RESTFRAMESSYS) then
   set restframessys="$RESTFRAMESSYS"
endif

setenv RESTFRAMESSYS /groups/mhildret/rgoldouz/run3Analyses/ZprimeToTt/RestFrames

if ($?restframessys) then
   setenv PATH `echo $PATH | sed -e "s;:$restframessys/bin:;:;g" \
                                 -e "s;:$restframessys/bin;;g"   \
                                 -e "s;$restframessys/bin:;;g"   \
                                 -e "s;$restframessys/bin;;g"`
   if ($?LD_LIBRARY_PATH) then
      setenv LD_LIBRARY_PATH `echo $LD_LIBRARY_PATH | \
                             sed -e "s;:$restframessys/lib:;:;g" \
                                 -e "s;:$restframessys/lib;;g"   \
                                 -e "s;$restframessys/lib:;;g"   \
                                 -e "s;$restframessys/lib;;g"`
   endif
   if ($?DYLD_LIBRARY_PATH) then
      setenv DYLD_LIBRARY_PATH `echo $DYLD_LIBRARY_PATH | \
                             sed -e "s;:$restframessys/lib:;:;g" \
                                 -e "s;:$restframessys/lib;;g"   \
                                 -e "s;$restframessys/lib:;;g"   \
                                 -e "s;$restframessys/lib;;g"`
   endif
   if ($?SHLIB_PATH) then
      setenv SHLIB_PATH `echo $SHLIB_PATH | \
                             sed -e "s;:$restframessys/lib:;:;g" \
                                 -e "s;:$restframessys/lib;;g"   \
                                 -e "s;$restframessys/lib:;;g"   \
                                 -e "s;$restframessys/lib;;g"`
   endif
   if ($?LIBPATH) then
      setenv LIBPATH `echo $LIBPATH | \
                             sed -e "s;:$restframessys/lib:;:;g" \
                                 -e "s;:$restframessys/lib;;g"   \
                                 -e "s;$restframessys/lib:;;g"   \
                                 -e "s;$restframessys/lib;;g"`
   endif
   if ($?PYTHONPATH) then
      setenv PYTHONPATH `echo $PYTHONPATH | \
                             sed -e "s;:$restframessys/lib:;:;g" \
                                 -e "s;:$restframessys/lib;;g"   \
                                 -e "s;$restframessys/lib:;;g"   \
                                 -e "s;$restframessys/lib;;g"`
   endif
   if ($?CPLUS_INCLUDE_PATH) then
      setenv CPLUS_INCLUDE_PATH `echo CPLUS_INCLUDE_PATH | \
                             sed -e "s;:$restframessys/include:;:;g" \
                                 -e "s;:$restframessys/include;;g"   \
                                 -e "s;$restframessys/include:;;g"   \
                                 -e "s;$restframessys/include;;g"`
   endif
   if ($?CPATH) then
      setenv CPATH `echo CPATH | \
                             sed -e "s;:$restframessys/include:;:;g" \
                                 -e "s;:$restframessys/include;;g"   \
                                 -e "s;$restframessys/include:;;g"   \
                                 -e "s;$restframessys/include;;g"`
   endif
endif

set path = (/groups/mhildret/rgoldouz/run3Analyses/ZprimeToTt/RestFrames/bin $path)

if ($?LD_LIBRARY_PATH) then
   setenv LD_LIBRARY_PATH /groups/mhildret/rgoldouz/run3Analyses/ZprimeToTt/RestFrames/lib:$LD_LIBRARY_PATH      # Linux, ELF HP-UX
else
   setenv LD_LIBRARY_PATH /groups/mhildret/rgoldouz/run3Analyses/ZprimeToTt/RestFrames/lib
endif

if ($?DYLD_LIBRARY_PATH) then
   setenv DYLD_LIBRARY_PATH /groups/mhildret/rgoldouz/run3Analyses/ZprimeToTt/RestFrames/lib:$DYLD_LIBRARY_PATH  # Mac OS X
else
   setenv DYLD_LIBRARY_PATH /groups/mhildret/rgoldouz/run3Analyses/ZprimeToTt/RestFrames/lib
endif

if ($?SHLIB_PATH) then
   setenv SHLIB_PATH /groups/mhildret/rgoldouz/run3Analyses/ZprimeToTt/RestFrames/lib:$SHLIB_PATH                # legacy HP-UX
else
   setenv SHLIB_PATH /groups/mhildret/rgoldouz/run3Analyses/ZprimeToTt/RestFrames/lib
endif

if ($?LIBPATH) then
   setenv LIBPATH /groups/mhildret/rgoldouz/run3Analyses/ZprimeToTt/RestFrames/lib:$LIBPATH                      # AIX
else
   setenv LIBPATH /groups/mhildret/rgoldouz/run3Analyses/ZprimeToTt/RestFrames/lib
endif

if ($?PYTHONPATH) then
   setenv PYTHONPATH /groups/mhildret/rgoldouz/run3Analyses/ZprimeToTt/RestFrames/lib:$PYTHONPATH
else
   setenv PYTHONPATH /groups/mhildret/rgoldouz/run3Analyses/ZprimeToTt/RestFrames/lib
endif

if ($?CPLUS_INCLUDE_PATH) then
   setenv CPLUS_INCLUDE_PATH /groups/mhildret/rgoldouz/run3Analyses/ZprimeToTt/RestFrames/include:$CPLUS_INCLUDE_PATH
else
   setenv CPLUS_INCLUDE_PATH /groups/mhildret/rgoldouz/run3Analyses/ZprimeToTt/RestFrames/include
endif

if ($?CPATH) then
   setenv CPATH /groups/mhildret/rgoldouz/run3Analyses/ZprimeToTt/RestFrames/include:$CPATH
else
   setenv CPATH /groups/mhildret/rgoldouz/run3Analyses/ZprimeToTt/RestFrames/include
endif

endif # if ("$setup_RestFrames" != "")

set restframessys=

