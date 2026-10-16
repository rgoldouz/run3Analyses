#include "MyAnalysis.h"
#include "JigsawRecTZFCNC.h"
#include "JigsawRecTHFCNC.h"
#include "Utils.h"

float MyAnalysis::recoTT(TLorentzVector& recoTop1, TLorentzVector& recoTop2, const lepton_candidate* l1, const lepton_candidate* l2, int ntjet, TString year){
  float mass;	
  TLorentzVector lep1, lep2;
  TLorentzVector jet1, jet2;
  TLorentzVector bjet1, bjet2;
  TLorentzVector nu1, nu2;
  TLorentzVector top1, top2;
  TLorentzVector Z;
  float best1,best2;
  double METx2, METy2;  
  bool chi;
  //resolved (just find mllbb)
  if(ntjet==0){
    lep1=l1->p4_;
    lep2=l2->p4_;	  
    for(int i=0;i<selectedJets->size();i++){
      if((*selectedJets)[i]->btag_){
	if(bjet1.Pt()==0){      
          bjet1 = (*selectedJets)[i]->p4_;
	}
	else{
	  bjet2 = (*selectedJets)[i]->p4_;	
        }
      }
    }
  mass=(bjet1 + lep1 + bjet2 + lep2).M(); 
  }
  //semi boosted
  else if(ntjet==1){
    bool isl1lep1=false;	  
    for(int j=0;j<selectedJets08->size();j++){
      auto* jet = (*selectedJets08)[j];	    
      if(jet->lepFlavor_ > 0 && jet->istop(jet->topTagBDT_,year)){
	 jet1=jet->p4_;
	 lep1 = (*selectedNonIsoLeptons)[jet->selectedLepIndex_]->p4_;
	 isl1lep1 = l1->indice_ == (*selectedNonIsoLeptons)[jet->selectedLepIndex_]->indice_;
	 break;
      }	 
    }

    bool found_bjet=false;
    for(int i=0;i<selectedJets->size();i++){
      if(!(*selectedJets)[i]->btag_) continue;
      bjet2 = (*selectedJets)[i]->p4_;
      break;
    }

    if (isl1lep1) lep2 = l2->p4_;
    else lep2 = l1->p4_;

    jet2= bjet2 + lep2;
    chi = solveNeutrinosChi2(lep1,lep2,jet1, jet2, PuppiMET_pt*cos(PuppiMET_phi),PuppiMET_pt*sin(PuppiMET_phi),nu1,nu2);
    top1=jet1 + nu1;
    top2 = jet2 + nu2;
    mass = (top1 + top2).M();
  }
  //fully boosted
  else{
    for(int j=0; j<selectedJets08->size(); j++){
      auto* jet = (*selectedJets08)[j];
      if(jet->lepFlavor_ > 0 && jet->istop(jet->topTagBDT_,year)){
	if(jet1.Pt() ==0){     
        jet1=jet->p4_;
        lep1 = (*selectedNonIsoLeptons)[jet->selectedLepIndex_]->p4_;
	}
	else{
        jet2=jet->p4_;
        lep2 = (*selectedNonIsoLeptons)[jet->selectedLepIndex_]->p4_;
        }
      }
    }
    chi = solveNeutrinosChi2(lep1,lep2,jet1, jet2, PuppiMET_pt*cos(PuppiMET_phi),PuppiMET_pt*sin(PuppiMET_phi),nu1,nu2);
    top1=jet1 + nu1;
    top2 = jet2 + nu2;   
    mass = (top1 + top2).M();
//    mass = (jet1 + jet2).M();
//   if((top1 + top2).M()>2000) cout<<(top1 + top2).M()<<":"<<deltaPhi(jet1.Phi(),jet2.Phi())<<":"<<top1.M()<<":"<<top2.M()<<":"<<"    j1,j2 mass"<<jet1.M()<<":"<<jet2.M()<<endl;
//   if((top1 + top2).M()>2000)  cout<<pow(top1.M()-172,2)<<":"<<pow(top2.M()-172,2)<<":"<<pow((nu1+nu2).Px()-PuppiMET_pt*cos(PuppiMET_phi),2)+pow((nu1+nu2).Py()-PuppiMET_pt*sin(PuppiMET_phi),2)<<endl;
  }
  recoTop1=top1;
  recoTop2=top2;
  return mass;
}  


int MyAnalysis::findRegion(std::vector<jet_candidate*> *J, int ch, int chFA){
  int reg=-1;
  int nB=0;
  TString C="";
  if (chFA<30) C=channelsFA[chFA];
  if (ch<30) C=channels[ch];
  for (UInt_t l=0;l<J->size();l++){
    if((*J)[l]->btag_) nB++;
  }
  NbTag=nB;
  NnonbTag=J->size()-nB;
  if(nB==0){
    reg=0;
  }  
  else if(nB==1){
    if (C.Contains("2l")) {
      if(J->size()<=3) reg=1;
      else reg=2;
    }
    else{
      if(J->size()==1) reg=1;
      else reg=2;
    }
  }
  else reg=3;
  return reg;
}

void MyAnalysis::evaluateTopSys(){
//Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "my_model");
//
//Ort::SessionOptions session_options;
//session_options.SetIntraOpNumThreads(1);
//
//Ort::Session session(
//    env,
//    "mymodel12_13TeV_MiNNLO_afterShower.onnx",
//    session_options
//);
//
//std::vector<float> input = {
//    pt, eta, phi, mass,
//    var5, var6, var7, var8, var9, var10
//};
//
//std::vector<int64_t> input_shape = {1, 10};
//
//Ort::MemoryInfo memory_info =
//    Ort::MemoryInfo::CreateCpu(
//        OrtArenaAllocator,
//        OrtMemTypeDefault);
//
//Ort::Value input_tensor =
//    Ort::Value::CreateTensor<float>(
//        memory_info,
//        input.data(),
//        input.size(),
//        input_shape.data(),
//        input_shape.size());
//
//Ort::AllocatorWithDefaultOptions allocator;
//
//auto input_name =
//    session.GetInputNameAllocated(0, allocator);
//
//auto output_name =
//    session.GetOutputNameAllocated(0, allocator);
//
//auto output_tensors = session.Run(
//    Ort::RunOptions{nullptr},
//    &input_name.get(),
//    &input_tensor,
//    1,
//    &output_name.get(),
//    1
//);
//
//float* output =
//    output_tensors[0].GetTensorMutableData<float>();
//
//std::cout << "NN output = " << output[0] << std::endl;

}	

void MyAnalysis::evaluateMVA(std::vector<jet_candidate*> *J, std::vector<lepton_candidate*> *L, std::vector<Z_candidate*> *Z, TString C, float metPt, float metPhi, float &MVAS_TU, float &MVAB_TU, float &MVAS_TC, float &MVAB_TC){
//  JigsawRecTZFCNC jigSawTZFCNC;
//  JigsawRecTHFCNC jigSawTHFCNC;
//  std::vector<Float_t> probs;
//  float rawBDT;
//  MVAS_TU=0;
//  MVAB_TU=1;
//  MVAS_TC=0;
//  MVAB_TC=1;
//  int bIndex=0;
//  float zValue=0;
//  int zIndex = 0;
//  int lZp=0;
//
//  int nB=0;
//  for (UInt_t l=0;l<J->size();l++){
//    if((*J)[l]->btag_) nB++;
//  }
//
//  if(Z->size()>0){
//    zValue = (*Z)[0]->mass_;
//    zIndex = 0;
//    for (UInt_t i = 1; i < Z->size(); ++i) {
//      if (std::abs((*Z)[i]->mass_ - 91.1876) > std::abs(zValue - 91.1876)) {
//        zValue = (*Z)[i]->mass_;
//        zIndex = i;
//      }
//    }
//    if(abs((*Z)[zIndex]->mass_ - 91.1876) > 10) {
//      lZp=zIndex;
//    }
//    zValue = (*Z)[0]->mass_;
//    zIndex = 0;
//    for (UInt_t i = 1; i < Z->size(); ++i) {
//      if (std::abs((*Z)[i]->mass_ - 91.1876) < std::abs(zValue - 91.1876)) {
//        zValue = (*Z)[i]->mass_;
//        zIndex = i;
//      }
//    }
//    if(abs((*Z)[zIndex]->mass_- 91.1876)<10) {
//      lZp=zIndex;
//    }
//  }
//
//
//  TLorentzVector bt_tZFCNC, lt_tZFCNC, nut_tZFCNC, lpZ_tZFCNC, lmZ_tZFCNC ;
//  TLorentzVector hL_tHFCNC, hU_tHFCNC, hD_tHFCNC, tB_tHFCNC, tL_tHFCNC, met_tHFCNC;
//
//   bt_tZFCNC.SetPtEtaPhiM(0,0,0,0); nut_tZFCNC.SetPtEtaPhiM(0,0,0,0);
//   hU_tHFCNC.SetPtEtaPhiM(0,0,0,0); hD_tHFCNC.SetPtEtaPhiM(0,0,0,0); met_tHFCNC.SetPtEtaPhiM(0,0,0,0); tB_tHFCNC.SetPtEtaPhiM(0,0,0,0);
//   for (UInt_t l=0;l<J->size();l++){
//     if((*J)[l]->btag_){
//       tB_tHFCNC=(*J)[l]->p4_;
//       bt_tZFCNC.SetPtEtaPhiM((*J)[l]->pt_,(*J)[l]->eta_,(*J)[l]->phi_,0);
//       bIndex=l;
//       break;
//     }
//   }
//   met_tHFCNC.SetPtEtaPhiM(metPt,0,metPhi,0);
//   nut_tZFCNC.SetPtEtaPhiM(metPt,0,metPhi,0);
//   float bestMass=10000;
//   for (UInt_t i=0;i<J->size();i++){
//     if(J->size()==2 && i!=bIndex) hU_tHFCNC=(*J)[i]->p4_;
//       for (UInt_t j=i+1;j<J->size();j++){
//         if(i==bIndex || j==bIndex) continue;
//         if(abs(((*J)[i]->p4_ + (*J)[j]->p4_).M()-80)<bestMass){
//          hU_tHFCNC=(*J)[i]->p4_; hD_tHFCNC=(*J)[j]->p4_;
//          bestMass=((*J)[i]->p4_ + (*J)[j]->p4_).M();
//        }
//      }
//   }
//
//  lt_tZFCNC.SetPtEtaPhiM(0,0,0,0); lpZ_tZFCNC.SetPtEtaPhiM(0,0,0,0); lmZ_tZFCNC.SetPtEtaPhiM(0,0,0,0);
//  hL_tHFCNC.SetPtEtaPhiM(0,0,0,0); tL_tHFCNC.SetPtEtaPhiM(0,0,0,0);
//  tL_tHFCNC=(*L)[0]->p4_;
//  hL_tHFCNC=(*L)[1]->p4_;
//  jigSawTHFCNC.Analyze(tL_tHFCNC, hL_tHFCNC, tB_tHFCNC, hU_tHFCNC, hD_tHFCNC, met_tHFCNC);
//  if(Z->size()>0 && L->size()>2){
//    lpZ_tZFCNC.SetPtEtaPhiM((*L)[(*Z)[lZp]->lep1_]->pt_,(*L)[(*Z)[lZp]->lep1_]->eta_,(*L)[(*Z)[lZp]->lep1_]->phi_,0);
//    lmZ_tZFCNC.SetPtEtaPhiM((*L)[(*Z)[lZp]->lep2_]->pt_,(*L)[(*Z)[lZp]->lep2_]->eta_,(*L)[(*Z)[lZp]->lep2_]->phi_,0);
//    for (UInt_t l=0;l<L->size();l++){
//      if(l!=(*Z)[lZp]->lep1_ && l!=(*Z)[lZp]->lep2_)  lt_tZFCNC.SetPtEtaPhiM((*L)[l]->pt_,(*L)[l]->eta_,(*L)[l]->phi_,0);
//    }
//  }
//  else{
//   if(L->size()>0) lpZ_tZFCNC.SetPtEtaPhiM((*L)[0]->pt_,(*L)[0]->eta_,(*L)[0]->phi_,0);
//   if(L->size()>1) lmZ_tZFCNC.SetPtEtaPhiM((*L)[1]->pt_,(*L)[1]->eta_,(*L)[1]->phi_,0);
//   if(L->size()>2) lt_tZFCNC.SetPtEtaPhiM((*L)[2]->pt_,(*L)[2]->eta_,(*L)[2]->phi_,0);
//  }
//  jigSawTZFCNC.Analyze(lt_tZFCNC, lpZ_tZFCNC, lmZ_tZFCNC, bt_tZFCNC,nut_tZFCNC);
//
//
//  MVA_nJets=J->size();
//  MVA_jet1Pt=0;
//  if(J->size()>0){
//    MVA_jet1Pt=(*J)[0]->pt_;
//   for (UInt_t l=0;l<J->size();l++){
//     if((*J)[l]->btag_){
//       MVA_bJetPt=(*J)[l]->pt_;
//       MVA_bJetEta=(*J)[l]->eta_;
//       break;
//     }
//   }
//  }
//  MVA_tH_topMass=jigSawTHFCNC.T->GetMass();
//  MVA_tH_HMass=jigSawTHFCNC.H->GetMass();
//  MVA_tH_WtopMass=jigSawTHFCNC.WT->GetMass();
//  MVA_tH_W1HMass=jigSawTHFCNC.W1H->GetMass();
//  MVA_tH_W2HMass=jigSawTHFCNC.W2H->GetMass();
//  MVA_tH_HPt=jigSawTHFCNC.H->GetFourVector().Pt();
//  MVA_tH_HEta=jigSawTHFCNC.H->GetFourVector().Eta();
//  MVA_tH_topPt=jigSawTHFCNC.T->GetFourVector().Pt();
//  MVA_tH_topEta=jigSawTHFCNC.T->GetFourVector().Eta();
//  MVA_tH_drWtopB=deltaR(jigSawTHFCNC.WT->GetFourVector().Eta(),jigSawTHFCNC.WT->GetFourVector().Phi(),tB_tHFCNC.Eta(),tB_tHFCNC.Phi());
//  MVA_tH_drW1HW2H=deltaR(jigSawTHFCNC.W1H->GetFourVector().Eta(),jigSawTHFCNC.W1H->GetFourVector().Phi(),jigSawTHFCNC.W2H->GetFourVector().Eta(),jigSawTHFCNC.W2H->GetFourVector().Phi());
//
//  MVA_tZ_topMass=jigSawTZFCNC.T->GetMass();
//  MVA_tZ_ZMass=jigSawTZFCNC.Z->GetMass();
//  MVA_tZ_WtopMass=jigSawTZFCNC.W->GetMass();;
//  MVA_tZ_ZPt=jigSawTZFCNC.Z->GetFourVector().Pt();
//  MVA_tZ_ZEta=jigSawTZFCNC.Z->GetFourVector().Eta();
//  MVA_tZ_topPt=jigSawTZFCNC.T->GetFourVector().Pt();
//  MVA_tZ_topEta=jigSawTZFCNC.T->GetFourVector().Eta();
//
//  MVA_lep1Pt=(*L)[0]->pt_;
//  MVA_lep2Pt=(*L)[1]->pt_;
//  MVA_lep1Eta=(*L)[0]->eta_;
//  MVA_lep2Eta=(*L)[1]->eta_;
//  MVA_llM=((*L)[0]->p4_ + (*L)[1]->p4_).M();
//  MVA_llPt=((*L)[0]->p4_ + (*L)[1]->p4_).Pt();
//  MVA_llDr=deltaR((*L)[0]->eta_,(*L)[0]->phi_,(*L)[1]->eta_,(*L)[1]->phi_);
//  MVA_llDphi=abs(deltaPhi((*L)[0]->phi_,(*L)[1]->phi_));
//  if(L->size()>2){
//    MVA_lep3Pt= (*L)[2]->pt_;
//    MVA_lep3Eta= (*L)[2]->eta_;
//  }
//  if(C.Contains("2l")){
//    rawBDT =  readerMVA2lss_TU->EvaluateMVA("BDT::BDT");
//    MVAS_TU=(rawBDT + 1.0);  // Maps [-1, 1] to [0, 2]
//    MVAB_TU=1;
//    rawBDT =  readerMVA2lss_TC->EvaluateMVA("BDT::BDT");
//    MVAS_TC=(rawBDT + 1.0);
//    MVAB_TC=1;
//  }
//  if(C.Contains("3lonZ")){
//    probs = readerMVA3lonZ_TU->EvaluateMulticlass("BDT::BDT");
//    for (size_t j = 0; j < probs.size()-1; ++j)   MVAS_TU=MVAS_TU+probs[j];
//    MVAB_TU=probs[probs.size()-1];
//    probs = readerMVA3lonZ_TC->EvaluateMulticlass("BDT::BDT");
//    for (size_t j = 0; j < probs.size()-1; ++j)   MVAS_TC=MVAS_TC+probs[j];
//    MVAB_TC=probs[probs.size()-1];
//  }
//
//  if(C.Contains("3loffZhigh")){
//    probs = readerMVA3loffZ_TU->EvaluateMulticlass("BDT::BDT");
//    for (size_t j = 0; j < probs.size()-1; ++j)   MVAS_TU=MVAS_TU+probs[j];
//    MVAB_TU=probs[probs.size()-1];
//    probs = readerMVA3loffZ_TC->EvaluateMulticlass("BDT::BDT");
//    for (size_t j = 0; j < probs.size()-1; ++j)   MVAS_TC=MVAS_TC+probs[j];
//    MVAB_TC=probs[probs.size()-1];
//  }
}

bool MyAnalysis::jethasTopB(float eta, float phi){
  bool foundTop = false;	
  for (unsigned int iGen = 0; iGen < nGenPart; ++iGen) {
    int pdgId = GenPart_pdgId[iGen];
    if (abs(pdgId) != 5)
        continue;
    if(deltaR(GenPart_eta[iGen],GenPart_phi[iGen],eta,phi)>0.8)
	continue;    
    Int_t parentIdx = GenPart_genPartIdxMother[iGen];
    int pdg = std::abs(GenPart_pdgId[parentIdx]);
    if (pdg == 6) {
      foundTop = true;
      break;
    }
  }
  return foundTop;
}

bool MyAnalysis::isFromTop(Int_t id){
  if (id < 0 || id >= nGenPart){
    return false;
  }
  else{
    Int_t parentIdx = GenPart_genPartIdxMother[id];
    bool foundTop = false;
    bool foundW = false;
    while (parentIdx != -1) {
      int pdg = std::abs(GenPart_pdgId[parentIdx]);
      if (pdg == 24)
          foundW = true;
      if (pdg == 6) {
          foundTop = true;
          break;
      }
      parentIdx = GenPart_genPartIdxMother[parentIdx];
    }
    return foundW && foundTop;
  }
}

bool MyAnalysis::isFromWZTau(Int_t id){
  if (id < 0 || id >= nGenPart){
    return false;
  }
  else{
    Int_t parentIdx = GenPart_genPartIdxMother[id];
    bool foundTop = false;
    bool foundW = false;
    bool foundZ = false;
    bool foundTau = false;
    while (parentIdx != -1) {
      int pdg = std::abs(GenPart_pdgId[parentIdx]);
      if (pdg == 15)
          foundTau = true;
      if (pdg == 23)
          foundZ = true;
      if (pdg == 24)
          foundW = true;
      if (pdg == 6) 
          foundTop = true;
      parentIdx = GenPart_genPartIdxMother[parentIdx];
    }
    return ((foundW || foundZ || foundTau) && !foundTop);
  }
}

bool MyAnalysis::overlapRemoval(double Et_cut, double Eta_cut, double dR_cut, bool verbose, bool reversePtCut){
    bool haveOverlap = false;
    vector<int> extraPIDIgnore={22};
    for(int mcInd=0; mcInd<nGenPart; ++mcInd){
        if(GenPart_pdgId[mcInd]==22){

            bool parentagePass=false;
            vector<double> minDR_Result = {-1.,0.};
            bool Overlaps = false;
            int maxPDGID = 0;
            bool ptCut;
            if(reversePtCut) ptCut=GenPart_pt[mcInd] < Et_cut;
            else ptCut= GenPart_pt[mcInd] >= Et_cut;
            if (ptCut &&
                fabs(GenPart_eta[mcInd]) <= Eta_cut){

                Int_t parentIdx = mcInd;
                int motherPDGID = 0;
                bool fromTopDecay = false;
                while (parentIdx != -1){
                    motherPDGID = std::abs(GenPart_pdgId[parentIdx]);
                    maxPDGID = std::max(maxPDGID,motherPDGID);
                    parentIdx = GenPart_genPartIdxMother[parentIdx];
                }

                bool parentagePass = maxPDGID < 37;
                if (parentagePass) {
                    minDR_Result= minGenDr(mcInd, extraPIDIgnore);
                    if(minDR_Result.at(0) > dR_cut) {
                        haveOverlap = true;
                        Overlaps = true;
                    }
                }
            }
            if (verbose){
                cout << " gen particle idx="<<mcInd << " pdgID="<<GenPart_pdgId[mcInd] << " status="<<GenPart_status[mcInd] << " pt="<<GenPart_pt[mcInd] << " eta=" << GenPart_eta[mcInd] << " parentage=" << (maxPDGID < 37) << " maxPDGID=" << maxPDGID << " minDR="<<minDR_Result.at(0) << " closestInd="<<minDR_Result.at(1) << " closestPDGID="<<GenPart_pdgId[(int)minDR_Result.at(1)]<<" OVERLAPS="<<Overlaps<<endl;
            }
        }
    }
    return haveOverlap;
}

std::vector<double> MyAnalysis::minGenDr(int myInd, std::vector<int> ignorePID){
    double myEta = GenPart_eta[myInd];
    double myPhi = GenPart_phi[myInd];
    int myPID = GenPart_pdgId[myInd];

    double mindr = 999.0;
    double dr;
    int bestInd = -1;
    for( int oind = 0; oind < nGenPart; oind++){
        if(oind == myInd) continue;
        if(GenPart_status[oind] != 1) continue; // check if it's final state
        if(GenPart_pt[oind] < 5)  continue;
        if(abs(GenPart_pt[oind] - GenPart_pt[myInd]) < 0.01 && (GenPart_pdgId[oind] == GenPart_pdgId[myInd]) && abs(GenPart_eta[oind] - GenPart_eta[myInd]) < 0.01)  continue; // skip if same particle
        int opid = abs(GenPart_pdgId[oind]);
        if(opid == 12 || opid == 14 || opid == 16) continue; // skip neutrinos
        if(std::find(ignorePID.begin(),ignorePID.end(),opid) != ignorePID.end()) continue; //skip any pid in ignorePID vector
        dr = dR(myEta, myPhi, GenPart_eta[oind], GenPart_phi[oind]);
        if( mindr > dr ) {
            //check if the second particle is a decay product of the first
            int genParentIdx = GenPart_genPartIdxMother[oind];
            bool isDecay = false;
            while (genParentIdx>=myInd){
                if (genParentIdx==myInd) isDecay = true;
                genParentIdx = GenPart_genPartIdxMother[genParentIdx];
            }
            if (isDecay) continue;

            mindr = dr;
            bestInd = oind;
        }
    }
    vector<double> v;
    v.push_back(mindr);
    v.push_back((double)bestInd);
    return v;
}
