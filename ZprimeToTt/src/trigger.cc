#include "MyAnalysis.h"
#include "TH1EFT.h"

bool MyAnalysis::trigger(TString data,TString dataset,string year){

  bool triggerPass=false;
  bool triggerPassEE = false;
  bool triggerPassMu = false;
//trigger MC

    if(data == "mc" && year == "2024"){
      if(HLT_Mu50 || HLT_CascadeMu100 || HLT_HighPtTkMu100 || HLT_DoubleEle25_CaloIdL_MW) triggerPass =true;
    }

//trigger DATA
    if(data == "data"){
      if(year == "2024"){
        triggerPassMu = (HLT_Mu50 || HLT_CascadeMu100 || HLT_HighPtTkMu100);
        triggerPassEE = (HLT_DoubleEle25_CaloIdL_MW);
        if(dataset=="Muon"){
          if (triggerPassMu) triggerPass=true;
        }
        if(dataset=="EGamma"){
          if (!triggerPassMu && triggerPassEE) triggerPass=true;
        }
      }
    }
  return triggerPass;  

}
