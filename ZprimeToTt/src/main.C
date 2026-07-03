#include "../include/MyAnalysis.h"
int main(){
    TChain* ch    = new TChain("Events") ;
//    ch ->Add("/cms/cephfs/data/store/user/rgoldouz/NanoAodPostProcessingUL/UL24/v1/data_UL24_H_Muon1/Muon1/crab_data_UL24_H_Muon1/260603_093722/0000/tree_63.root");
//    ch ->Add("    /cms/cephfs/data/store/user/rgoldouz/NanoAodPostProcessingUL/UL24/v1/data_UL24_H_EGamma1/EGamma1/crab_data_UL24_H_EGamma1/260603_093640/0000/tree_88.root");
//    ch ->Add("/cms/cephfs/data/store/user/rgoldouz/NanoAodPostProcessingUL/UL24/v1/UL24_Signal_ZPrimeToTTM1000W1p/ZPrimeToTT_Par-M-1000-W-10_TuneCP5_13p6TeV_madgraph-pythia8/crab_UL24_Signal_ZPrimeToTTM1000W1p/260311_111932/0000/tree_31*");
    ch ->Add("/cms/cephfs/data/store/user/rgoldouz/NanoAodPostProcessingUL/UL24/v1/UL24_DY_DYto2MuMLL400to800/DYto2Mu_Bin-MLL-400to800_TuneCP5_13p6TeV_powheg-pythia8/crab_UL24_DY_DYto2MuMLL400to800/260307_110505/0000/tree_11.root");
//    ch ->Add("/cms/cephfs/data/store/user/rgoldouz/NanoAodPostProcessingUL/UL24/v1/UL24_TT_TTTo2L2Nu/TTto2L2Nu_TuneCP5_13p6TeV_powheg-pythia8/crab_UL24_TTTo2L2Nu/260306_111411/0000/tree_1**");
    MyAnalysis * t1 = new MyAnalysis(ch);
//    t1->Analyze("data_UL24_F_EGamma2", "data" , "Muon" , "2024" , "UL24" , 1 , 109 , 1 , 0 , 1 , 0, t1);
    t1->Analyze("UL24_Signal_ZPrimeToTTM1000W1p", "mc", "none", "2024", "none", 0.01, 109, 1763000.0, 0, 1, 0, t1);
    delete t1;
}
