#define MyAnalysis_cxx
#include "MyAnalysis.h"
#include "objectSelection.h"
#include "genLevelAnalysis.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include "PU_reWeighting.h"
#include "sumOfWeights.h"
#include "sumOfWeightsSignal.h"
#include "lepton_candidate.h"
#include "jet_candidate.h"
#include "LumiMask.h"
#include "TRandom.h"
#include "TRandom3.h"
#include "TDirectory.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TRandom3.h>
#include <TLorentzVector.h>
#include <time.h>
#include <cstdio>
#include <iostream>
#include <cmath>
#include <vector>
#include "RoccoR.h"
#if not defined(__CINT__) || defined(__MAKECINT__)
#include "TMVA/Tools.h"
#include "TMVA/Reader.h"
#include "TMVA/MethodCuts.h"
#include "GEScaleSyst.h"
#include "Utils.h"
#include "correction.h"
#include "WCPoint.h"
#include "WCFit.h"
#include "TH1EFT.h"
#include <map>
#include "sys/types.h"
#include "sys/sysinfo.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <regex>
#include <TFileMerger.h>
#include <malloc.h>
#include <thread>
#include <sys/resource.h>

#endif


#define COMPILER (!defined(__CINT__) && !defined(__CLING__))
#if defined(__MAKECINT__) || defined(__ROOTCLING__) || COMPILER
#include "RestFrames/RestFrames.hh"
#else
RestFrames::RFKey ensure_autoload(1);
#endif
using namespace RestFrames;
using namespace correction;
using namespace std;


bool isLastCopy(int statusFlags){
    return statusFlags & (1 << 13);
}
void resetVecInt(std::vector<int> &K){
  for (size_t i=0;i<K.size();++i){
    K[i]=-1;
  }
}

void resetVecIntofInt(std::vector<std::vector<int>> &K){
  for (size_t i=0;i<K.size();++i){
    for (size_t j=0;j<K[i].size();++j){
      K[i][j]=-1;
    }
  }
}

void resetVecFloat(std::vector<float> &K){
  for (size_t i=0;i<K.size();++i){
    K[i]=0;
  }
}

void resetVecFloatOfFloat(std::vector<std::vector<float>> &K){
  for (size_t i=0;i<K.size();++i){
    for (size_t j=0;j<K[i].size();++j){
      K[i][j]=0;
    }
  }
}


int parseLine(char* line){
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

int getValue(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    int resultVmm = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmRSS:", 6) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}


void printMemoryUsage() {
    struct rusage r_usage;
    getrusage(RUSAGE_SELF, &r_usage);
    std::cout << "Memory usage: " << r_usage.ru_maxrss/1000000.0 << " GB" << std::endl;
}

float topPtPowheg(float pt){
  return (0.973 - (0.000134 * pt) + (0.103 * exp(pt * (-0.0118))));
}

float topPtMGLO(float x){
  return (0.688 -  0.0000174*x + 0.824*exp(-0.0000253*x)/(pow(x,0.2185)));
}


bool ifSysNeeded(std::vector<lepton_candidate*> *lep, float cut){
  bool pass=false;
  if (lep->size()==2){
    if(((*lep)[0]->p4_ + (*lep)[1]->p4_).M()> cut) pass=true;
  }
  return pass;
}     

int countNonBjets(std::vector<jet_candidate*> *jet){
  int n=0;
  for (UInt_t l=0;l<jet->size();l++){
    if(!(*jet)[l]->btag_) n++;
  }
  return n;
}

void MyAnalysis::Analyze(TString fname, TString data, TString dataset ,string year, TString Run, float xs, float lumi, float Nevent, int iseft, int nRuns, int onlyGen, MyAnalysis *Evt){
  ROOT::DisableImplicitMT();
  if (fChain == 0) return;
  inputs(data, year);
  Long64_t ntr = fChain->GetEntries ();
  Long64_t CHUNK_SIZE = 1000;
  if (data == "data") CHUNK_SIZE = 1000;
  
  Long64_t chunkStart = 0;
  while (chunkStart < ntr) {
    Long64_t chunkEnd = std::min(chunkStart + CHUNK_SIZE, ntr);
    std::cout << "Processing events from " << chunkStart << " to " << chunkEnd - 1 << std::endl;

    Loop(fname, data, dataset , year, Run, xs, lumi, Nevent, iseft, nRuns, onlyGen, Evt, chunkStart, chunkEnd);

    chunkStart = chunkEnd; // move forward by the actual number of events processed

    if (memory < 1000000 && CHUNK_SIZE<25000) {
        std::cout << "Memory used less than 1GB (" << memory / 1000000.0 << "), doubling CHUNK_SIZE for next iteration." << std::endl;
        CHUNK_SIZE *= 2;
    }
  }
}

void MyAnalysis::Loop(TString fname, TString data, TString dataset ,string year, TString Run, float xs, float lumi, float Nevent, int iseft, int nRuns, int onlyGen, MyAnalysis *Evt, Long64_t start, Long64_t end){
//*** decide if you want to run with or without saving systematic uncertainty histograms
  bool ifSys=false;
//*** make the histograms and all needed information ready to start the analysis 
  initiateHists(data, year, ifSys);

//  genLevelAnalysis genAnalysis(Evt);

  TH1EFT  *crossSection = new TH1EFT("crossSection","crossSection",1,0,1);
  PU wPU;

//  auto myLumiMask = LumiMask::fromJSON(gLumiMask,fRun,lRun);
  TRandom3 Tr;

  float DibosonCorr[10]={1.0,1.0,1.20, 1.48, 1.77, 2.05,2.34, 2.62,1.0,1.0};
  Double_t ptBins[11] = {30., 40., 60., 80., 100., 150., 200., 300., 400., 500., 1000.};
  Double_t etaBins [4]= {0., 0.6, 1.2, 2.4};

  TH1F *mll_SS_Zwindow_0jet = new TH1F("mll_SS_Zwindow_0jet","mll_SS_Zwindow_0jet",10,85,95);

  TH2D *h2_BTaggingEff_Denom_b    = new TH2D("h2_BTaggingEff_Denom_b"   , ";p_{T} [GeV];#eta", 10 , ptBins, 3 , etaBins);
  TH2D *h2_BTaggingEff_Denom_c    = new TH2D("h2_BTaggingEff_Denom_c"   , ";p_{T} [GeV];#eta", 10 , ptBins, 3 , etaBins);
  TH2D *h2_BTaggingEff_Denom_udsg = new TH2D("h2_BTaggingEff_Denom_udsg", ";p_{T} [GeV];#eta", 10 , ptBins, 3 , etaBins);
  TH2D *h2_BTaggingEff_Num_b      = new TH2D("h2_BTaggingEff_Num_b"     , ";p_{T} [GeV];#eta", 10 , ptBins, 3 , etaBins);
  TH2D *h2_BTaggingEff_Num_c      = new TH2D("h2_BTaggingEff_Num_c"     , ";p_{T} [GeV];#eta", 10 , ptBins, 3 , etaBins);
  TH2D *h2_BTaggingEff_Num_udsg   = new TH2D("h2_BTaggingEff_Num_udsg"  , ";p_{T} [GeV];#eta", 10 , ptBins, 3 , etaBins);


  TH1F  *tTagProbQcdAll = new TH1F("tTagProbQcdAll","tTagProbQcdAll",110,0,1.1);
  TH1F  *tTagProbQcdB = new TH1F("tTagProbQcdB","tTagProbQcdB",110,0,1.1);  
  TH1F  *tTagProbQcdNonB = new TH1F("tTagProbQcdNonB","tTagProbQcdNonB",110,0,1.1);
  TH1F  *tTagProbTop = new TH1F("tTagProbTop","tTagProbTop",110,0,1.1);

  double binsPtH[]={200, 300, 400, 500,800,1200,2000};
  double binsEta[]={-2.5,-2.25,-2,-1.75,-1.5,-1.25,-1,-0.75,-0.5,-0.25,0,0.25,0.5,0.75,1,1.25,1.5,1.75,2,2.25,2.5};  
  double binsmass[]={0, 10, 20, 40, 80, 110, 150, 200, 300, 400, 500};
  TH1F  *tTag_ee_pt_Den = new TH1F("tTag_ee_pt_Den","tTag_ee_pt_Den",sizeof(binsPtH)/sizeof(double) - 1, binsPtH);
  TH1F  *tTag_ee_pt_Num = new TH1F("tTag_ee_pt_Num","tTag_ee_pt_Num",sizeof(binsPtH)/sizeof(double) - 1, binsPtH);	  
  TH1F  *tTag_ee_eta_Den = new TH1F("tTag_ee_eta_Den","tTag_ee_eta_Den",sizeof(binsEta)/sizeof(double) - 1, binsEta);
  TH1F  *tTag_ee_eta_Num = new TH1F("tTag_ee_eta_Num","tTag_ee_eta_Num",sizeof(binsEta)/sizeof(double) - 1, binsEta);
  TH1F  *tTag_ee_mass_Den = new TH1F("tTag_ee_mass_Den","tTag_ee_mass_Den",sizeof(binsmass)/sizeof(double) - 1, binsmass);
  TH1F  *tTag_ee_mass_Num = new TH1F("tTag_ee_mass_Num","tTag_ee_mass_Num",sizeof(binsmass)/sizeof(double) - 1, binsmass);
  TH1F  *tTag_ee_lfrac_Den = new TH1F("tTag_ee_lfrac_Den","tTag_ee_lfrac_Den",10,0,10);
  TH1F  *tTag_ee_lfrac_Num = new TH1F("tTag_ee_lfrac_Num","tTag_ee_lfrac_Num",10,0,10);

  TH1F  *tTag_mm_pt_Den = new TH1F("tTag_mm_pt_Den","tTag_mm_pt_Den",sizeof(binsPtH)/sizeof(double) - 1, binsPtH);
  TH1F  *tTag_mm_pt_Num = new TH1F("tTag_mm_pt_Num","tTag_mm_pt_Num",sizeof(binsPtH)/sizeof(double) - 1, binsPtH);
  TH1F  *tTag_mm_eta_Den = new TH1F("tTag_mm_eta_Den","tTag_mm_eta_Den",sizeof(binsEta)/sizeof(double) - 1, binsEta);
  TH1F  *tTag_mm_eta_Num = new TH1F("tTag_mm_eta_Num","tTag_mm_eta_Num",sizeof(binsEta)/sizeof(double) - 1, binsEta);
  TH1F  *tTag_mm_mass_Den = new TH1F("tTag_mm_mass_Den","tTag_mm_mass_Den",sizeof(binsEta)/sizeof(double) - 1, binsmass);
  TH1F  *tTag_mm_mass_Num = new TH1F("tTag_mm_mass_Num","tTag_mm_mass_Num",sizeof(binsEta)/sizeof(double) - 1, binsmass);
  TH1F  *tTag_mm_lfrac_Den = new TH1F("tTag_mm_lfrac_Den","tTag_mm_lfrac_Den",10,0,10);
  TH1F  *tTag_mm_lfrac_Num = new TH1F("tTag_mm_lfrac_Num","tTag_mm_lfrac_Num",10,0,10);


  TH2F  *jetVetoMap_before = new TH2F("jetVetoMap_before","jetVetoMap_before",20,-2.6,2.6,20,-3.2,3.2);
  TH2F  *jetVetoMap_after = new TH2F("jetVetoMap_after","jetVetoMap_after",20,-2.6,2.6,20,-3.2,3.2);

  TH1F  *triggEff_denom_mll = new TH1F("triggEff_denom_mll","triggEff_denom_mll",100,0,1500);
  TH1F  *triggEff_num_mll = new TH1F("triggEff_num_mll","triggEff_num_mll",100,0,1500);
  TH1F  *triggEff_denom_ptl1 = new TH1F("triggEff_denom_ptl1","triggEff_denom_ptl1",100,0,1500);
  TH1F  *triggEff_num_ptl1 = new TH1F("triggEff_num_ptl1","triggEff_num_ptl1",100,0,1500);

  TH1F  *cutFlow = new TH1F("cutFlow","cutFlow",20,0,20);
  TH1F  *cutFlowFinal = new TH1F("cutFlowFinal","cutFlowFinal",20,0,20);
  TH1F  *tagEff = new TH1F("tagEff","tagEff",20,0,20);
  TH1F  *mass2 = new TH1F("mass2","mass2",100,0,6000);
  TH1F  *mass1 = new TH1F("mass1","mass1",100,0,6000);
  TH1F  *mass3 = new TH1F("mass3","mass3",100,0,6000);
  TH1F  *mass4 = new TH1F("mass4","mass4",100,0,6000);
  TH1F  *RECOmass0 = new TH1F("RECOmass0","RECOmass0",100,0,6000);
  TH1F  *RECOmass1 = new TH1F("RECOmass1","RECOmass1",100,0,6000);
  TH1F  *RECOmass2 = new TH1F("RECOmass2","RECOmass2",100,0,6000);  
  TH1F  *lep0Pt = new TH1F("lep0Pt","lep0Pt",100,-200,200);

  std::vector<string> wc_names_lst={};
  std::vector<string> wc_names_lst_BNV={"cT", "cS"};
  std::vector<string> wc_names_lst_FCNC={"ctp","ctlS","cte","ctl","ctlT","ctZ","cpt","cpQM","ctA","cQe","ctG","cQlM"};
  std::vector<string> wc_names_lst_SMEFT={"ctu1","cqd1","cqq13","ctu8","cqu1","cqq11","cqq83","ctd1","ctd8","ctg","ctq1","cqq81","cqu8","cqd8","ctq8"};
  if (fname.Contains("BNV") && iseft) wc_names_lst = wc_names_lst_BNV;
  if (fname.Contains("FCNC") && iseft) wc_names_lst = wc_names_lst_FCNC;
  if (fname.Contains("SMEFT") && iseft) wc_names_lst = wc_names_lst_SMEFT;
  string listWC="";

  WCFit *eft_fit;

  TLorentzVector wp, wm, b, ab, top, atop;
  TLorentzVector recoTop, recoBjet, recoW, recoNu, recoL1, recoL2, highPtMu;
  TLorentzVector recoTop2, recoTop1;
  bool leptonPass;
  bool metFilterPass;
  int chNonIso;
  int chIso;
  int chOSppWeighted;
  int sumCharge;
  int sumFlavour;
  int myreg;
  bool onZ;
  bool offZhigh;
  float weight_Lumi;
  float weight_lep;
  float weight_lepB;
  float weight_EFT;
  int nAccept=0;
  float sumWeight=0;
  int nbjet;
  int nbjetOut;
  int nbjetIn;
  int ntjet;
  int nbSubjet;
  int nIsolOut;
  int nNonIsolIn;
  int tTagIndex;
  float drll;
  float mll;
  float HTjets;
  float MVAS_TU;
  float MVAB_TU;
  float MVAS_TC;
  float MVAB_TC;
  float MVAS_TUFA;
  float MVAB_TUFA;
  float MVAS_TCFA;
  float MVAB_TCFA;
  std::vector<Float_t> probs;
  float rawBDT;
  float ttKFactor;
  bool topEvent;
  bool lep0IntJet;
  bool lep1IntJet;

  //in order to save systematic variations in one histogram, I use the TH1EFT feature and consider each EFT constant for knowing sum of event weights in each bin
  //for Dim number of WC, TH1EFT keep track of (Dim + 1) * (Dim + 2) / 2 constant. We use one constant for one uncertainty
  //number of systematic sources is 17 so 5 constant is enough
  const int Dim = 3;  
  const int N = (Dim + 1) * (Dim + 2) / 2;
  Float_t sysFitCoefficientsUp[N];
  Float_t sysFitCoefficientsDown[N];
  std::vector<std::string> sys_std;
  for (int i = 0; i < Dim; ++i) {
        sys_std.push_back("S" + std::to_string(i));
  }
  const int DimTh = 14;
  const int NTh = (DimTh + 1) * (DimTh + 2) / 2;
  Float_t sysThFitCoefficients[NTh];
  std::vector<std::string> sysTh_std;
  for (int i = 0; i < DimTh; ++i) {
        sysTh_std.push_back("Th" + std::to_string(i));
  }
  
  // Fill all with zero

  UInt_t nss=channels.size();
  std::vector<int> reg(regions.size());
  std::vector<int> allCh(nss);
  std::vector<int> allChFA(1);
  std::vector<std::vector<float>> wgt(nss);
  std::vector<std::vector<float>> wgtSysUp(nss);
  std::vector<std::vector<float>> wgtSysDown(nss);
  for (UInt_t i = 0 ; i < nss ; i++) {
    wgt[i].resize(regions.size());
    wgtSysUp[i].resize(regions.size());
    wgtSysDown[i].resize(regions.size());
  }
  std::vector<std::vector<WCFit>> wcfit(nss);
  std::vector<std::vector<WCFit>> wcfitSysUp(nss);
  std::vector<std::vector<WCFit>> wcfitSysDown(nss);
  for (UInt_t i = 0 ; i < nss ; i++) {
    wcfit[i].resize(regions.size());
    wcfitSysUp[i].resize(regions.size());
    wcfitSysDown[i].resize(regions.size());
  }


  std::vector<std::vector<float>> wgtFA(1);
  wgtFA[0].resize(regions.size());
  std::vector<std::vector<WCFit>> wcfitFA(1);
  wcfitFA[0].resize(regions.size());
  int nLHEl;
  float fakeRate;
  std::vector<float> sysDownWeightsFA;
  std::vector<float> sysUpWeightsFA;
  sysUpWeightsFA.assign(sysFA.size(), 1);
  sysDownWeightsFA.assign(sysFA.size(), 1);
  float fi;
  float probChOS;
  int lZp;
  int lZfp;
  bool isHem;
  float HemMcWeight;
  float metUnclusMETUp;
  float metUnclusMETDown;
  float metUnclusMETPhiUp;
  float metUnclusMETPhiDown;
  bool DyInZPass;
  bool MetPass;
  float MetCut=25;
  TLorentzVector top_tZFCNC, bt_tZFCNC, Wt_tZFCNC, lt_tZFCNC, nut_tZFCNC, Z_tZFCNC, lpZ_tZFCNC, lmZ_tZFCNC ;
  TLorentzVector hL_tHFCNC, hNu_tHFCNC, hU_tHFCNC, hD_tHFCNC, tNu_tHFCNC, tB_tHFCNC, tL_tHFCNC, met_tHFCNC;
  WCPoint *A = new WCPoint("EFTrwgt4_cpQM_1.0_cpt_1.0_ctA_1.0_ctZ_0.5_ctG_0.1_cQlM_1.0_cQe_1.0_ctl_1.0_cte_1.0_ctlS_1.0_ctlT_0.05_ctp_1.0");

  if (fChain == 0) return;
  Long64_t nentries = fChain->GetEntriesFast();
  Long64_t nbytes = 0, nb = 0;
  Long64_t ntr = fChain->GetEntries ();

//Loop over events
cout<<"starting loop with  Virtual Memory: "<<getValue()/1048576.0<<" GB"<<endl;
//  for (Long64_t jentry=0; jentry<nentries;jentry++) {
  for (Long64_t jentry=start; jentry<end;jentry++) {
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    nb = fChain->GetEntry(jentry);   nbytes += nb;
    displayProgress(jentry, ntr) ;
    if (jentry % 1000 == 0) cout<<"Virtual Memory: "<<getValue()/1048576.0<<" GB"<<endl;

//##    tH_topMass_=-999;    tH_HMass_=-999;    tH_WtopMass_=-999;    tH_W1HMass_=-999;    tH_W2HMass_=-999;    tH_HPt_=-999;    tH_HEta_=-999;    tH_topPt_=-999;    tH_topEta_=-999;    tH_drWtopB_=-999;    tH_drW1HW2H_=-999;
//##    tZ_topMass_=-999; tZ_ZMass_=-999; tZ_WtopMass_=-999; tZ_ZPt_=-999; tZ_ZEta_=-999; tZ_topPt_=-999; tZ_topEta_=-999;
//##//write out the WCs at the begining of the code to make sure what you have set is used in the sample
//##
//##//    if(jentry==0 && fname.Contains("FCNC")){
//##//      listWC="";
//##//      for (UInt_t i=0;i<nWCnames;++i){
//##//         char ch[4];
//##//         for(int j = 0; j < 4; j++) ch[j] = (WCnames[i] >> (4-1-j)*8) & 0xFF;
//##//         for(int n=0 ; n<4 ; ++n) listWC += ch[n];
//##//      }
//##//      cout<<"\n original WC list= {"<<listWC<<"}"<<endl;
//##//      listWC=std::regex_replace(listWC, std::regex("-"), std::string(""));
//##//      listWC=std::regex_replace(listWC, std::regex("c"), std::string("\",\"c"));
//##//      listWC.erase(0,2);
//##//      listWC+="\"";
//##//      cout<<"\n WC list = {"<<listWC<<"}"<<endl;
//##//      cout<<"\n  WC list in this code = {";
//##//      for (auto i: wc_names_lst) {
//##//        std::cout << i << ',';} cout<<"}"<<endl;
//##//      }
//##
//##    //put the Lumi mask
//##    if(data == "data"){
//##      if(!myLumiMask.accept(run, luminosityBlock)) continue;
//##    }
//##   //photon overlap removal
//##   //find the cuts here: /cvmfs/cms.cern.ch/phys_generator/gridpacks/UL/13TeV/madgraph/V5_2.6.5/TTGamma_Dilept/ttGamma_Dilept_5f_ckm_LO_1line_slc7_amd64_gcc700_CMSSW_10_6_19_tarball.tar.xz
//##   //find the cuts here: /cvmfs/cms.cern.ch/phys_generator/gridpacks/2017/13TeV/madgraph/V5_2.6.1/QCD_ZA_FXFX/ZATo2LA01j_5f_NLO_FXFX_slc6_amd64_gcc481_CMSSW_7_1_30_tarball.tar.xz
//##   if(data == "mc" && fname.Contains("TTTo") || fname.Contains("WJetsToLNu") || fname.Contains("DY") || fname.Contains("t_channel")){
//##     if (overlapRemoval(15.0, 5.0, 0.4, false,false)) {
//##       continue;
//##     }
//##   }
//##   if(data == "mc" && fname.Contains("TTGamma") || fname.Contains("WGToLNuG") || fname.Contains("ZGToLLG") || fname.Contains("TGJets")){
//##     if (overlapRemoval(15.0, 5.0, 0.4, false,true)) {
//##       continue;
//##     }
//##   }
//##//    if(jentry==0 && fname.Contains("FCNC")){
//##//      listWC="";
//##//      for (UInt_t i=0;i<nWCnames;++i){
//##//         char ch[4];
//##//         for(int j = 0; j < 4; j++) ch[j] = (WCnames[i] >> (4-1-j)*8) & 0xFF;
//##//         for(int n=0 ; n<4 ; ++n) listWC += ch[n];
//##//      }
//##//      cout<<"\n original WC list= {"<<listWC<<"}"<<endl;
//##//      listWC=std::regex_replace(listWC, std::regex("-"), std::string(""));
//##//      listWC=std::regex_replace(listWC, std::regex("c"), std::string("\",\"c"));
//##//      listWC.erase(0,2);
//##//      listWC+="\"";
//##//      cout<<"\n WC list = {"<<listWC<<"}"<<endl;
//##//      cout<<"\n  WC list in this code = {";
//##//      for (auto i: wc_names_lst) {
//##//        std::cout << i << ',';} cout<<"}"<<endl;
//##//    }
//##
//##    Z_tZFCNC.SetPtEtaPhiM(0,0,0,0); top_tZFCNC.SetPtEtaPhiM(0,0,0,0); bt_tZFCNC.SetPtEtaPhiM(0,0,0,0); Wt_tZFCNC.SetPtEtaPhiM(0,0,0,0); lt_tZFCNC.SetPtEtaPhiM(0,0,0,0); lpZ_tZFCNC.SetPtEtaPhiM(0,0,0,0); lmZ_tZFCNC.SetPtEtaPhiM(0,0,0,0);
//##    if(data == "mc" && fname.Contains("FCNC")){
//##      for (UInt_t l=0;l<nGenPart;l++){
//##        if(isnan(GenPart_pt[l]) || isinf(GenPart_pt[l]) || GenPart_pt[l]==0) continue;
//##//        if(GenPart_statusFlags[l] & (1<<13)){
//##//          cout<<l<<":"<<GenPart_pdgId[l]<<":"<<GenPart_pdgId[GenPart_genPartIdxMother[l]]<<":pt:"<<GenPart_pt[l]<<endl;
//##          if(abs(GenPart_pdgId[l])==24) Wt_tZFCNC.SetPtEtaPhiM(GenPart_pt[l], GenPart_eta[l], GenPart_phi[l], GenPart_mass[l]);
//##          if(abs(GenPart_pdgId[l])==6) top_tZFCNC.SetPtEtaPhiM(GenPart_pt[l], GenPart_eta[l], GenPart_phi[l], GenPart_mass[l]);
//##          if(abs(GenPart_pdgId[l])==23) Z_tZFCNC.SetPtEtaPhiM(GenPart_pt[l], GenPart_eta[l], GenPart_phi[l], GenPart_mass[l]);
//##          if(abs(GenPart_pdgId[l])==11 || abs(GenPart_pdgId[l])==13 || abs(GenPart_pdgId[l])==15){
//##            if(abs(GenPart_pdgId[GenPart_genPartIdxMother[l]])==24) lt_tZFCNC.SetPtEtaPhiM(GenPart_pt[l], GenPart_eta[l], GenPart_phi[l], GenPart_mass[l]);
//##            if(abs(GenPart_pdgId[GenPart_genPartIdxMother[l]])==23 && GenPart_pdgId[l]>0) lpZ_tZFCNC.SetPtEtaPhiM(GenPart_pt[l], GenPart_eta[l], GenPart_phi[l], GenPart_mass[l]);
//##            if(abs(GenPart_pdgId[GenPart_genPartIdxMother[l]])==23 && GenPart_pdgId[l]<0) lmZ_tZFCNC.SetPtEtaPhiM(GenPart_pt[l], GenPart_eta[l], GenPart_phi[l], GenPart_mass[l]);
//##          }
//##          if(abs(GenPart_pdgId[l])==12 || abs(GenPart_pdgId[l])==14 || abs(GenPart_pdgId[l])==16){
//##            if(abs(GenPart_pdgId[GenPart_genPartIdxMother[l]])==24) nut_tZFCNC.SetPtEtaPhiM(GenPart_pt[l], 0, GenPart_phi[l], 0);
//##          }
//##          if(abs(GenPart_pdgId[l])==5) bt_tZFCNC.SetPtEtaPhiM(GenPart_pt[l], GenPart_eta[l], GenPart_phi[l], GenPart_mass[l]);
//##//        }
//##      }
//##    if(Z_tZFCNC.Pt()>0 && lpZ_tZFCNC.Pt()>15 && lmZ_tZFCNC.Pt()>15){
//##       jigsawTzFCNC->Analyze(lt_tZFCNC, lpZ_tZFCNC, lmZ_tZFCNC, bt_tZFCNC,nut_tZFCNC);
//##       JigsawWmass->Fill(jigsawTzFCNC->W->GetMass());
//##       JigsawTmass->Fill(jigsawTzFCNC->T->GetMass());
//##//cout<<jigsawTzFCNC->W->GetMass()<<":"<<(lt_tZFCNC+nut_tZFCNC).M()<<endl;
//##    }
//##//tH FCNC
//##      hL_tHFCNC.SetPtEtaPhiM(0,0,0,0); hNu_tHFCNC.SetPtEtaPhiM(0,0,0,0); hU_tHFCNC.SetPtEtaPhiM(0,0,0,0); hD_tHFCNC.SetPtEtaPhiM(0,0,0,0); tNu_tHFCNC.SetPtEtaPhiM(0,0,0,0); tB_tHFCNC.SetPtEtaPhiM(0,0,0,0); tL_tHFCNC.SetPtEtaPhiM(0,0,0,0);
//##      for (UInt_t l=0;l<nGenPart;l++){
//##        if(isnan(GenPart_pt[l]) || isinf(GenPart_pt[l]) || GenPart_pt[l]==0) continue;
//##          if(abs(GenPart_pdgId[l])==11 || abs(GenPart_pdgId[l])==13 || abs(GenPart_pdgId[l])==15){
//##             if(abs(GenPart_pdgId[GenPart_genPartIdxMother[l]])==24){
//##               if(tL_tHFCNC.Pt()==0) tL_tHFCNC.SetPtEtaPhiM(GenPart_pt[l], GenPart_eta[l], GenPart_phi[l], GenPart_mass[l]);
//##               else hL_tHFCNC.SetPtEtaPhiM(GenPart_pt[l], GenPart_eta[l], GenPart_phi[l], GenPart_mass[l]);
//##             }
//##           }
//##          if(abs(GenPart_pdgId[l])==5) tB_tHFCNC.SetPtEtaPhiM(GenPart_pt[l], GenPart_eta[l], GenPart_phi[l], GenPart_mass[l]);
//##          if(abs(GenPart_pdgId[l])==2 || abs(GenPart_pdgId[l])==4 ){
//##             if(abs(GenPart_pdgId[GenPart_genPartIdxMother[l]])==24) hU_tHFCNC.SetPtEtaPhiM(GenPart_pt[l], GenPart_eta[l], GenPart_phi[l], GenPart_mass[l]);
//##           }
//##          if(abs(GenPart_pdgId[l])==1 || abs(GenPart_pdgId[l])==3 ){
//##             if(abs(GenPart_pdgId[GenPart_genPartIdxMother[l]])==24) hD_tHFCNC.SetPtEtaPhiM(GenPart_pt[l], GenPart_eta[l], GenPart_phi[l], GenPart_mass[l]);
//##           }
//##       }
//##       met_tHFCNC.SetPtEtaPhiM(GenMET_pt,0,GenMET_phi,0);
//##//      if(tL_tHFCNC.Pt()>0 && hL_tHFCNC.Pt()>15 && hU_tHFCNC.Pt()>15){
//##//       jigsawThFCNC->Analyze(tL_tHFCNC, hL_tHFCNC, tB_tHFCNC, hU_tHFCNC, hD_tHFCNC, met_tHFCNC);
//##//       JigsawWmass_tHFCNC->Fill(jigsawThFCNC->WT->GetMass());
//##//       JigsawTmass_tHFCNC->Fill(jigsawThFCNC->T->GetMass());
//##//       JigsawHmass_tHFCNC->Fill(jigsawThFCNC->H->GetMass());
//##//      }
//##//cout<<"Gen top mass:"<<top_tZFCNC.M()<<" W:"<<Wt_tZFCNC.M()<<" Z:"<<Z_tZFCNC.M()<<" top l pt:"<<lt_tZFCNC.Pt()<<" b pt:"<<bt_tZFCNC.Pt()<<" l+ pt:"<<lpZ_tZFCNC.Pt()<<" l- pt:"<<lmZ_tZFCNC.Pt()<<endl;
//##    }
//##//    if(data == "mc" && onlyGen){
//##//      if(iseft && fname.Contains("rwgt")) genAnalysis.fillGENHists(1.0/nRuns,wc_names_lst);
//##//      else if(iseft && !fname.Contains("rwgt")) genAnalysis.fillGENHists(LHEWeight_originalXWGTUP/nRuns,wc_names_lst);
//##//      else  genAnalysis.fillGENHists(xs/Nevent,wc_names_lst);
//##//      continue;
//##//    }
//##//    if(data == "mc" && !onlyGen){
//##//      if(iseft && fname.Contains("rwgt")) genAnalysis.fillGENHists(1.0/nRuns,wc_names_lst);
//##//      else if(iseft && !fname.Contains("rwgt")) genAnalysis.fillGENHists(LHEWeight_originalXWGTUP/nRuns,wc_names_lst);
//##//      else  genAnalysis.fillGENHists(xs/Nevent,wc_names_lst);
//##//    }
//##//    nLHEl=0;
//##//    if(data == "mc"){
//##//      for (UInt_t l=0;l<nLHEPart;l++){
//##//        if(abs(LHEPart_pdgId[l]) ==11 || abs(LHEPart_pdgId[l]) ==13 ){
//##//          if(LHEPart_pt[l]>20 && abs(LHEPart_eta[l])<2.4) nLHEl++;
//##//        }
//##//      }
//##//      for (UInt_t l=0;l<nGenPart;l++){
//##//        if(abs(GenPart_pdgId[l])==11 || abs(GenPart_pdgId[l])==13){
//##//          if(abs(GenPart_pdgId[GenPart_genPartIdxMother[l]])==24 && GenPart_pt[l]>20 &&  abs(GenPart_eta[l])<2.4) nLHEl++;
//##//        }
//##//      }
//##//    }
//##
    topEvent=false;
    metFilterPass = false;
    leptonPass = false;
    chNonIso =100;
    chIso=100;
//##    chOSppWeighted=100;
    reg_=100;
    sumCharge =0;
    sumFlavour = 0;
    onZ = false;
    offZhigh = false;
    weight_Lumi =1;
    weight_lep =1;
    weight_lepB =1;
//##    weight_EFT =1;
    nbjet=0;
    nbjetOut=0;
    nbjetIn=0;
    ntjet=0;
    nbSubjet=0;
    nIsolOut=0;
    nNonIsolIn=0;
    DyInZPass  = false;
    MetPass = false;
    tTagIndex = 0;
    lep0IntJet = false;
    lep1IntJet = false;
//##    MVAS_TU=0;
//##    MVAB_TU=1;
//##    MVAS_TC=0;
//##    MVAB_TC=1;
//##    MVAS_TUFA=0;
//##    MVAB_TUFA=1;
//##    MVAS_TCFA=0;
//##    MVAB_TCFA=1;
//##    HTjets=0;
//##    ttKFactor=1;
//##    HemMcWeight=1;
//##    if (fname.Contains("Decay")) ttKFactor=831.7/445.0;
//##    else ttKFactor=1;
//##
//##    fakeRate=1;
//##    fi=1;
//##    probChOS=1;
//##    for (int n=0;n<sysFA.size();++n){
//##      sysUpWeightsFA[n] =1;
//##      sysDownWeightsFA[n] =1;
//##    }
//##    if (iseft) {
//##      eft_fit = new WCFit(nWCnames, wc_names_lst, nEFTfitCoefficients, EFTfitCoefficients, (xs/Nevent)*(1.0/LHEWeight_originalXWGTUP));
//##//      eft_fit = new WCFit(nWCnames, wc_names_lst, nEFTfitCoefficients, EFTfitCoefficients, 1.0/nRuns);
//##//      for (UInt_t i=0;i<nWCnames;++i){
//##//         char ch[4];
//##////         for(int j = 0; j < 4; j++) ch[j] = (WCnames[i] >> (4-1-j)*8) & 0xFF;
//##//         cout<< " - "<<WCnames[i] <<endl;
//##////         for(int n=0 ; n<4 ; ++n) cout << ch[n];
//##////         cout<<" : "<<endl;;
//##//      }
//##      
//##    }
//##    else {
//##      eft_fit = new WCFit(0,wc_names_lst,1, &genWeight, 1.0); 
//##    }
//##//cout<<run<<":"<<luminosityBlock<<":"<<event<<endl;
//##    for (UInt_t l=0;l<wc_names_lst.size();l++){
//##      if(wc_names_lst[l]!="cpQM") continue;
//##      A= new WCPoint("EFTrwgt131_"+wc_names_lst[l]+"_1.0");
//##//      cout<<"EFTrwgt131_"+wc_names_lst[l]+"_1.0"<<endl;
//##//      if(eft_fit->evalPoint(A)>0.1) cout<<wc_names_lst[l]<<":"<<eft_fit->evalPoint(A)<<endl;
//##      delete A;
//##    }
//##    crossSection->Fill(0.5, 1,*eft_fit);
//##//cout<<eft_fit->getCoefficient("sm","sm")<<endl;
//##//cout<<eft_fit->getCoefficient("ctp","ctp")<<endl;
//##    delete eft_fit; 
// MET Filters https://twiki.cern.ch/twiki/bin/view/CMS/MissingETOptionalFiltersRun2#Run_3_2024_data_and_MC_Recommend
    objectSelection(data,year);

    //histograms for calculating the b-tag/misstag efficiencies 
    for (int l=0;l<selectedJets->size();l++){
      if( abs((*selectedJets)[l]->flavor_) == 5){
        h2_BTaggingEff_Denom_b->Fill((*selectedJets)[l]->pt_, abs((*selectedJets)[l]->eta_));
        if( (*selectedJets)[l]->btag_ ) {
          h2_BTaggingEff_Num_b->Fill((*selectedJets)[l]->pt_, abs((*selectedJets)[l]->eta_));	   
	}
      }
      if( abs((*selectedJets)[l]->flavor_) == 4){
        h2_BTaggingEff_Denom_c->Fill((*selectedJets)[l]->pt_, abs((*selectedJets)[l]->eta_));
        if( (*selectedJets)[l]->btag_) {
          h2_BTaggingEff_Num_c->Fill((*selectedJets)[l]->pt_, abs((*selectedJets)[l]->eta_));
        }
      }
      if( abs((*selectedJets)[l]->flavor_) != 4 && abs((*selectedJets)[l]->flavor_) != 5){
        h2_BTaggingEff_Denom_udsg->Fill((*selectedJets)[l]->pt_, abs((*selectedJets)[l]->eta_));
        if( (*selectedJets)[l]->btag_) {
          h2_BTaggingEff_Num_udsg->Fill((*selectedJets)[l]->pt_, abs((*selectedJets)[l]->eta_));
	  }
      }
    }

    //histograms for calculating the top tagging efficiencies
    int nLeptons = 0;
    TLorentzVector Tlep1, Tlep2, b1, b2;
    if (data == "mc") {
      bool topEvent = false;
      for (int l = 0; l < nGenPart; ++l) {
        if (abs(GenPart_pdgId[l]) == 6) {
          topEvent = true;
          break;
        }
      }
      if (topEvent) {
        for (int i = 0; i < nLHEPart; ++i) {
          const int id = LHEPart_pdgId[i];
          if (id == 5) b1.SetPtEtaPhiM(LHEPart_pt[i], LHEPart_eta[i], LHEPart_phi[i], 0.);
          else if (id == -5)  b2.SetPtEtaPhiM(LHEPart_pt[i], LHEPart_eta[i], LHEPart_phi[i], 0.);    
          else if (id == 11 || id == 13 || id == 15) Tlep2.SetPtEtaPhiM(LHEPart_pt[i], LHEPart_eta[i], LHEPart_phi[i], 0.);    
          else if (id == -11 || id == -13 || id == -15) Tlep1.SetPtEtaPhiM(LHEPart_pt[i], LHEPart_eta[i], LHEPart_phi[i], 0.);
          if (abs(id) == 11 || abs(id) == 13 || abs(id) == 15)
            ++nLeptons;
        }
      }
    
      // Loop over AK8 jets
      for (size_t l = 0; l < selectedJets08->size(); ++l) {
        auto *jet = (*selectedJets08)[l];
        tTagProbQcdAll->Fill(jet->toptag_in_);
        int genIdx = FatJet_genJetAK8Idx[jet->indice_];
        if (genIdx >= 0 && GenJetAK8_hadronFlavour[genIdx] == 5) tTagProbQcdB->Fill(jet->toptag_in_);
        else  tTagProbQcdNonB->Fill(jet->toptag_in_);
        if (!topEvent) continue;
        TLorentzVector jetP4;
        jetP4.SetPtEtaPhiM(jet->pt_, jet->eta_, jet->phi_, jet->mass_);
        bool matched = false;
        // Require one b and one lepton inside the jet cone
        if (b1.Pt() > 0 && Tlep1.Pt() > 0 &&  jetP4.DeltaR(b1) < 0.8 && jetP4.DeltaR(Tlep1) < 0.8) matched = true;
        if (b2.Pt() > 0 && Tlep2.Pt() > 0 && jetP4.DeltaR(b2) < 0.8 && jetP4.DeltaR(Tlep2) < 0.8) matched = true;
        if (matched) tTagProbTop->Fill(jet->toptag_in_);
      }

      if(topEvent){
        float WP=0.85;
        int ttag=0;
        bool ifeve = false;
	bool ifeveIso = false;
    if(selectedNonIsoLeptons->size() >=2) drll=deltaR((*selectedNonIsoLeptons)[0]->eta_,(*selectedNonIsoLeptons)[0]->phi_,(*selectedNonIsoLeptons)[1]->eta_,(*selectedNonIsoLeptons)[1]->phi_);
    if(selectedNonIsoLeptons->size() >=2 && (*selectedNonIsoLeptons)[0]->lep_ == 1 &&  (*selectedNonIsoLeptons)[1]->lep_==1 && drll>0.8) ifeve=true;
    if(selectedNonIsoLeptons->size() >=2 && (*selectedNonIsoLeptons)[0]->lep_ == 1 &&  (*selectedNonIsoLeptons)[1]->lep_==10 && drll>0.8) ifeve=true;
    if(selectedNonIsoLeptons->size() >=2 && (*selectedNonIsoLeptons)[0]->lep_ == 10 &&  (*selectedNonIsoLeptons)[1]->lep_==1  && drll>0.8) ifeve=true;
    if(selectedNonIsoLeptons->size() >=2 && (*selectedNonIsoLeptons)[0]->lep_ == 10 &&  (*selectedNonIsoLeptons)[1]->lep_==10 && drll>0.8) ifeve=true;
    if(selectedOldIsoLeptons->size() >=2) drll=deltaR((*selectedOldIsoLeptons)[0]->eta_,(*selectedOldIsoLeptons)[0]->phi_,(*selectedOldIsoLeptons)[1]->eta_,(*selectedOldIsoLeptons)[1]->phi_);
    if(selectedOldIsoLeptons->size() >=2 && (*selectedOldIsoLeptons)[0]->lep_ == 1 &&  (*selectedOldIsoLeptons)[1]->lep_==1 && drll>0.8) ifeveIso=true;
    if(selectedOldIsoLeptons->size() >=2 && (*selectedOldIsoLeptons)[0]->lep_ == 1 &&  (*selectedOldIsoLeptons)[1]->lep_==10 && drll>0.8) ifeveIso=true;
    if(selectedOldIsoLeptons->size() >=2 && (*selectedOldIsoLeptons)[0]->lep_ == 10 &&  (*selectedOldIsoLeptons)[1]->lep_==1  && drll>0.8) ifeveIso=true;
    if(selectedOldIsoLeptons->size() >=2 && (*selectedOldIsoLeptons)[0]->lep_ == 10 &&  (*selectedOldIsoLeptons)[1]->lep_==10 && drll>0.8) ifeveIso=true;

    std::vector<int> topJets;
    for (int l=0;l<nFatJet;l++){
      if((FatJet_globalParT3_TopbWev[l]+FatJet_globalParT3_TopbWmv[l])/(FatJet_globalParT3_TopbWev[l]+FatJet_globalParT3_TopbWmv[l]+FatJet_globalParT3_QCD[l])>WP) {
          ttag=ttag+1;
          topJets.push_back(l);
      }
    }

    int nout = 0;

    for (int i = 0; i < selectedOldIsoLeptons->size(); ++i) {
      bool overlaps = false;
      for (int l = 0; l < nFatJet; ++l) {
        float score = (FatJet_globalParT3_TopbWev[l] + FatJet_globalParT3_TopbWmv[l]) / (FatJet_globalParT3_TopbWev[l] + FatJet_globalParT3_TopbWmv[l] +  FatJet_globalParT3_QCD[l]);
        if (score > WP && deltaR(FatJet_eta[l], FatJet_phi[l], (*selectedOldIsoLeptons)[i]->eta_, (*selectedOldIsoLeptons)[i]->phi_) < 0.8) {
          overlaps = true;
          break;
        }
      }
      if (!overlaps) ++nout;
    }
    int noutIso = 0;
    for (int i = 0; i < selectedIsoLeptons->size(); ++i) {
      bool overlaps = false;
      for (int l = 0; l < nFatJet; ++l) {
        float score = (FatJet_globalParT3_TopbWev[l] + FatJet_globalParT3_TopbWmv[l]) / (FatJet_globalParT3_TopbWev[l] + FatJet_globalParT3_TopbWmv[l] + FatJet_globalParT3_QCD[l]);
        if (score > WP && deltaR(FatJet_eta[l], FatJet_phi[l], (*selectedIsoLeptons)[i]->eta_, (*selectedIsoLeptons)[i]->phi_) < 0.8) {
          overlaps = true;
          break;
        }
    }  
    if (!overlaps) ++noutIso;
    }

    if(ifeve) cutFlow->Fill(0.5);
    if(ifeve && trigger(data, dataset,year)) cutFlow->Fill(1.5);
    if(ifeve && trigger(data, dataset,year) && nLeptons==0) cutFlow->Fill(2.5);
    if(ifeve && trigger(data, dataset,year) && nLeptons==1) cutFlow->Fill(3.5);
    if(ifeve && trigger(data, dataset,year) && nLeptons==2) cutFlow->Fill(4.5);
    if(ifeve && trigger(data, dataset,year) && ifeveIso && nLeptons==0) cutFlow->Fill(5.5);
    if(ifeve && trigger(data, dataset,year) && ifeveIso && nLeptons==1) cutFlow->Fill(6.5);
    if(ifeve && trigger(data, dataset,year) && ifeveIso && nLeptons==2) cutFlow->Fill(7.5);
    if(ifeve && trigger(data, dataset,year) && nLeptons==0 && ttag==0 && noutIso >=2) cutFlow->Fill(8.5);
    if(ifeve && trigger(data, dataset,year) && nLeptons==1 && ttag==0 && noutIso >=2) cutFlow->Fill(9.5);    
    if(ifeve && trigger(data, dataset,year) && nLeptons==2 && ttag==0 && noutIso >=2) cutFlow->Fill(10.5);
    if(ifeve && trigger(data, dataset,year) && nLeptons==0 && ttag==1 && noutIso >=1) cutFlow->Fill(11.5);
    if(ifeve && trigger(data, dataset,year) && nLeptons==1 && ttag==1 && noutIso >=1) cutFlow->Fill(12.5);
    if(ifeve && trigger(data, dataset,year) && nLeptons==2 && ttag==1 && noutIso >=1) cutFlow->Fill(13.5);    
    if(ifeve && trigger(data, dataset,year) && nLeptons==0 && ttag==2) cutFlow->Fill(14.5);
    if(ifeve && trigger(data, dataset,year) && nLeptons==1 && ttag==2) cutFlow->Fill(15.5);
    if(ifeve && trigger(data, dataset,year) && nLeptons==2 && ttag==2) cutFlow->Fill(16.5);
    if(ifeve && trigger(data, dataset,year) && ifeveIso && nLeptons==2) cutFlow->Fill(18.5);
    if((ifeve && trigger(data, dataset,year) && nLeptons==2 && ttag==0 && noutIso >=2) || (ifeve && trigger(data, dataset,year) && nLeptons==2 && ttag==1 && noutIso >=1) || (ifeve && trigger(data, dataset,year) && nLeptons==2 && ttag==2)) cutFlow->Fill(19.5);
//        if(ifeve && trigger(data, dataset,year) && ifeveIso && nLeptons==2 && PFMET_pt>MetCut) cutFlow->Fill(17.5);
      }
    }

//MET noise filters 2024
    if (Flag_goodVertices  &&  Flag_globalSuperTightHalo2016Filter && Flag_EcalDeadCellTriggerPrimitiveFilter && Flag_BadPFMuonFilter && Flag_eeBadScFilter && Flag_ecalBadCalibFilter && Flag_BadPFMuonDzFilter) metFilterPass = true;
//pre selection    
    bool preSelection=true;
    if(!metFilterPass || !trigger(data, dataset,year) || selectedNonIsoLeptons->size() < 2) preSelection=false;
    if(!preSelection){
      objectSelectionEnd();
      continue;
    }

    if(selectedNonIsoLeptons->size() >=2 && ((*selectedNonIsoLeptons)[0]->lep_+ (*selectedNonIsoLeptons)[1]->lep_)==2)
    {
      triggEff_denom_mll->Fill(((*selectedNonIsoLeptons)[0]->p4_ + (*selectedNonIsoLeptons)[1]->p4_).M());
      triggEff_denom_ptl1->Fill((*selectedNonIsoLeptons)[0]->pt_);
    }
    if(selectedNonIsoLeptons->size() >=2 && ((*selectedNonIsoLeptons)[0]->lep_+ (*selectedNonIsoLeptons)[1]->lep_)==2 && HLT_DoubleEle25_CaloIdL_MW){
      triggEff_num_mll->Fill(((*selectedNonIsoLeptons)[0]->p4_ + (*selectedNonIsoLeptons)[1]->p4_).M());
      triggEff_num_ptl1->Fill((*selectedNonIsoLeptons)[0]->pt_);
    }
//##//jet veto map
//##    for (UInt_t l=0;l<selectedJets->size();l++){
//##      jetVetoMap_before->Fill((*selectedJets)[l]->eta_, (*selectedJets)[l]->phi_);
//##      if (!jetVetoMaps_H->GetBinContent(jetVetoMaps_H->GetXaxis()->FindBin((*selectedJets)[l]->eta_),jetVetoMaps_H->GetYaxis()->FindBin((*selectedJets)[l]->phi_))>0) jetVetoMap_after->Fill((*selectedJets)[l]->eta_, (*selectedJets)[l]->phi_);
//##    }
//##    bool jetVetoMap=false;
//##    for (UInt_t l=0;l<selectedJets->size();l++){
//##    if (jetVetoMaps_H->GetBinContent(jetVetoMaps_H->GetXaxis()->FindBin((*selectedJets)[l]->eta_),jetVetoMaps_H->GetYaxis()->FindBin((*selectedJets)[l]->phi_))>0) jetVetoMap=true;
//##      HTjets=HTjets+(*selectedJets)[l]->pt_;
//##      if((*selectedJets)[l]->btag_) nbjetOut++;
//##    }
//##    if(jetVetoMap) continue;
//##
//##// HEM issue veto
//##    isHem=false;
//##    if (year == "2018"){
//##      for (int l=0;l<selectedJets->size();l++){
//##        if((*selectedJets)[l]->eta_ < -1.3 && (*selectedJets)[l]->phi_ < -0.87 && (*selectedJets)[l]->phi_ > -1.57) isHem=true;
//##      }
//##      for (int l=0;l<selectedIsoLeptons->size();l++){
//##        if((*selectedIsoLeptons)[l]->lep_==1 && (*selectedIsoLeptons)[l]->eta_ < -1.3 && (*selectedIsoLeptons)[l]->phi_ < -0.87 && (*selectedIsoLeptons)[l]->phi_ > -1.57) isHem=true;
//##      }
//##    }
//##    if(data == "mc" && isHem){
//##      isHem=false;
//##      HemMcWeight= (6.89+31.74)/59.4;
//##    }
//##    if(data == "data" && isHem){
//##      if( run<319077) isHem=false;
//##    }
//##
//##    if(jetVetoMap || isHem) {
//##      objectSelectionEnd();
//##      continue;
//##    }
//##
    if (data == "mc") weight_Lumi = (1000*xs*lumi)/Nevent;

//Event selection
    resetVecInt(reg);
    resetVecInt(allCh);
    leptonPass = false;
    DyInZPass  = false;
    MetPass    = false;

    const lepton_candidate* lep0 = nullptr;
    const lepton_candidate* lep1 = nullptr;
    
    double bestPtSum = -1.0;
    bool foundZPair = false;
    
    for (size_t i = 0; i < selectedNonIsoLeptons->size(); ++i) {
      for (size_t j = i + 1; j < selectedNonIsoLeptons->size(); ++j) {
    
        const auto& l1 = *(*selectedNonIsoLeptons)[i];
        const auto& l2 = *(*selectedNonIsoLeptons)[j];
    
        double dr   = deltaR(l1.eta_, l1.phi_, l2.eta_, l2.phi_);
        double mass = (l1.p4_ + l2.p4_).M();
    
        if (mass < 20. || dr < 0.8)
          continue;
    
        bool sf = (l1.lep_ == l2.lep_);
        bool os = (l1.charge_ * l2.charge_ < 0);
        bool inZ = (76. < mass && mass < 106.);
    
        if (!(sf && os && inZ))
          continue;
    
        double ptSum = l1.pt_ + l2.pt_;
    
        if (ptSum > bestPtSum) {
          bestPtSum = ptSum;
          lep0 = &l1;
          lep1 = &l2;
          foundZPair = true;
        }
      }
    }

    const auto& l1 = *(*selectedNonIsoLeptons)[0];
    const auto& l2 = *(*selectedNonIsoLeptons)[1];
    double dr   = deltaR(l1.eta_, l1.phi_, l2.eta_, l2.phi_);
    double mass = (l1.p4_ + l2.p4_).M();

    if (!foundZPair && mass > 20. && dr > 0.8) {
      lep0 = selectedNonIsoLeptons->at(0);
      lep1 = selectedNonIsoLeptons->at(1);
    }

    if (!lep0) {
      objectSelectionEnd();
      continue;	    
    }
    drll = deltaR(lep0->eta_, lep0->phi_, lep1->eta_, lep1->phi_);
    mll  = (lep0->p4_ + lep1->p4_).M();
    sumCharge  = lep0->charge_ + lep1->charge_;
    sumFlavour = lep0->lep_    + lep1->lep_;

    const bool ee = (sumFlavour == 2);
    const bool em = (sumFlavour == 11);
    const bool mm = (sumFlavour == 20);
    leptonPass = ee ||(em && ((lep0->pt_ > 60 && lep0->lep_ == 10) || (lep1->pt_ > 60 && lep1->lep_ == 10))) ||(mm && lep0->pt_ > 60);
    const bool inZWindow = (76. < mll && mll < 106.);
    DyInZPass = (ee && inZWindow) || (mm && inZWindow) ||em;

    if (leptonPass) {
        if (ee) {
            const char* ch = DyInZPass ? "2l_eeInZ" : "2l_eeOutZ";
            allCh[getVecPos(channels, ch)] = getVecPos(channels, ch);
        }
        else if (mm) {
            const char* ch = DyInZPass ? "2l_mmInZ" : "2l_mmOutZ";
            allCh[getVecPos(channels, ch)] = getVecPos(channels, ch);
        }
        else if (em) {
            allCh[getVecPos(channels, "2l_em")] = getVecPos(channels, "2l_em");
        }
    }    

//    leptonPass=false;
//    DyInZPass=false;
//    MetPass=false;
//    drll=deltaR((*selectedNonIsoLeptons)[0]->eta_,(*selectedNonIsoLeptons)[0]->phi_,(*selectedNonIsoLeptons)[1]->eta_,(*selectedNonIsoLeptons)[1]->phi_);
//    mll=((*selectedNonIsoLeptons)[0]->p4_ + (*selectedNonIsoLeptons)[1]->p4_).M();
//    for (UInt_t i=0;i<2;i++){
//      sumCharge += (*selectedNonIsoLeptons)[i]->charge_;
//      sumFlavour += (*selectedNonIsoLeptons)[i]->lep_;
//    }
//    if (sumFlavour== 2 && mll>20 && drll>0.8) leptonPass=true;
//    if (sumFlavour== 11 && mll>20 && (((*selectedNonIsoLeptons)[0]->pt_ > 60 && (*selectedNonIsoLeptons)[0]->lep_==10) || ((*selectedNonIsoLeptons)[1]->pt_ > 60 && (*selectedNonIsoLeptons)[1]->lep_==10)) && drll>0.8) leptonPass=true;
//    if (sumFlavour== 20 && mll>20 && (*selectedNonIsoLeptons)[0]->pt_ > 60 && drll>0.8) leptonPass=true;
//    if ((mll<106 && mll>76 && sumFlavour==2) || (mll<106 && mll>76 && sumFlavour==20) || sumFlavour==11) DyInZPass=true;
//
//    if(sumFlavour== 2 && leptonPass && DyInZPass) allCh[getVecPos(channels,"2l_eeInZ")]=getVecPos(channels,"2l_eeInZ");
//    if(sumFlavour== 20 && leptonPass && DyInZPass) allCh[getVecPos(channels,"2l_mmInZ")]=getVecPos(channels,"2l_mmInZ");
//    if(sumFlavour== 2 && leptonPass && !DyInZPass) allCh[getVecPos(channels,"2l_eeOutZ")]=getVecPos(channels,"2l_eeOutZ");
//    if(sumFlavour== 20 && leptonPass && !DyInZPass) allCh[getVecPos(channels,"2l_mmOutZ")]=getVecPos(channels,"2l_mmOutZ");
//    if(sumFlavour== 11 && leptonPass) allCh[getVecPos(channels,"2l_em")]=getVecPos(channels,"2l_em");	      

//count number of b-tagged jets and leptons outside the top tagged jets    
  float tagProb = -1.f;
  for (size_t j = 0; j < selectedJets08->size(); ++j) {
      const auto& jet = *(*selectedJets08)[j];
      if (jet.toptag_in_ > tagProb) {
          tagProb = jet.toptag_in_;
          tTagIndex = j;
      }
  }

  for(int i=0; i<selectedJets->size(); i++){
      if(!(*selectedJets)[i]->btag_) continue;
      nbjet++;
      bool insideTopJet = false;
      for(int j=0; j<selectedJets08->size(); j++){
          if(!(*selectedJets08)[j]->toptag_) continue;
          if(deltaR((*selectedJets)[i]->eta_, (*selectedJets)[i]->phi_, (*selectedJets08)[j]->eta_, (*selectedJets08)[j]->phi_) < 0.8){
              insideTopJet = true;
              break;
          }
      }
      if(!insideTopJet) nbjetOut++;
      else nbjetIn++;
  }

  for(int i=0; i<selectedNonIsoLeptons->size(); i++){
      bool isIso = (((*selectedNonIsoLeptons)[i]->lep_==1 &&  Electron_miniPFRelIso_all[(*selectedNonIsoLeptons)[i]->indice_] < 0.2)
       || ((*selectedNonIsoLeptons)[i]->lep_==10 && Muon_miniPFRelIso_all[(*selectedNonIsoLeptons)[i]->indice_] < 0.2)
      );
      if(!isIso) continue;
      bool insideTopJet = false;
      for(int j=0; j<selectedJets08->size(); j++){
          if(!(*selectedJets08)[j]->toptag_) continue;
          if(deltaR((*selectedNonIsoLeptons)[i]->eta_, (*selectedNonIsoLeptons)[i]->phi_, (*selectedJets08)[j]->eta_, (*selectedJets08)[j]->phi_) < 0.8){
              insideTopJet = true;
              break;  
          }
      }
      if(!insideTopJet) nIsolOut++;
  }

  for(int i=0; i<selectedNonIsoLeptons->size(); i++){
      bool insideTopJet = false;
      for(int j=0; j<selectedJets08->size(); j++){
          if(!(*selectedJets08)[j]->toptag_) continue;
          if(deltaR((*selectedNonIsoLeptons)[i]->eta_, (*selectedNonIsoLeptons)[i]->phi_, (*selectedJets08)[j]->eta_, (*selectedJets08)[j]->phi_) < 0.8){
              insideTopJet = true;
              break;
          }
      }
      if(insideTopJet) nNonIsolIn++;
  }

  for(int i=0;i<selectedJets08->size();i++){
    if((*selectedJets08)[i]->toptag_) {
      ntjet++;
      if(deltaR(lep0->eta_,lep0->phi_, (*selectedJets08)[i]->eta_, (*selectedJets08)[i]->phi_) < 0.8) lep0IntJet = true;
      if(deltaR(lep1->eta_,lep1->phi_, (*selectedJets08)[i]->eta_, (*selectedJets08)[i]->phi_) < 0.8) lep1IntJet = true;
    }  
  }  

  const jet_candidate* tjet = nullptr;
  if(selectedJets08->size()>0){
    tjet = (*selectedJets08)[tTagIndex];
    if(SubJet_btagUParTAK4B[FatJet_subJetIdx1[tjet->indice_]] > 0.1272) nbSubjet++;
    if(SubJet_btagUParTAK4B[FatJet_subJetIdx2[tjet->indice_]] > 0.1272) nbSubjet++;
  }

  const bool lep0Iso =    (!lep0IntJet) &&    ((lep0->lep_ == 1  && Electron_miniPFRelIso_all[lep0->indice_] < 0.2) ||     (lep0->lep_ == 10 && Muon_tkIsoId[lep0->indice_] >= 1));
  const bool lep1Iso =    (!lep1IntJet) &&    ((lep1->lep_ == 1  && Electron_miniPFRelIso_all[lep1->indice_] < 0.2) ||     (lep1->lep_ == 10 && Muon_tkIsoId[lep1->indice_] >= 1));
  bool topMass=false;
  if(selectedJets08->size()>0 && (*selectedJets08)[tTagIndex]->mass_>50) topMass=true;

  reg[getVecPos(regions,"All")]=getVecPos(regions,"All");
  if( ntjet==0 && nbjetOut==2 && lep0Iso && lep1Iso) reg[getVecPos(regions,"2b0t")]=getVecPos(regions,"2b0t"); 
  if( ntjet==0 && nbjetOut==1 && lep0Iso && lep1Iso) reg[getVecPos(regions,"1b0t")]=getVecPos(regions,"1b0t");  
  if( ntjet==0 && nbjetOut==0 && lep0Iso && lep1Iso) reg[getVecPos(regions,"0b0t")]=getVecPos(regions,"0b0t");  
  if (ntjet == 1 && nbjetOut == 1 && (lep0Iso || lep1Iso))   reg[getVecPos(regions, "1b1t")] = getVecPos(regions, "1b1t");
  if (ntjet == 1 && nbjetOut == 0 && (lep0Iso || lep1Iso))   reg[getVecPos(regions, "0b1t")] = getVecPos(regions, "0b1t");
  if( ntjet==2) reg[getVecPos(regions,"2t")]=getVecPos(regions,"2t");  
  if (ntjet == 1 && nbjetOut == 1 && (lep0Iso || lep1Iso) && topMass)   reg[getVecPos(regions, "1b1tM50")] = getVecPos(regions, "1b1tM50");
  if( ntjet==2 && topMass) reg[getVecPos(regions,"2tM50")]=getVecPos(regions,"2tM50");  
 
  //if(reg[getVecPos(regions, "0b1t")] >= 0 


    if(ntjet==0 && nbjetOut==2 && nIsolOut==2) RECOmass0->Fill(recoTT(recoTop1,recoTop2));
    if( ntjet==1 && nbjetOut==1 && nIsolOut==1) RECOmass1->Fill(recoTT(recoTop1,recoTop2));
    if( ntjet==2) RECOmass2->Fill(recoTT(recoTop1,recoTop2));

    if(ntjet==2) cutFlowFinal->Fill(0.5);
    if(ntjet==2 && nNonIsolIn>=2) cutFlowFinal->Fill(1.5);
    if(ntjet==2 && nNonIsolIn>=2 && nbjetIn>=2) cutFlowFinal->Fill(2.5);

    if(ntjet==2 && nLeptons==2) cutFlowFinal->Fill(3.5);
    if(ntjet==2 && nNonIsolIn>=2 && nLeptons==2) cutFlowFinal->Fill(4.5);
    if(ntjet==2 && nNonIsolIn>=2 && nbjetIn>=2 && nLeptons==2) cutFlowFinal->Fill(5.5);

    if (data == "mc"){ 
      weight_lep  = weight_Lumi * signnum_typical(LHEWeight_originalXWGTUP)*nominalWeights[getVecPos(sys,"pu")]*nominalWeights[getVecPos(sys,"eleRecoId")]*nominalWeights[getVecPos(sys,"muRecoId")];
    }  
//##     weight_lepB = weight_lep* nominalWeights[getVecPos(sys,"bcTagSfUnCorr")];
//##     weight_EFT = weight_Lumi* signnum_typical(LHEWeight_originalXWGTUP)*(1.0/LHEWeight_originalXWGTUP)*nominalWeights[getVecPos(sys,"eleRecoIdIso")]*nominalWeights[getVecPos(sys,"muRecoIdIso")]*nominalWeights[getVecPos(sys,"triggerSF")]*nominalWeights[getVecPos(sys,"pu")]*nominalWeights[getVecPos(sys,"prefiring")]*nominalWeights[getVecPos(sys,"bcTagSfUnCorr")]*nominalWeights[getVecPos(sys,"JetPuID")]*HemMcWeight;
//##     if(abs(weight_lepB)>100){
//##       cout<<"Warning: event weight too large ="<<weight_lepB<<endl;
//##       continue;
//##     }
//##     if(fname.Contains("_WZTo3LNu")) weight_lepB = weight_lepB * DibosonCorr[selectedJets->size()];
//##//cout<< weight_Lumi<<":"<<signnum_typical(LHEWeight_originalXWGTUP)<<":"<<(1.0/LHEWeight_originalXWGTUP)<<":"<<nominalWeights[getVecPos(sys,"eleRecoIdIso")]<<":"<<nominalWeights[getVecPos(sys,"muRecoIdIso")]<<":"<<nominalWeights[getVecPos(sys,"triggerSF")]<<":"<<nominalWeights[getVecPos(sys,"pu")]<<":"<<nominalWeights[getVecPos(sys,"prefiring")]<<":"<<nominalWeights[getVecPos(sys,"bcTagSfUnCorr")]<<":"<<nominalWeights[getVecPos(sys,"JetPuID")]<<endl;
//##//  for (int n=0;n<sys.size();++n){
//##//    if(std::isnan(nominalWeights[n]) || !std::isfinite(nominalWeights[n])) cout<<sys[n]<<" is nan/inf "<<nominalWeights[n]<<endl;
//##//    if(std::isnan(nominalWeights[n]) || !std::isfinite(nominalWeights[n])) std::cerr<<sys[n]<<" is nan/inf "<<nominalWeights[n]<<std::endl;
//##//    if(std::isnan(nominalWeights[n]) || !std::isfinite(nominalWeights[n])) std::cout <<sys[n]<<" is nan/inf "<<nominalWeights[n]<<std::flush;
//##//  }
//##
//##   }
//##   for (UInt_t i=0;i<selectedOldIsoLeptons->size();i++){
//##     if ((*selectedOldIsoLeptons)[i]->lep_ == 1) fi = scale_factor(fr_ele_H, conept_TTH((*selectedOldIsoLeptons)[i]->indice_,11), abs((*selectedOldIsoLeptons)[i]->eta_),"central", true, false);
//##     else fi =scale_factor(fr_mu_H, conept_TTH((*selectedOldIsoLeptons)[i]->indice_,13), abs((*selectedOldIsoLeptons)[i]->eta_),"central", true, false);
//##     fakeRate=fakeRate*(fi/(1-fi));
//##     if ((*selectedOldIsoLeptons)[i]->lep_ == 1) fi = scale_factor(fr_ele_H_up, conept_TTH((*selectedOldIsoLeptons)[i]->indice_,11), abs((*selectedOldIsoLeptons)[i]->eta_),"central", true, false);
//##     else fi =scale_factor(fr_mu_H_up, conept_TTH((*selectedOldIsoLeptons)[i]->indice_,13), abs((*selectedOldIsoLeptons)[i]->eta_),"central", true, false);
//##     sysUpWeightsFA[0]=sysUpWeightsFA[0]*(fi/(1-fi));
//##     if ((*selectedOldIsoLeptons)[i]->lep_ == 1) fi = scale_factor(fr_ele_H_down, conept_TTH((*selectedOldIsoLeptons)[i]->indice_,11), abs((*selectedOldIsoLeptons)[i]->eta_),"central", true, false);
//##     else fi =scale_factor(fr_mu_H_down, conept_TTH((*selectedOldIsoLeptons)[i]->indice_,13), abs((*selectedOldIsoLeptons)[i]->eta_),"central", true, false);
//##     sysDownWeightsFA[0]=sysDownWeightsFA[0]*(fi/(1-fi));
//##     if ((*selectedOldIsoLeptons)[i]->lep_ == 1) fi = scale_factor(fr_ele_H_ptUp, conept_TTH((*selectedOldIsoLeptons)[i]->indice_,11), abs((*selectedOldIsoLeptons)[i]->eta_),"central", true, false);
//##     else fi =scale_factor(fr_mu_H_ptUp, conept_TTH((*selectedOldIsoLeptons)[i]->indice_,13), abs((*selectedOldIsoLeptons)[i]->eta_),"central", true, false);
//##     sysUpWeightsFA[1]=sysUpWeightsFA[1]*(fi/(1-fi));
//##     if ((*selectedOldIsoLeptons)[i]->lep_ == 1) fi = scale_factor(fr_ele_H_ptDown, conept_TTH((*selectedOldIsoLeptons)[i]->indice_,11), abs((*selectedOldIsoLeptons)[i]->eta_),"central", true, false);
//##     else fi =scale_factor(fr_mu_H_ptDown, conept_TTH((*selectedOldIsoLeptons)[i]->indice_,13), abs((*selectedOldIsoLeptons)[i]->eta_),"central", true, false);
//##     sysDownWeightsFA[1]=sysDownWeightsFA[1]*(fi/(1-fi));
//##     if ((*selectedOldIsoLeptons)[i]->lep_ == 1) fi = scale_factor(fr_ele_H_etaUp, conept_TTH((*selectedOldIsoLeptons)[i]->indice_,11), abs((*selectedOldIsoLeptons)[i]->eta_),"central", true, false);
//##     else fi =scale_factor(fr_mu_H_etaUp, conept_TTH((*selectedOldIsoLeptons)[i]->indice_,13), abs((*selectedOldIsoLeptons)[i]->eta_),"central", true, false);
//##     sysUpWeightsFA[2]=sysUpWeightsFA[2]*(fi/(1-fi));
//##     if ((*selectedOldIsoLeptons)[i]->lep_ == 1) fi = scale_factor(fr_ele_H_etaDown, conept_TTH((*selectedOldIsoLeptons)[i]->indice_,11), abs((*selectedOldIsoLeptons)[i]->eta_),"central", true, false);
//##     else fi =scale_factor(fr_mu_H_etaDown, conept_TTH((*selectedOldIsoLeptons)[i]->indice_,13), abs((*selectedOldIsoLeptons)[i]->eta_),"central", true, false);
//##     sysDownWeightsFA[2]=sysDownWeightsFA[2]*(fi/(1-fi));
//##   }
   if (iseft) eft_fit = new WCFit(nWCnames, wc_names_lst, nEFTfitCoefficients, EFTfitCoefficients, weight_EFT);
   else eft_fit = new WCFit(0,wc_names_lst,1, &genWeight, 1.0);
//##   resetVecInt(allChFA);
//   if(chNonIso<30) allCh[0]=chNonIso;
//##   if(chIso<30) allChFA[0]=chIso;
//##   if(chOSppWeighted<30) allCh[1]=chOSppWeighted;
//##   myreg = findRegion(selectedJets, ch, chIso);
  for(int i=0;i<channels.size();i++){
    for(int j=0;j<regions.size();j++){   
      wgt[i][j]=weight_lep;
      wcfit[i][j]= *eft_fit;
   }
  }
  if(lep0->pt_<15) cout<<"***********************************************!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! "<<lep0->pt_<<endl;
//##   if(ch<30 && channels[ch].Contains("2lss") && myreg==-1) mll_SS_Zwindow_0jet->Fill( ((*selectedIsoLeptons)[0]->p4_ + (*selectedIsoLeptons)[1]->p4_).M(),weight_lepB);
//##     evaluateMVA(selectedJets, selectedIsoLeptons, Z_P, channels[ch], PFMET_pt, PFMET_phi, MVAS_TU, MVAB_TU, MVAS_TC, MVAB_TC);
     lep0Pt->Fill(lep0->pt_);
     FillD3Hists(Hists, allCh, reg, vInd(vars,"lep1Pt"), lep0->pt_ ,wgt, wcfit);
     FillD3Hists(Hists, allCh, reg, vInd(vars,"lep1Eta"), lep0->eta_ ,wgt, wcfit); 
     FillD3Hists(Hists, allCh, reg, vInd(vars,"lep1Phi"), lep0->phi_ ,wgt, wcfit); 
     FillD3Hists(Hists, allCh, reg, vInd(vars,"lep2Pt"), lep1->pt_ ,wgt, wcfit); 
     FillD3Hists(Hists, allCh, reg, vInd(vars,"lep2Eta"), lep1->eta_ ,wgt, wcfit); 
     FillD3Hists(Hists, allCh, reg, vInd(vars,"lep2Phi"), lep1->phi_ ,wgt, wcfit); 
     FillD3Hists(Hists, allCh, reg, vInd(vars,"llM"), (lep0->p4_ + lep1->p4_).M() ,wgt, wcfit); 
     FillD3Hists(Hists, allCh, reg, vInd(vars,"llPt"), (lep0->p4_ + lep1->p4_).Pt() ,wgt, wcfit); 
     FillD3Hists(Hists, allCh, reg, vInd(vars,"llDr"), deltaR(lep0->eta_,lep0->phi_,lep1->eta_,lep1->phi_) ,wgt, wcfit); 
     FillD3Hists(Hists, allCh, reg, vInd(vars,"llDphi"), abs(deltaPhi(lep0->phi_,lep1->phi_)) ,wgt, wcfit); 
     FillD3Hists(Hists, allCh, reg, vInd(vars,"njet"), selectedJets->size() ,wgt, wcfit); 
     FillD3Hists(Hists, allCh, reg, vInd(vars,"njet08"), selectedJets08->size() ,wgt, wcfit);
     FillD3Hists(Hists, allCh, reg, vInd(vars,"nbjet"), nbjet ,wgt, wcfit);
     FillD3Hists(Hists, allCh, reg, vInd(vars,"ntTagjet08"), ntjet ,wgt, wcfit);
     FillD3Hists(Hists, allCh, reg, vInd(vars,"Met"), PFMET_pt ,wgt, wcfit);
     FillD3Hists(Hists, allCh, reg, vInd(vars,"MetPhi"), PFMET_phi ,wgt, wcfit);
     FillD3Hists(Hists, allCh, reg, vInd(vars,"nVtx"), PV_npvs ,wgt, wcfit);
     FillD3Hists(Hists, allCh, reg, vInd(vars,"llMZw"), (lep0->p4_ + lep1->p4_).M() ,wgt, wcfit);
     FillD3Hists(Hists, allCh, reg, vInd(vars,"ttM"), recoTT(recoTop1,recoTop2) ,wgt, wcfit);
     if(selectedJets->size()>0){
       FillD3Hists(Hists, allCh, reg, vInd(vars,"jet1Pt"), (*selectedJets)[0]->pt_ ,wgt, wcfit);
       FillD3Hists(Hists, allCh, reg, vInd(vars,"jet1Eta"), (*selectedJets)[0]->eta_ ,wgt, wcfit);
       FillD3Hists(Hists, allCh, reg, vInd(vars,"jet1Phi"), (*selectedJets)[0]->phi_ ,wgt, wcfit);
     }
     if(selectedJets08->size()>0){
       FillD3Hists(Hists, allCh, reg, vInd(vars,"t1prob"), (*selectedJets08)[tTagIndex]->toptag_in_  ,wgt, wcfit);
       FillD3Hists(Hists, allCh, reg, vInd(vars,"t1mass"), (*selectedJets08)[tTagIndex]->mass_ ,wgt, wcfit);
       FillD3Hists(Hists, allCh, reg, vInd(vars,"t1chEmEF"), FatJet_chEmEF[(*selectedJets08)[tTagIndex]->indice_] + FatJet_muEF[(*selectedJets08)[tTagIndex]->indice_], wgt, wcfit);
       FillD3Hists(Hists, allCh, reg, vInd(vars,"t1tau21"), FatJet_tau2[(*selectedJets08)[tTagIndex]->indice_]/FatJet_tau1[(*selectedJets08)[tTagIndex]->indice_], wgt, wcfit);
       FillD3Hists(Hists, allCh, reg, vInd(vars,"t1nbSubjet"),nbSubjet, wgt, wcfit);
//       if(FatJet_genJetAK8Idx[(*selectedJets08)[tTagIndex]->indice_] >=0 && (*selectedJets08)[tTagIndex]->toptag_in_>0.95) cout<<static_cast<int>(GenJetAK8_hadronFlavour[FatJet_genJetAK8Idx[(*selectedJets08)[tTagIndex]->indice_]])<<endl;
     }  
//##  if(chIso<30){ 
//##     evaluateMVA(selectedJets, selectedNonIsoLeptons, Z_FP, channelsFA[chIso], PFMET_pt, PFMET_phi, MVAS_TUFA, MVAB_TUFA, MVAS_TCFA, MVAB_TCFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"lep1Pt"), (*selectedNonIsoLeptons)[0]->pt_ ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"lep1Eta"), (*selectedNonIsoLeptons)[0]->eta_ ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"lep1Phi"), (*selectedNonIsoLeptons)[0]->phi_ ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"lep2Pt"), (*selectedNonIsoLeptons)[1]->pt_ ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"lep2Eta"), (*selectedNonIsoLeptons)[1]->eta_ ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"lep2Phi"), (*selectedNonIsoLeptons)[1]->phi_ ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"llM"), ((*selectedNonIsoLeptons)[0]->p4_ + (*selectedNonIsoLeptons)[1]->p4_).M() ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"llPt"), ((*selectedNonIsoLeptons)[0]->p4_ + (*selectedNonIsoLeptons)[1]->p4_).Pt() ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"llDr"), deltaR((*selectedNonIsoLeptons)[0]->eta_,(*selectedNonIsoLeptons)[0]->phi_,(*selectedNonIsoLeptons)[1]->eta_,(*selectedNonIsoLeptons)[1]->phi_) ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"llDphi"), abs(deltaPhi((*selectedNonIsoLeptons)[0]->phi_,(*selectedNonIsoLeptons)[1]->phi_)) ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"njet"), selectedJets->size() ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"nbjetOut"), nbjetOut ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"nNonBjet"), countNonBjets(selectedJets) ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"Met"), PFMET_pt ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"MetPhi"), PFMET_phi ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"nVtx"), PV_npvs ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"llMZw"), ((*selectedNonIsoLeptons)[0]->p4_ + (*selectedNonIsoLeptons)[1]->p4_).M() ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"MVATU"), MVAS_TUFA/MVAB_TUFA ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"MVATC"), MVAS_TCFA/MVAB_TCFA ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"lep3Pt"), MVA_lep3Pt,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"lep3Eta"), MVA_lep3Eta ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"bJetPt"), MVA_bJetPt ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"bJetEta"), MVA_bJetEta ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"tH_topMass"), MVA_tH_topMass ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"tH_HMass"), MVA_tH_HMass ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"tH_WtopMass"), MVA_tH_WtopMass ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"tH_W1HMass"), MVA_tH_W1HMass ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"tH_W2HMass"), MVA_tH_W2HMass ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"tH_HPt"), MVA_tH_HPt,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"tH_HEta"), MVA_tH_HEta,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"tH_topPt"),MVA_tH_topPt ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"tH_topEta"),MVA_tH_topEta ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"tH_drWtopB"), MVA_tH_drWtopB ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"tH_drW1HW2H"), MVA_tH_drW1HW2H ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"tZ_topMass"), MVA_tZ_topMass ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"tZ_ZMass"),MVA_tZ_ZMass ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"tZ_WtopMass"),MVA_tZ_WtopMass ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"tZ_ZPt"),MVA_tZ_ZPt ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"tZ_ZEta"),MVA_tZ_ZEta ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"tZ_topPt"), MVA_tZ_topPt ,wgtFA, wcfitFA);
//##     FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"tZ_topEta"), MVA_tZ_topEta,wgtFA, wcfitFA);
//##     if(selectedJets->size()>0){
//##       FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"jet1Pt"), (*selectedJets)[0]->pt_ ,wgtFA, wcfitFA);
//##       FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"jet1Eta"), (*selectedJets)[0]->eta_ ,wgtFA, wcfitFA);
//##       FillD3Hists(HistsFA, allChFA, reg, vInd(varsFA,"jet1Phi"), (*selectedJets)[0]->phi_ ,wgtFA, wcfitFA);
//##     }
//##     //Fake sys 
//##     if (myreg>=0){
//##       for(int n=0;n<sysFA.size();++n){
//##         wgtSysUp[0][myreg]=wgtFA[0][myreg]*(sysUpWeightsFA[n]/fakeRate);
//##         wcfitSysUp[0][myreg]= *eft_fit;
//##         FillD4Hists(HistsFAUp, allChFA, reg, vInd(varsFullSys,"MVATU"), MVAS_TUFA/MVAB_TUFA ,wgtSysUp, wcfitSysUp,n);
//##         FillD4Hists(HistsFAUp, allChFA, reg, vInd(varsFullSys,"MVATC"), MVAS_TCFA/MVAB_TCFA ,wgtSysUp, wcfitSysUp,n);
//##
//##         wgtSysUp[0][myreg]=wgtFA[0][myreg]*(sysDownWeightsFA[n]/fakeRate);
//##         FillD4Hists(HistsFADown, allChFA, reg, vInd(varsFullSys,"MVATU"), MVAS_TUFA/MVAB_TUFA ,wgtSysUp, wcfitSysUp,n);
//##         FillD4Hists(HistsFADown, allChFA, reg, vInd(varsFullSys,"MVATC"), MVAS_TCFA/MVAB_TCFA ,wgtSysUp, wcfitSysUp,n);
//##       }
//##     }
//##   }
//##   delete eft_fit;
//##//Fill syst histsi 
//##   if (data == "mc"  && ifSys && ch<30 && std::find(channelsSys.begin(), channelsSys.end(), channels[ch]) != channelsSys.end() && myreg>=0){
//##     for (int i = 0, idx = 0; i <= Dim; ++i) {
//##         for (int j = 0; j <= i; ++j, ++idx) {
//##             sysFitCoefficientsUp[idx] = 0.0f; 
//##             sysFitCoefficientsDown[idx] = 0.0f;
//##         }
//##     }
//##     for (int i = 0, idx = 0; i <= DimTh; ++i) {
//##         for (int j = 0; j <= i; ++j, ++idx) {
//##             sysThFitCoefficients[idx] = 0.0f;
//##         }
//##     }
//##
//##     auto it = std::find(channelsSys.begin(), channelsSys.end(), channels[ch]);
//##     size_t index = std::distance(channelsSys.begin(), it);
//##     allCh[0]=index; allCh[1]=-1;  
//##     for(int n=0;n<sys.size();++n){
//##       if (std::find(sysNotWeight.begin(), sysNotWeight.end(), sys[n]) != sysNotWeight.end()) continue;
//##       if (iseft) eft_fit = new WCFit(nWCnames, wc_names_lst, nEFTfitCoefficients, EFTfitCoefficients, weight_EFT*(sysUpWeights[n]/nominalWeights[n]));
//##       else eft_fit = new WCFit(0,wc_names_lst,1, &genWeight, 1.0);
//##       wgtSysUp[0][myreg]=wgt[0][myreg]*(sysUpWeights[n]/nominalWeights[n]);
//##       wcfitSysUp[0][myreg]= *eft_fit;
//##       FillD4Hists(HistsSysUp, allCh, reg, vInd(varsFullSys,"MVATU"), MVAS_TU/MVAB_TU ,wgtSysUp, wcfitSysUp, n);
//##       FillD4Hists(HistsSysUp, allCh, reg, vInd(varsFullSys,"MVATC"), MVAS_TC/MVAB_TC ,wgtSysUp, wcfitSysUp, n);
//##       sysFitCoefficientsUp[n]=wgt[0][myreg]*(sysUpWeights[n]/nominalWeights[n]);
//##
//##       delete eft_fit;
//##       if (iseft) eft_fit = new WCFit(nWCnames, wc_names_lst, nEFTfitCoefficients, EFTfitCoefficients, weight_EFT*(sysDownWeights[n]/nominalWeights[n]));
//##       else eft_fit = new WCFit(0,wc_names_lst,1, &genWeight, 1.0);
//##       wgtSysDown[0][myreg]=wgt[0][myreg]*(sysDownWeights[n]/nominalWeights[n]);
//##       wcfitSysDown[0][myreg]= *eft_fit;
//##       FillD4Hists(HistsSysDown, allCh, reg, vInd(varsFullSys,"MVATU"), MVAS_TU/MVAB_TU ,wgtSysDown, wcfitSysDown, n);
//##       FillD4Hists(HistsSysDown, allCh, reg, vInd(varsFullSys,"MVATC"), MVAS_TC/MVAB_TC ,wgtSysDown, wcfitSysDown, n);
//##       delete eft_fit;
//##       sysFitCoefficientsDown[n]=wgt[0][myreg]*(sysDownWeights[n]/nominalWeights[n]);
//##     }
//##//Theory uncertainty {RenUp, RenDown, FacUp, FacDown, IsrUp, IsrDown, FsrUp, FsrDown, PDF1, PDF2,..., PDF100}
//##//Qscale uncertainty
//##     if (iseft){
//##       sysThFitCoefficients[0]=wgt[0][myreg]*(LHEScaleWeight[6])*csetScale->evaluate({std::string(fname.Data()), 6});
//##       sysThFitCoefficients[1]=wgt[0][myreg]*(LHEScaleWeight[1])*csetScale->evaluate({std::string(fname.Data()), 1});
//##       sysThFitCoefficients[2]=wgt[0][myreg]*(LHEScaleWeight[4])*csetScale->evaluate({std::string(fname.Data()), 4});
//##       sysThFitCoefficients[3]=wgt[0][myreg]*(LHEScaleWeight[3])*csetScale->evaluate({std::string(fname.Data()), 3});
//##     }
//##     else{
//##       sysThFitCoefficients[0]=wgt[0][myreg]*(LHEScaleWeight[7])*csetScale->evaluate({std::string(fname.Data()), 7});
//##       sysThFitCoefficients[1]=wgt[0][myreg]*(LHEScaleWeight[1])*csetScale->evaluate({std::string(fname.Data()), 1});
//##       sysThFitCoefficients[2]=wgt[0][myreg]*(LHEScaleWeight[5])*csetScale->evaluate({std::string(fname.Data()), 5});
//##       sysThFitCoefficients[3]=wgt[0][myreg]*(LHEScaleWeight[3])*csetScale->evaluate({std::string(fname.Data()), 3});
//##     }       
//##//ISR/FSR uncertainty
//##     sysThFitCoefficients[4]=wgt[0][myreg]*(PSWeight[0]);
//##     sysThFitCoefficients[5]=wgt[0][myreg]*(PSWeight[2]);
//##     sysThFitCoefficients[6]=wgt[0][myreg]*(PSWeight[1]);
//##     sysThFitCoefficients[7]=wgt[0][myreg]*(PSWeight[3]);
//##//PDF uncertainty
//##     for (int n=0;n<100;++n){
//##       if(isnan(LHEPdfWeight[n]) || isinf(LHEPdfWeight[n])) continue;
//##       sysThFitCoefficients[n+8]= wgt[0][myreg]*LHEPdfWeight[n]*csetPDF->evaluate({std::string(fname.Data()), n});
//##     }
//##     eft_fit = new WCFit(DimTh, sysTh_std, NTh, sysThFitCoefficients, 1.0f);
//##//cout<<eft_fit->evalPoint("S1",1)<<" "<<eft_fit->evalPoint("S2",1)<<endl;
//##     if (myreg>=0){
//##       wcfit[0][myreg]= *eft_fit;
//##     }
//##     FillD3Hists(HistsTh, allCh, reg, vInd(varsFullSys,"MVATU"), MVAS_TU/MVAB_TU ,wgt, wcfit);
//##     FillD3Hists(HistsTh, allCh, reg, vInd(varsFullSys,"MVATC"), MVAS_TC/MVAB_TC ,wgt, wcfit);
//##     delete eft_fit;
//##//Fill uncluster MET
//##     if (iseft) eft_fit = new WCFit(nWCnames, wc_names_lst, nEFTfitCoefficients, EFTfitCoefficients, weight_EFT);
//##     else eft_fit = new WCFit(0,wc_names_lst,1, &genWeight, 1.0);
//##      auto itt = std::find(sys.begin(), sys.end(), "UnclustEn");
//##      int itum = std::distance(sys.begin(), itt);
//##      resetVecInt(reg);
//##      myreg = findRegion(selectedJets,  ch, chIso);
//##      if (myreg>=0){
//##        reg[myreg]=myreg;
//##        wgt[0][myreg]=weight_lepB;
//##        wcfit[0][myreg]= *eft_fit;
//##      }
//##//      metUnclusMETUp = sqrt(pow(PFMET_pt * cos(PFMET_phi) + PFMET_MetUnclustEnUpDeltaX,2)+pow(PFMET_pt * sin(PFMET_phi) + PFMET_MetUnclustEnUpDeltaY,2));
//##//      metUnclusMETDown = sqrt(pow(PFMET_pt * cos(PFMET_phi) - PFMET_MetUnclustEnUpDeltaX,2)+pow(PFMET_pt * sin(PFMET_phi) - PFMET_MetUnclustEnUpDeltaY,2));
//##//      metUnclusMETPhiUp = atan2 ((PFMET_pt * sin(PFMET_phi) + PFMET_MetUnclustEnUpDeltaY),(PFMET_pt * cos(PFMET_phi) + PFMET_MetUnclustEnUpDeltaX));
//##//      metUnclusMETPhiDown = atan2 ((PFMET_pt * sin(PFMET_phi) - PFMET_MetUnclustEnUpDeltaY),(PFMET_pt * cos(PFMET_phi) - PFMET_MetUnclustEnUpDeltaX));
//##      evaluateMVA(selectedJets, selectedIsoLeptons, Z_P, channels[ch],metUnclusMETUp , metUnclusMETPhiUp,  MVAS_TU, MVAB_TU, MVAS_TC, MVAB_TC);
//##      FillD4Hists(HistsSysUp, allCh, reg, vInd(varsFullSys,"MVATU"), MVAS_TU/MVAB_TU ,wgt, wcfit,itum);
//##      FillD4Hists(HistsSysUp, allCh, reg, vInd(varsFullSys,"MVATC"), MVAS_TC/MVAB_TC ,wgt, wcfit,itum);
//##      evaluateMVA(selectedJets, selectedIsoLeptons, Z_P, channels[ch],metUnclusMETDown , metUnclusMETPhiDown,  MVAS_TU, MVAB_TU, MVAS_TC, MVAB_TC);
//##      FillD4Hists(HistsSysDown, allCh, reg, vInd(varsFullSys,"MVATU"), MVAS_TU/MVAB_TU ,wgt, wcfit,itum);
//##      FillD4Hists(HistsSysDown, allCh, reg, vInd(varsFullSys,"MVATC"), MVAS_TC/MVAB_TC ,wgt, wcfit,itum);
//##
//##
//##//Fill JES(6)+Jer(1) histograms
//##     for (int n=0;n<nsrc+1;++n){
//##      auto itt = std::find(sys.begin(), sys.end(), sysJec[n]);
//##      int it = std::distance(sys.begin(), itt);
//##      auto normalItt = std::find(sysNormal.begin(), sysNormal.end(), sysJec[n]);  // looks for "JER/JES"
//##
//##      resetVecInt(reg);
//##      myreg = findRegion(&(*JECsysUp)[n],  ch, chIso);
//##      if (myreg>=0){
//##        reg[myreg]=myreg;
//##        wgt[0][myreg]=weight_lepB;
//##        wcfit[0][myreg]= *eft_fit;
//##      }
//##       evaluateMVA(&(*JECsysUp)[n], selectedIsoLeptons, Z_P, channels[ch], (*MetJECsysUp)[n], (*MetPhiJECsysUp)[n],  MVAS_TU, MVAB_TU, MVAS_TC, MVAB_TC);
//##       FillD4Hists(HistsSysUp, allCh, reg, vInd(varsFullSys,"MVATU"), MVAS_TU/MVAB_TU ,wgt, wcfit,it);
//##       FillD4Hists(HistsSysUp, allCh, reg, vInd(varsFullSys,"MVATC"), MVAS_TC/MVAB_TC ,wgt, wcfit,it);
//##       if(normalItt!= sysNormal.end()) {
//##         int nIt = std::distance(sysNormal.begin(), normalItt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"lep1Pt"), (*selectedIsoLeptons)[0]->pt_ ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"lep1Eta"), (*selectedIsoLeptons)[0]->eta_ ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"lep1Phi"), (*selectedIsoLeptons)[0]->phi_ ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"lep2Pt"), (*selectedIsoLeptons)[1]->pt_ ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"lep2Eta"), (*selectedIsoLeptons)[1]->eta_ ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"lep2Phi"), (*selectedIsoLeptons)[1]->phi_ ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"llM"), ((*selectedIsoLeptons)[0]->p4_ + (*selectedIsoLeptons)[1]->p4_).M() ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"llPt"), ((*selectedIsoLeptons)[0]->p4_ + (*selectedIsoLeptons)[1]->p4_).Pt() ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"llDr"), deltaR((*selectedIsoLeptons)[0]->eta_,(*selectedIsoLeptons)[0]->phi_,(*selectedIsoLeptons)[1]->eta_,(*selectedIsoLeptons)[1]->phi_) ,wgt,nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"llDphi"), abs(deltaPhi((*selectedIsoLeptons)[0]->phi_,(*selectedIsoLeptons)[1]->phi_)) ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"njet"), (*JECsysUp)[n].size() ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"nbjetOut"), NbTag ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"nNonBjet"), NnonbTag ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"Met"), (*MetJECsysUp)[n] ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"MetPhi"),  (*MetPhiJECsysUp)[n] ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"nVtx"), PV_npvs ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"llMZw"), ((*selectedIsoLeptons)[0]->p4_ + (*selectedIsoLeptons)[1]->p4_).M() ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"MVATU"), MVAS_TU/MVAB_TU ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"MVATC"), MVAS_TC/MVAB_TC ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"lep3Pt"), MVA_lep3Pt,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"lep3Eta"), MVA_lep3Eta ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"bJetPt"), MVA_bJetPt ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"bJetEta"), MVA_bJetEta ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"tH_topMass"), MVA_tH_topMass ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"tH_HMass"), MVA_tH_HMass ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"tH_WtopMass"), MVA_tH_WtopMass ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"tH_W1HMass"), MVA_tH_W1HMass ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"tH_W2HMass"), MVA_tH_W2HMass ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"tH_HPt"), MVA_tH_HPt,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"tH_HEta"), MVA_tH_HEta,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"tH_topPt"),MVA_tH_topPt ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"tH_topEta"),MVA_tH_topEta ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"tH_drWtopB"), MVA_tH_drWtopB ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"tH_drW1HW2H"), MVA_tH_drW1HW2H ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"tZ_topMass"), MVA_tZ_topMass ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"tZ_ZMass"),MVA_tZ_ZMass ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"tZ_WtopMass"),MVA_tZ_WtopMass ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"tZ_ZPt"),MVA_tZ_ZPt ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"tZ_ZEta"),MVA_tZ_ZEta ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"tZ_topPt"), MVA_tZ_topPt ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"tZ_topEta"), MVA_tZ_topEta,wgt,nIt);
//##         if((*JECsysUp)[n].size()>0){
//##           normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"jet1Pt"), (*JECsysUp)[n][0]->pt_ ,wgt, nIt);
//##           normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"jet1Eta"), (*JECsysUp)[n][0]->eta_ ,wgt, nIt);
//##           normalFillD4Hists(HistsNormalSysUp, allCh, reg, vInd(vars,"jet1Phi"), (*JECsysUp)[n][0]->phi_ ,wgt, nIt);
//##         }
//##       }
//##
//##
//##      resetVecInt(reg);
//##      myreg = findRegion(&(*JECsysDown)[n],  ch, chIso);
//##      if (myreg>=0){
//##        reg[myreg]=myreg;
//##        wgt[0][myreg]=weight_lepB;
//##        wcfit[0][myreg]= *eft_fit;
//##      }
//##       evaluateMVA(&(*JECsysDown)[n], selectedIsoLeptons, Z_P, channels[ch], (*MetJECsysDown)[n], (*MetPhiJECsysDown)[n], MVAS_TU, MVAB_TU, MVAS_TC, MVAB_TC);
//##       FillD4Hists(HistsSysDown, allCh, reg, vInd(varsFullSys,"MVATU"), MVAS_TU/MVAB_TU ,wgt, wcfit,it);
//##       FillD4Hists(HistsSysDown, allCh, reg, vInd(varsFullSys,"MVATC"), MVAS_TC/MVAB_TC ,wgt, wcfit,it);
//##       if(normalItt!= sysNormal.end()) {
//##         int nIt = std::distance(sysNormal.begin(), normalItt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"lep1Pt"), (*selectedIsoLeptons)[0]->pt_ ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"lep1Eta"), (*selectedIsoLeptons)[0]->eta_ ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"lep1Phi"), (*selectedIsoLeptons)[0]->phi_ ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"lep2Pt"), (*selectedIsoLeptons)[1]->pt_ ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"lep2Eta"), (*selectedIsoLeptons)[1]->eta_ ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"lep2Phi"), (*selectedIsoLeptons)[1]->phi_ ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"llM"), ((*selectedIsoLeptons)[0]->p4_ + (*selectedIsoLeptons)[1]->p4_).M() ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"llPt"), ((*selectedIsoLeptons)[0]->p4_ + (*selectedIsoLeptons)[1]->p4_).Pt() ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"llDr"), deltaR((*selectedIsoLeptons)[0]->eta_,(*selectedIsoLeptons)[0]->phi_,(*selectedIsoLeptons)[1]->eta_,(*selectedIsoLeptons)[1]->phi_) ,wgt,nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"llDphi"), abs(deltaPhi((*selectedIsoLeptons)[0]->phi_,(*selectedIsoLeptons)[1]->phi_)) ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"njet"), (*JECsysDown)[n].size() ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"nbjetOut"), NbTag ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"nNonBjet"), NnonbTag ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"Met"), (*MetJECsysDown)[n] ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"MetPhi"), (*MetPhiJECsysDown)[n] ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"nVtx"), PV_npvs ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"llMZw"), ((*selectedIsoLeptons)[0]->p4_ + (*selectedIsoLeptons)[1]->p4_).M() ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"MVATU"), MVAS_TU/MVAB_TU ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"MVATC"), MVAS_TC/MVAB_TC ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"lep3Pt"), MVA_lep3Pt,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"lep3Eta"), MVA_lep3Eta ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"bJetPt"), MVA_bJetPt ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"bJetEta"), MVA_bJetEta ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"tH_topMass"), MVA_tH_topMass ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"tH_HMass"), MVA_tH_HMass ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"tH_WtopMass"), MVA_tH_WtopMass ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"tH_W1HMass"), MVA_tH_W1HMass ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"tH_W2HMass"), MVA_tH_W2HMass ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"tH_HPt"), MVA_tH_HPt,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"tH_HEta"), MVA_tH_HEta,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"tH_topPt"),MVA_tH_topPt ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"tH_topEta"),MVA_tH_topEta ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"tH_drWtopB"), MVA_tH_drWtopB ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"tH_drW1HW2H"), MVA_tH_drW1HW2H ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"tZ_topMass"), MVA_tZ_topMass ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"tZ_ZMass"),MVA_tZ_ZMass ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"tZ_WtopMass"),MVA_tZ_WtopMass ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"tZ_ZPt"),MVA_tZ_ZPt ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"tZ_ZEta"),MVA_tZ_ZEta ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"tZ_topPt"), MVA_tZ_topPt ,wgt, nIt);
//##         normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"tZ_topEta"), MVA_tZ_topEta,wgt,nIt);
//##         if((*JECsysDown)[n].size()>0){
//##           normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"jet1Pt"),  (*JECsysDown)[n][0]->pt_ ,wgt, nIt);
//##           normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"jet1Eta"),  (*JECsysDown)[n][0]->eta_ ,wgt, nIt);
//##           normalFillD4Hists(HistsNormalSysDown, allCh, reg, vInd(vars,"jet1Phi"),  (*JECsysDown)[n][0]->phi_ ,wgt, nIt);
//##         }
//##       }
//##     }
//##     delete eft_fit;
//##
//##//    for (int i = 0, idx = 0; i <= Dim; ++i) {
//##//      if(i>0) cout<<sys[i-1]<<endl;
//##//      for (int j = 0; j <= i; ++j, ++idx) {
//##//        std::cout << "(" << i << "," << j << "): "<< std::setw(5) << sysFitCoefficientsUp[idx] << "\n";
//##//      }
//##//    }
//##
//##    eft_fit = new WCFit(Dim, sys_std, N, sysFitCoefficientsUp, 1.0f);
//##//cout<<eft_fit->evalPoint("S1",1)<<" "<<eft_fit->evalPoint("S2",1)<<endl;
//##    resetVecInt(reg);
//##    myreg = findRegion(selectedJets, ch, chIso);
//##    if (myreg>=0){
//##      reg[myreg]=myreg;
//##      wgt[0][myreg]=weight_lepB;
//##      wcfit[0][myreg]= *eft_fit;
//##    }
//##    evaluateMVA(selectedJets, selectedIsoLeptons, Z_P, channels[ch],  PFMET_pt ,PFMET_phi, MVAS_TU, MVAB_TU, MVAS_TC, MVAB_TC);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"lep1Pt"), (*selectedIsoLeptons)[0]->pt_ ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"lep1Eta"), (*selectedIsoLeptons)[0]->eta_ ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"lep1Phi"), (*selectedIsoLeptons)[0]->phi_ ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"lep2Pt"), (*selectedIsoLeptons)[1]->pt_ ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"lep2Eta"), (*selectedIsoLeptons)[1]->eta_ ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"lep2Phi"), (*selectedIsoLeptons)[1]->phi_ ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"llM"), ((*selectedIsoLeptons)[0]->p4_ + (*selectedIsoLeptons)[1]->p4_).M() ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"llPt"), ((*selectedIsoLeptons)[0]->p4_ + (*selectedIsoLeptons)[1]->p4_).Pt() ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"llDr"), deltaR((*selectedIsoLeptons)[0]->eta_,(*selectedIsoLeptons)[0]->phi_,(*selectedIsoLeptons)[1]->eta_,(*selectedIsoLeptons)[1]->phi_) ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"llDphi"), abs(deltaPhi((*selectedIsoLeptons)[0]->phi_,(*selectedIsoLeptons)[1]->phi_)) ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"njet"), selectedJets->size() ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"nbjetOut"), nbjetOut ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"nNonBjet"), countNonBjets(selectedJets) ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"Met"), PFMET_pt ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"MetPhi"), PFMET_phi ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"nVtx"), PV_npvs ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"llMZw"), ((*selectedIsoLeptons)[0]->p4_ + (*selectedIsoLeptons)[1]->p4_).M() ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"MVATU"), MVAS_TU/MVAB_TU ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"MVATC"), MVAS_TC/MVAB_TC ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"lep3Pt"), MVA_lep3Pt,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"lep3Eta"), MVA_lep3Eta ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"bJetPt"), MVA_bJetPt ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"bJetEta"), MVA_bJetEta ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"tH_topMass"), MVA_tH_topMass ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"tH_HMass"), MVA_tH_HMass ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"tH_WtopMass"), MVA_tH_WtopMass ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"tH_W1HMass"), MVA_tH_W1HMass ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"tH_W2HMass"), MVA_tH_W2HMass ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"tH_HPt"), MVA_tH_HPt,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"tH_HEta"), MVA_tH_HEta,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"tH_topPt"),MVA_tH_topPt ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"tH_topEta"),MVA_tH_topEta ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"tH_drWtopB"), MVA_tH_drWtopB ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"tH_drW1HW2H"), MVA_tH_drW1HW2H ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"tZ_topMass"), MVA_tZ_topMass ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"tZ_ZMass"),MVA_tZ_ZMass ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"tZ_WtopMass"),MVA_tZ_WtopMass ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"tZ_ZPt"),MVA_tZ_ZPt ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"tZ_ZEta"),MVA_tZ_ZEta ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"tZ_topPt"), MVA_tZ_topPt ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"tZ_topEta"), MVA_tZ_topEta,wgt, wcfit);
//##    if(selectedJets->size()>0){
//##      FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"jet1Pt"), (*selectedJets)[0]->pt_ ,wgt, wcfit);
//##      FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"jet1Eta"), (*selectedJets)[0]->eta_ ,wgt, wcfit);
//##      FillD3Hists(HistsSysCompactUp, allCh, reg, vInd(vars,"jet1Phi"), (*selectedJets)[0]->phi_ ,wgt, wcfit);
//##    }
//## 
//##    delete eft_fit;
//##    eft_fit = new WCFit(Dim, sys_std, N, sysFitCoefficientsDown, 1.0f);
//##    if (myreg>=0){
//##      wcfit[0][myreg]= *eft_fit;
//##    }
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"lep1Pt"), (*selectedIsoLeptons)[0]->pt_ ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"lep1Eta"), (*selectedIsoLeptons)[0]->eta_ ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"lep1Phi"), (*selectedIsoLeptons)[0]->phi_ ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"lep2Pt"), (*selectedIsoLeptons)[1]->pt_ ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"lep2Eta"), (*selectedIsoLeptons)[1]->eta_ ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"lep2Phi"), (*selectedIsoLeptons)[1]->phi_ ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"llM"), ((*selectedIsoLeptons)[0]->p4_ + (*selectedIsoLeptons)[1]->p4_).M() ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"llPt"), ((*selectedIsoLeptons)[0]->p4_ + (*selectedIsoLeptons)[1]->p4_).Pt() ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"llDr"), deltaR((*selectedIsoLeptons)[0]->eta_,(*selectedIsoLeptons)[0]->phi_,(*selectedIsoLeptons)[1]->eta_,(*selectedIsoLeptons)[1]->phi_) ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"llDphi"), abs(deltaPhi((*selectedIsoLeptons)[0]->phi_,(*selectedIsoLeptons)[1]->phi_)) ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"njet"), selectedJets->size() ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"nbjetOut"), nbjetOut ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"nNonBjet"), countNonBjets(selectedJets) ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"Met"), PFMET_pt ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"MetPhi"), PFMET_phi ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"nVtx"), PV_npvs ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"llMZw"), ((*selectedIsoLeptons)[0]->p4_ + (*selectedIsoLeptons)[1]->p4_).M() ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"MVATU"), MVAS_TU/MVAB_TU ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"MVATC"), MVAS_TC/MVAB_TC ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"lep3Pt"), MVA_lep3Pt,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"lep3Eta"), MVA_lep3Eta ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"bJetPt"), MVA_bJetPt ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"bJetEta"), MVA_bJetEta ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"tH_topMass"), MVA_tH_topMass ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"tH_HMass"), MVA_tH_HMass ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"tH_WtopMass"), MVA_tH_WtopMass ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"tH_W1HMass"), MVA_tH_W1HMass ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"tH_W2HMass"), MVA_tH_W2HMass ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"tH_HPt"), MVA_tH_HPt,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"tH_HEta"), MVA_tH_HEta,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"tH_topPt"),MVA_tH_topPt ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"tH_topEta"),MVA_tH_topEta ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"tH_drWtopB"), MVA_tH_drWtopB ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"tH_drW1HW2H"), MVA_tH_drW1HW2H ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"tZ_topMass"), MVA_tZ_topMass ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"tZ_ZMass"),MVA_tZ_ZMass ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"tZ_WtopMass"),MVA_tZ_WtopMass ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"tZ_ZPt"),MVA_tZ_ZPt ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"tZ_ZEta"),MVA_tZ_ZEta ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"tZ_topPt"), MVA_tZ_topPt ,wgt, wcfit);
//##    FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"tZ_topEta"), MVA_tZ_topEta,wgt, wcfit);
//##    if(selectedJets->size()>0){
//##      FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"jet1Pt"), (*selectedJets)[0]->pt_ ,wgt, wcfit);
//##      FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"jet1Eta"), (*selectedJets)[0]->eta_ ,wgt, wcfit);
//##      FillD3Hists(HistsSysCompactDown, allCh, reg, vInd(vars,"jet1Phi"), (*selectedJets)[0]->phi_ ,wgt, wcfit);
//##    }
//##    delete eft_fit;
//##  }
//##//Fill a tree for doing MVA training
//##   if (nbjetOut==1 && ((ch<30 && !channels[ch].Contains("CR")) || chIso<30)) {
//##     bt_tZFCNC.SetPtEtaPhiM(0,0,0,0); nut_tZFCNC.SetPtEtaPhiM(0,0,0,0);
//##     hU_tHFCNC.SetPtEtaPhiM(0,0,0,0); hD_tHFCNC.SetPtEtaPhiM(0,0,0,0); met_tHFCNC.SetPtEtaPhiM(0,0,0,0); tB_tHFCNC.SetPtEtaPhiM(0,0,0,0);
//##     for (UInt_t l=0;l<selectedJets->size();l++){
//##       if((*selectedJets)[l]->btag_){
//##         tB_tHFCNC=(*selectedJets)[l]->p4_;
//##         bt_tZFCNC.SetPtEtaPhiM((*selectedJets)[l]->pt_,(*selectedJets)[l]->eta_,(*selectedJets)[l]->phi_,0);
//##           break;
//##       }
//##     }
//##     met_tHFCNC.SetPtEtaPhiM(PFMET_pt,0,PFMET_phi,0);
//##     nut_tZFCNC.SetPtEtaPhiM(PFMET_pt,0,PFMET_phi,0);
//##     float bestMass=10000;
//##     for (UInt_t i=0;i<selectedJets->size();i++){
//##       if(selectedJets->size()==2 && !(*selectedJets)[i]->btag_) hU_tHFCNC=(*selectedJets)[i]->p4_;
//##         for (UInt_t j=i+1;j<selectedJets->size();j++){
//##           if((*selectedJets)[i]->btag_ || (*selectedJets)[j]->btag_) continue;
//##           if(abs(((*selectedJets)[i]->p4_ + (*selectedJets)[j]->p4_).M()-80)<bestMass){
//##            hU_tHFCNC=(*selectedJets)[i]->p4_; hD_tHFCNC=(*selectedJets)[j]->p4_;
//##            bestMass=((*selectedJets)[i]->p4_ + (*selectedJets)[j]->p4_).M();
//##          }
//##        }
//##     }
//##     lt_tZFCNC.SetPtEtaPhiM(0,0,0,0); lpZ_tZFCNC.SetPtEtaPhiM(0,0,0,0); lmZ_tZFCNC.SetPtEtaPhiM(0,0,0,0);
//##     hL_tHFCNC.SetPtEtaPhiM(0,0,0,0); tL_tHFCNC.SetPtEtaPhiM(0,0,0,0);
//##     tL_tHFCNC=(*selectedNonIsoLeptons)[0]->p4_;
//##     hL_tHFCNC=(*selectedNonIsoLeptons)[1]->p4_;
//##     jigsawThFCNC->Analyze(tL_tHFCNC, hL_tHFCNC, tB_tHFCNC, hU_tHFCNC, hD_tHFCNC, met_tHFCNC);
//##     if(Z_FP->size()>0 && selectedNonIsoLeptons->size()>2){
//##       lpZ_tZFCNC.SetPtEtaPhiM((*selectedNonIsoLeptons)[(*Z_FP)[lZfp]->lep1_]->pt_,(*selectedNonIsoLeptons)[(*Z_FP)[lZfp]->lep1_]->eta_,(*selectedNonIsoLeptons)[(*Z_FP)[lZfp]->lep1_]->phi_,0);
//##       lmZ_tZFCNC.SetPtEtaPhiM((*selectedNonIsoLeptons)[(*Z_FP)[lZfp]->lep2_]->pt_,(*selectedNonIsoLeptons)[(*Z_FP)[lZfp]->lep2_]->eta_,(*selectedNonIsoLeptons)[(*Z_FP)[lZfp]->lep2_]->phi_,0);
//##       for (UInt_t l=0;l<selectedNonIsoLeptons->size();l++){
//##         if(l!=(*Z_FP)[lZfp]->lep1_ && l!=(*Z_FP)[lZfp]->lep2_)  lt_tZFCNC.SetPtEtaPhiM((*selectedNonIsoLeptons)[l]->pt_,(*selectedNonIsoLeptons)[l]->eta_,(*selectedNonIsoLeptons)[l]->phi_,0);
//##       }
//##     }
//##     else{
//##      if(selectedNonIsoLeptons->size()>0) lpZ_tZFCNC.SetPtEtaPhiM((*selectedNonIsoLeptons)[0]->pt_,(*selectedNonIsoLeptons)[0]->eta_,(*selectedNonIsoLeptons)[0]->phi_,0);
//##      if(selectedNonIsoLeptons->size()>1) lmZ_tZFCNC.SetPtEtaPhiM((*selectedNonIsoLeptons)[1]->pt_,(*selectedNonIsoLeptons)[1]->eta_,(*selectedNonIsoLeptons)[1]->phi_,0);
//##      if(selectedNonIsoLeptons->size()>2) lt_tZFCNC.SetPtEtaPhiM((*selectedNonIsoLeptons)[2]->pt_,(*selectedNonIsoLeptons)[2]->eta_,(*selectedNonIsoLeptons)[2]->phi_,0);
//##     }
//##     jigsawTzFCNC->Analyze(lt_tZFCNC, lpZ_tZFCNC, lmZ_tZFCNC, bt_tZFCNC,nut_tZFCNC);
//##
//##     tH_topMass_=jigsawThFCNC->T->GetMass();
//##     tH_HMass_=jigsawThFCNC->H->GetMass();
//##     tH_WtopMass_=jigsawThFCNC->WT->GetMass();
//##     tH_W1HMass_=jigsawThFCNC->W1H->GetMass();
//##     tH_W2HMass_=jigsawThFCNC->W2H->GetMass();
//##     tH_HPt_=jigsawThFCNC->H->GetFourVector().Pt();
//##     tH_HEta_=jigsawThFCNC->H->GetFourVector().Eta();
//##     tH_topPt_=jigsawThFCNC->T->GetFourVector().Pt();
//##     tH_topEta_=jigsawThFCNC->T->GetFourVector().Eta();
//##     tH_drWtopB_=deltaR(jigsawThFCNC->WT->GetFourVector().Eta(),jigsawThFCNC->WT->GetFourVector().Phi(),tB_tHFCNC.Eta(),tB_tHFCNC.Phi());
//##     tH_drW1HW2H_=deltaR(jigsawThFCNC->W1H->GetFourVector().Eta(),jigsawThFCNC->W1H->GetFourVector().Phi(),jigsawThFCNC->W2H->GetFourVector().Eta(),jigsawThFCNC->W2H->GetFourVector().Phi());
//##
//##     tZ_topMass_=jigsawTzFCNC->T->GetMass();
//##     tZ_ZMass_=jigsawTzFCNC->Z->GetMass();
//##     tZ_WtopMass_=jigsawTzFCNC->W->GetMass();;
//##     tZ_ZPt_=jigsawTzFCNC->Z->GetFourVector().Pt();
//##     tZ_ZEta_=jigsawTzFCNC->Z->GetFourVector().Eta();
//##     tZ_topPt_=jigsawTzFCNC->T->GetFourVector().Pt();
//##     tZ_topEta_=jigsawTzFCNC->T->GetFourVector().Eta();
//##
//##     HT_=HTjets;
//##     lep1Pt_= (*selectedNonIsoLeptons)[0]->pt_;
//##     lep1Eta_= (*selectedNonIsoLeptons)[0]->eta_;
//##     lep2Pt_= (*selectedNonIsoLeptons)[1]->pt_;
//##     lep2Eta_= (*selectedNonIsoLeptons)[1]->eta_;
//##     llM_= ((*selectedNonIsoLeptons)[0]->p4_ + (*selectedNonIsoLeptons)[1]->p4_).M();
//##     llPt_=  ((*selectedNonIsoLeptons)[0]->p4_ + (*selectedNonIsoLeptons)[1]->p4_).Pt();
//##     llDr_= deltaR((*selectedNonIsoLeptons)[0]->eta_,(*selectedNonIsoLeptons)[0]->phi_,(*selectedNonIsoLeptons)[1]->eta_,(*selectedNonIsoLeptons)[1]->phi_);
//##     llDphi_= abs(deltaPhi((*selectedNonIsoLeptons)[0]->phi_,(*selectedNonIsoLeptons)[1]->phi_));
//##     if(selectedNonIsoLeptons->size()>2){
//##       lep3Pt_= (*selectedNonIsoLeptons)[2]->pt_;
//##       lep3Eta_= (*selectedNonIsoLeptons)[2]->eta_;
//##     }
//##     nJets_=selectedJets->size();
//##     jet1Pt_=0;
//##     jet1Eta_=-999;
//##     if(selectedJets->size()>0){
//##       jet1Pt_=(*selectedJets)[0]->pt_;
//##       jet1Eta_=(*selectedJets)[0]->eta_;
//##      for (UInt_t l=0;l<selectedJets->size();l++){
//##        if((*selectedJets)[l]->btag_){
//##          bJetPt_=(*selectedJets)[l]->pt_;
//##          bJetEta_=(*selectedJets)[l]->eta_;
//##          break;
//##        }
//##      }
//##     }
//##     ch_=ch;
//##     chIso_=chIso;
//##     weightSM_=weight_lepB;
//##     weightSMfake_=weight_lepB*fakeRate;
//##
//##     if (iseft) eft_fit = new WCFit(nWCnames, wc_names_lst, nEFTfitCoefficients, EFTfitCoefficients, weight_EFT);
//##     else eft_fit = new WCFit(0,wc_names_lst,1, &genWeight, 1.0);
//##     weightctp_ = eft_fit->evalPoint("ctp",1);
//##     weightctlS_= eft_fit->evalPoint("ctlS",1);
//##     weightcte_= eft_fit->evalPoint("cte",1);
//##     weightctl_= eft_fit->evalPoint("ctl",1);
//##     weightctlT_= eft_fit->evalPoint("ctlT",1);
//##     weightctZ_= eft_fit->evalPoint("ctZ",1);
//##     weightcpt_= eft_fit->evalPoint("cpt",1);
//##     weightcpQM_= eft_fit->evalPoint("cpQM",1);
//##     weightctA_= eft_fit->evalPoint("ctA",1);
//##     weightcQe_= eft_fit->evalPoint("cQe",1);
//##     weightctG_= eft_fit->evalPoint("ctG",1);
//##     weightcQlM_= eft_fit->evalPoint("cQlM",1);
//##     delete eft_fit;
//##  
//##    if(MVAB_TU!=0) MVATU_=MVAS_TU/MVAB_TU;
//##    else MVATU_= -1;
//## //   tree_out->Fill();
//##   }
//##
   objectSelectionEnd();
//##//cout<<1<<" "; printMemoryUsage();
  }
  memory = getValue(); 
  cout<<"Virtual Memory: "<<getValue()/1000000.0<<" GB"<<endl;
  printMemoryUsage();
  cout<<"Loop is completed"<<endl;
  cout<<"from "<<ntr<<" events, "<<nAccept<<" events are accepted"<<endl;
  TFile file_out (("ANoutput" + std::to_string(end) + ".root").c_str(),"RECREATE");
  endHists(data, year, ifSys);
  h2_BTaggingEff_Denom_b   ->Write("",TObject::kOverwrite);
  h2_BTaggingEff_Denom_c   ->Write("",TObject::kOverwrite);
  h2_BTaggingEff_Denom_udsg->Write("",TObject::kOverwrite);
  h2_BTaggingEff_Num_b     ->Write("",TObject::kOverwrite);
  h2_BTaggingEff_Num_c     ->Write("",TObject::kOverwrite);
  h2_BTaggingEff_Num_udsg  ->Write("",TObject::kOverwrite);
  crossSection             ->Write("",TObject::kOverwrite);

  tTagProbQcdAll->Write("",TObject::kOverwrite);
  tTagProbQcdB->Write("",TObject::kOverwrite);
  tTagProbQcdNonB->Write("",TObject::kOverwrite);
  tTagProbTop->Write("",TObject::kOverwrite);

  tTag_ee_pt_Den->Write("",TObject::kOverwrite); 
  tTag_ee_pt_Num->Write("",TObject::kOverwrite);
  tTag_ee_eta_Den->Write("",TObject::kOverwrite);
  tTag_ee_eta_Num->Write("",TObject::kOverwrite);
  tTag_ee_mass_Den->Write("",TObject::kOverwrite); 
  tTag_ee_mass_Num->Write("",TObject::kOverwrite); 
  tTag_ee_lfrac_Den->Write("",TObject::kOverwrite);
  tTag_ee_lfrac_Num->Write("",TObject::kOverwrite);

  tTag_mm_pt_Den->Write("",TObject::kOverwrite); 
  tTag_mm_pt_Num->Write("",TObject::kOverwrite); 
  tTag_mm_eta_Den->Write("",TObject::kOverwrite);
  tTag_mm_eta_Num->Write("",TObject::kOverwrite);
  tTag_mm_mass_Den->Write("",TObject::kOverwrite); 
  tTag_mm_mass_Num->Write("",TObject::kOverwrite); 
  tTag_mm_lfrac_Den->Write("",TObject::kOverwrite);
  tTag_mm_lfrac_Num->Write("",TObject::kOverwrite);

  mll_SS_Zwindow_0jet->Write("",TObject::kOverwrite);
  jetVetoMap_before->Write("",TObject::kOverwrite);
  jetVetoMap_after->Write("",TObject::kOverwrite);
  triggEff_denom_mll->Write("",TObject::kOverwrite);
  triggEff_num_mll->Write("",TObject::kOverwrite);
  triggEff_denom_ptl1->Write("",TObject::kOverwrite);
  triggEff_num_ptl1->Write("",TObject::kOverwrite);
  cutFlow->Write("",TObject::kOverwrite);
  cutFlowFinal->Write("",TObject::kOverwrite);
  tagEff->Write("",TObject::kOverwrite);
  mass1->Write("",TObject::kOverwrite);
  mass2->Write("",TObject::kOverwrite);
  mass3->Write("",TObject::kOverwrite);
  mass4->Write("",TObject::kOverwrite);
  RECOmass0->Write("",TObject::kOverwrite);
  RECOmass1->Write("",TObject::kOverwrite);
  RECOmass2->Write("",TObject::kOverwrite); 
  lep0Pt->Write("",TObject::kOverwrite); 
//  genAnalysis.writeGENHists();
  file_out.Close() ;

  delete h2_BTaggingEff_Denom_b    ;
  delete h2_BTaggingEff_Denom_c    ;
  delete h2_BTaggingEff_Denom_udsg ;
  delete h2_BTaggingEff_Num_b      ;
  delete h2_BTaggingEff_Num_c      ;
  delete h2_BTaggingEff_Num_udsg   ;
  delete tTagProbQcdAll;
  delete tTagProbQcdB;
  delete tTagProbQcdNonB;
  delete tTagProbTop;
  delete crossSection;
  delete mll_SS_Zwindow_0jet;
  delete jetVetoMap_before;
  delete jetVetoMap_after;
  delete triggEff_denom_mll;
  delete triggEff_num_mll;
  delete triggEff_denom_ptl1;
  delete triggEff_num_ptl1;
  delete cutFlow;
  delete tagEff;
  delete mass1;
  delete mass2;
  delete mass3;
  delete mass4;
  delete RECOmass0;
  delete RECOmass1;
  delete RECOmass2;
  delete cutFlowFinal;   

  delete tTag_ee_pt_Den;
  delete tTag_ee_pt_Num;
  delete tTag_ee_eta_Den;
  delete tTag_ee_eta_Num;
  delete tTag_ee_mass_Den;
  delete tTag_ee_mass_Num;
  delete tTag_ee_lfrac_Den;
  delete tTag_ee_lfrac_Num;

  delete tTag_mm_pt_Den;
  delete tTag_mm_pt_Num;
  delete tTag_mm_eta_Den;
  delete tTag_mm_eta_Num;
  delete tTag_mm_mass_Den;
  delete tTag_mm_mass_Num;
  delete tTag_mm_lfrac_Den;
  delete tTag_mm_lfrac_Num;

  malloc_trim(0);
  cout<<  "File ANoutput" + std::to_string(end) + ".root is made"<<endl;
  cout<<"Job is finished"<<endl;
  cout<<"Total Virtual Memory: "<<getValue()/1048576.0<<" GB"<<endl;

}


void MyAnalysis::FillD3Hists(D3HistsContainer H3, std::vector<int> v1, std::vector<int> v2, int v3, float value, std::vector<std::vector<float>> weight, std::vector<std::vector<WCFit>> wcfit){
  for (UInt_t i = 0; i < v1.size(); ++i) {
    for (UInt_t j = 0; j < v2.size(); ++j) {
      if(v1[i]<0 || v2[j]<0) continue;
//      std::cout << "Accessing hist at [" << v1[i] << "][" << v2[j] << "]["<<v3<< std::endl;
// cout<<channels[v1[i]]<<" for "<<regions[v2[j]]<<" is files"<<endl;
      H3[v1[i]][v2[j]][v3]->Fill(value, weight[i][j], wcfit[i][j]);
    }
  }
}

void MyAnalysis::FillD4Hists(D4HistsContainer H4, std::vector<int> v1, std::vector<int> v2, int v3, float value, std::vector<std::vector<float>> weight, std::vector<std::vector<WCFit>> wcfit, int n){
  for (UInt_t i = 0; i < v1.size(); ++i) {
    for (UInt_t j = 0; j < v2.size(); ++j) {
      if(v1[i]<0 || v2[j]<0) continue;
      H4[v1[i]][v2[j]][v3][n]->Fill(value, weight[i][j], wcfit[i][j]);
    }
  }
}

void MyAnalysis::normalFillD4Hists(normalD4HistsContainer H4, std::vector<int> v1, std::vector<int> v2, int v3, float value, std::vector<std::vector<float>> weight, int n){
  for (UInt_t i = 0; i < v1.size(); ++i) {
    for (UInt_t j = 0; j < v2.size(); ++j) {
      if(v1[i]<0 || v2[j]<0) continue;
      H4[v1[i]][v2[j]][v3][n]->Fill(value, weight[i][j]);
    }
  }
}
