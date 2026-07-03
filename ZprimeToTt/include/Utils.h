#pragma once
#ifndef UTIL_H
#define UTIL_H 

#include<TMath.h>
#include "PU_reWeighting.h"
#include "lepton_candidate.h"
#include "jet_candidate.h"
#include "TRandom.h"
#include "TRandom3.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TRandom3.h>
#include <TLorentzVector.h>
#include <time.h>
#include <iostream>
#include <cmath>
#include <vector>
#include "RoccoR.h"
//#include "BTagCalibrationStandalone.h"
#include <memory>
#include <TLorentzVector.h>
#include "TMVA/Tools.h"
#include "TMVA/Reader.h"
#include "TMVA/MethodCuts.h"
#include <iostream>
#include <map>
#include <string>
#include <bitset>
#include <random>
#include <typeinfo>
#include "TVector3.h"

double dR(double eta1, double phi1, double eta2, double phi2);
void displayProgress(long current, long max);
Double_t deltaPhi(Double_t phi1, Double_t phi2);
Double_t deltaR(Double_t eta1, Double_t phi1, Double_t eta2, Double_t phi2);
int signnum_typical(double x);
bool ComparePtLep(lepton_candidate *a, lepton_candidate *b);
bool ComparePtJet(jet_candidate *a, jet_candidate *b);
bool CompareMassJet(jet_candidate *a, jet_candidate *b);
std::vector<bool> parsePhotonVIDCuts(int bitMap, int cutLevel);
//float scale_factor( TH2F* h, float X, float Y , TString uncert);
float scale_factor( TH2F* h, float X, float Y , TString uncert,bool, bool);
float topPt(float pt);
TLorentzVector Wneutrino(double MET, double METphi, double leptonPT, double leptonEta, double leptonPhi);
int vInd(std::map<TString, std::tuple<int, int, const double*>> V, TString name);
int getVecPos(std::vector<TString> vec, string element);
bool reconstructNeutrinos(
    const TVector3& jet1_dir,
    const TVector3& jet2_dir,
    double METx,
    double METy,
    TLorentzVector& nu1,
    TLorentzVector& nu2);
bool solveNeutrinosApprox(
    const TVector3& jet1_dir,
    const TVector3& jet2_dir,
    double METx,
    double METy,
    TLorentzVector& nu1,
    TLorentzVector& nu2);

bool solveNeutrinosChi2(
    const TLorentzVector& lep1,
    const TLorentzVector& lep2,		
    const TLorentzVector& jet1,
    const TLorentzVector& jet2,
    double METx, double METy,
    TLorentzVector& nu1_best,
    TLorentzVector& nu2_best
);

TLorentzVector solveBoostedNeutrinoForTop(const TLorentzVector& pvis,  const TVector3& jet_dir);
TLorentzVector solveBoostedNeutrinoForW(const TLorentzVector& pvis,  const TVector3& jet_dir);

TLorentzVector solveResolvedNeutrino(const TLorentzVector& lep, double METx, double METy);
template <class T>
void cleanVec( std::vector<T*> *vec ){
  for( typename std::vector<T*>::iterator iter = vec->begin();  iter != vec->end(); ++iter ) delete *iter;
  vec->clear();
  vec->shrink_to_fit();
  delete vec;
}


int getIndex(std::vector<TString> words, std::string target);
#endif
