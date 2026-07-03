#pragma once
#ifndef MY_triggerEffAnalysis
#define MY_triggerEffAnalysis

#include<cmath>
#include<string>
#include<iostream>
#include<vector>
#include<complex>
#include <TLorentzVector.h>
#include "MyAnalysis.h"
#include "WCPoint.h"
#include "WCFit.h"
#include <map>
#include<utility>

using namespace std;
//using namespace math;
class triggerEffAnalysis{
  
public:
  triggerEffAnalysis(MyAnalysis *evt,string year);
  MyAnalysis *eI;
  virtual ~triggerEffAnalysis();
  int WInd(std::map<TString, std::pair<std::pair<int, int>, float*>>, TString name);
  bool isProbeMatchedWPTight(int l);
  bool isProbeSeeded(int l);
  bool isProbeSeededPh(int l);
  bool isProbeMatchedEt(int l, float Et);
  bool isProbeMatchedHE(int l, float Et);
  bool isProbeMatchedClusterShape(int l, float Et);
  bool isProbeMatchedPixelMatch(int l, float Et);
  bool isProbeMatchedCaloIdLMWPMS2(int l, float Et);
  bool isProbeMatchedHESeeded(int l, float Et);
  bool isProbeMatchedClusterShapeSeeded(int l, float Et);
  bool isProbeMatchedPixelMatchSeeded(int l, float Et);
  bool isProbeMatchedCaloIdLMWPMS2Seeded(int l, float Et);
  bool matchWPTight(int l);
  bool isProbe(int l);
  bool isProbePh(int l);
  bool isTag(int l);
  int etaRegion(float eta);
  void writeTriggerHists();
  void cleanTriggerHists();
  void fillTriggerHists();
  void fillTriggerHistsPh();
  float ptTag = 32;
  float etProbe = 25;
  float zMassL = 60;
  float zMassR = 120;
  int ProbeMatchedTO;
  int tagMatchTO;
  TString year;

  float eta_bins[19] = {-2.5,-2.4,-2.3,-2.2,-2.1,-1.566,-1.4442,-0.8,-0.4,0,0.4,0.8,1.4442,1.566,2.1,2.2,2.3,2.4,2.5};
  int n_bins_Ele32 = 16;
  float pt_bins_Ele32[16] = {20,22,24,26,28,30,32.5,35,38,40,45,50,60,120,200,500};
  float eta_bins_115[11] = {-2.5,-2.0,-1.566,-1.4442,-0.8,0,0.8,1.4442,1.566,2.0,2.5};
  float PU_bins[8] = {0,20,30,40,50,60,70,100};
  float mass_bins[23] = {20,70,72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,106,108,110,200};
  int n_bins_Ph = 8;
  float pt_bins_Ph[8] = {120,140,170,200,220,240,300,500};

  const std::map<TString, std::pair<std::pair<int, int>, float*>> vars =
  {
    {"h_pt_Barrel_totalEt",                     pair(pair(0,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Barrel_totalWPTightEt35",              pair(pair(1,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Barrel_totalCaloIdLMWPMS2",            pair(pair(2,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Barrel_passEt",                      pair(pair(3,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Barrel_passWPTightEt35",               pair(pair(4,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Barrel_passCaloIdLMWPMS2",             pair(pair(5,n_bins_Ele32),pt_bins_Ele32)},

    {"h_pt_Endcaps_totalEt",                    pair(pair(6,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Endcaps_totalWPTightEt35",             pair(pair(7,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Endcaps_totalCaloIdLMWPMS2",           pair(pair(8,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Endcaps_passEt",                     pair(pair(9,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Endcaps_passWPTightEt35",              pair(pair(10,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Endcaps_passCaloIdLMWPMS2",            pair(pair(11,n_bins_Ele32),pt_bins_Ele32)},

    {"h_eta_totalEt",                            pair(pair(12,19),eta_bins)},
    {"h_eta_totalWPTightEt35",                     pair(pair(13,19),eta_bins)},
    {"h_eta_totalCaloIdLMWPMS2",                   pair(pair(14,19),eta_bins)},
    {"h_eta_passEt",                             pair(pair(15,19),eta_bins)},
    {"h_eta_passWPTightEt35",                      pair(pair(16,19),eta_bins)},
    {"h_eta_passCaloIdLMWPMS2",                    pair(pair(17,19),eta_bins)},

    {"h_mass_totalEt",                           pair(pair(18,23),mass_bins)},
    {"h_mass_totalWPTightEt35",                    pair(pair(19,23),mass_bins)},
    {"h_mass_totalCaloIdLMWPMS2",                   pair(pair(20,23),mass_bins)},
    {"h_mass_passEt",                            pair(pair(21,23),mass_bins)},
    {"h_mass_passWPTightEt35",                     pair(pair(22,23),mass_bins)},
    {"h_mass_passCaloIdLMWPMS2",                   pair(pair(23,23),mass_bins)},

    {"h_pteta1_Barrel_totalEt",                  pair(pair(24,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta1_Barrel_totalWPTightEt35",           pair(pair(25,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta1_Barrel_totalCaloIdLMWPMS2",          pair(pair(26,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta1_Barrel_passEt",                   pair(pair(27,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta1_Barrel_passWPTightEt35",            pair(pair(28,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta1_Barrel_passCaloIdLMWPMS2",          pair(pair(29,n_bins_Ele32),pt_bins_Ele32)},

    {"h_pteta1_Endcaps_totalEt",                 pair(pair(30,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta1_Endcaps_totalWPTightEt35",          pair(pair(31,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta1_Endcaps_totalCaloIdLMWPMS2",        pair(pair(32,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta1_Endcaps_passEt",                  pair(pair(33,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta1_Endcaps_passWPTightEt35",           pair(pair(34,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta1_Endcaps_passCaloIdLMWPMS2",         pair(pair(35,n_bins_Ele32),pt_bins_Ele32)},

    {"h_pteta2_Barrel_totalEt",                   pair(pair(36,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta2_Barrel_totalWPTightEt35",            pair(pair(37,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta2_Barrel_totalCaloIdLMWPMS2",          pair(pair(38,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta2_Barrel_passEt",                    pair(pair(39,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta2_Barrel_passWPTightEt35",             pair(pair(40,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta2_Barrel_passCaloIdLMWPMS2",           pair(pair(41,n_bins_Ele32),pt_bins_Ele32)},

    {"h_pteta2_Endcaps_totalEt",                  pair(pair(42,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta2_Endcaps_totalWPTightEt35",           pair(pair(43,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta2_Endcaps_totalCaloIdLMWPMS2",         pair(pair(44,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta2_Endcaps_passEt",                   pair(pair(45,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta2_Endcaps_passWPTightEt35",            pair(pair(46,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta2_Endcaps_passCaloIdLMWPMS2",          pair(pair(47,n_bins_Ele32),pt_bins_Ele32)},

    {"h_pteta3_Barrel_totalEt",                   pair(pair(48,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta3_Barrel_totalWPTightEt35",            pair(pair(49,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta3_Barrel_totalCaloIdLMWPMS2",          pair(pair(50,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta3_Barrel_passEt",                    pair(pair(51,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta3_Barrel_passWPTightEt35",             pair(pair(52,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta3_Barrel_passCaloIdLMWPMS2",           pair(pair(53,n_bins_Ele32),pt_bins_Ele32)},

    {"h_pteta3_Endcaps_totalEt",                  pair(pair(54,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta3_Endcaps_totalWPTightEt35",           pair(pair(55,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta3_Endcaps_totalCaloIdLMWPMS2",         pair(pair(56,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta3_Endcaps_passEt",                   pair(pair(57,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta3_Endcaps_passWPTightEt35",            pair(pair(58,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pteta3_Endcaps_passCaloIdLMWPMS2",          pair(pair(59,n_bins_Ele32),pt_bins_Ele32)},

    {"h_mass_Barrel_totalCaloIdLMWPMS2",                   pair(pair(60,23),mass_bins)},
    {"h_mass_Barrel_passCaloIdLMWPMS2",                   pair(pair(61,23),mass_bins)},
    {"h_mass_Endcaps_totalCaloIdLMWPMS2",                   pair(pair(62,23),mass_bins)},
    {"h_mass_Endcaps_passCaloIdLMWPMS2",                   pair(pair(63,23),mass_bins)},

    {"h_pt_Barrel_totalHE",                     pair(pair(64,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Barrel_totalClusterShape",              pair(pair(65,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Barrel_totalPixelMatch",            pair(pair(66,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Barrel_passHE",                      pair(pair(67,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Barrel_passClusterShape",               pair(pair(68,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Barrel_passPixelMatch",             pair(pair(69,n_bins_Ele32),pt_bins_Ele32)},

    {"h_pt_Endcaps_totalHE",                    pair(pair(70,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Endcaps_totalClusterShape",             pair(pair(71,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Endcaps_totalPixelMatch",           pair(pair(72,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Endcaps_passHE",                     pair(pair(73,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Endcaps_passClusterShape",              pair(pair(74,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Endcaps_passPixelMatch",            pair(pair(75,n_bins_Ele32),pt_bins_Ele32)},

    {"h_pt_Barrel_totalHESeeded",                     pair(pair(76,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Barrel_totalClusterShapeSeeded",              pair(pair(77,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Barrel_totalPixelMatchSeeded",            pair(pair(78,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Barrel_totalCaloIdLMWPMS2Seeded",            pair(pair(79,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Barrel_passHESeeded",                      pair(pair(80,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Barrel_passClusterShapeSeeded",               pair(pair(81,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Barrel_passPixelMatchSeeded",             pair(pair(82,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Barrel_passCaloIdLMWPMS2Seeded",            pair(pair(83,n_bins_Ele32),pt_bins_Ele32)},

    {"h_pt_Endcaps_totalHESeeded",                    pair(pair(84,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Endcaps_totalClusterShapeSeeded",             pair(pair(85,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Endcaps_totalPixelMatchSeeded",           pair(pair(86,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Endcaps_totalCaloIdLMWPMS2Seeded",           pair(pair(87,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Endcaps_passHESeeded",                     pair(pair(88,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Endcaps_passClusterShapeSeeded",              pair(pair(89,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Endcaps_passPixelMatchSeeded",            pair(pair(90,n_bins_Ele32),pt_bins_Ele32)},
    {"h_pt_Endcaps_passCaloIdLMWPMS2Seeded",            pair(pair(91,n_bins_Ele32),pt_bins_Ele32)},
//photon histograms
    {"hPh_pt_Barrel_totalL1",                     pair(pair(92,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pt_Barrel_totalL1Et",                   pair(pair(93,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pt_Barrel_totalL1EtId",                 pair(pair(94,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pt_Barrel_passL1",                      pair(pair(95,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pt_Barrel_passL1Et",                    pair(pair(96,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pt_Barrel_passL1EtId",                  pair(pair(97,n_bins_Ph),pt_bins_Ph)},

    {"hPh_pt_Endcaps_totalL1",                    pair(pair(98,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pt_Endcaps_totalL1Et",                  pair(pair(99,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pt_Endcaps_totalL1EtId",                pair(pair(100,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pt_Endcaps_passL1",                     pair(pair(101,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pt_Endcaps_passL1Et",                   pair(pair(102,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pt_Endcaps_passL1EtId",                 pair(pair(103,n_bins_Ph),pt_bins_Ph)},

    {"hPh_eta_totalL1",                            pair(pair(104,19),eta_bins)},
    {"hPh_eta_totalL1Et",                          pair(pair(105,19),eta_bins)},
    {"hPh_eta_totalL1EtId",                        pair(pair(106,19),eta_bins)},
    {"hPh_eta_passL1",                             pair(pair(107,19),eta_bins)},
    {"hPh_eta_passL1Et",                           pair(pair(108,19),eta_bins)},
    {"hPh_eta_passL1EtId",                         pair(pair(109,19),eta_bins)},

    {"hPh_pteta1_Barrel_totalL1",                   pair(pair(110,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta1_Barrel_totalL1Et",                 pair(pair(111,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta1_Barrel_totalL1EtId",               pair(pair(112,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta1_Barrel_passL1",                    pair(pair(113,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta1_Barrel_passL1Et",                  pair(pair(114,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta1_Barrel_passL1EtId",                pair(pair(115,n_bins_Ph),pt_bins_Ph)},

    {"hPh_pteta1_Endcaps_totalL1",                  pair(pair(116,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta1_Endcaps_totalL1Et",                pair(pair(117,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta1_Endcaps_totalL1EtId",              pair(pair(118,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta1_Endcaps_passL1",                   pair(pair(119,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta1_Endcaps_passL1Et",                 pair(pair(120,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta1_Endcaps_passL1EtId",               pair(pair(121,n_bins_Ph),pt_bins_Ph)},

    {"hPh_pteta2_Barrel_totalL1",                   pair(pair(122,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta2_Barrel_totalL1Et",                 pair(pair(123,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta2_Barrel_totalL1EtId",               pair(pair(124,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta2_Barrel_passL1",                    pair(pair(125,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta2_Barrel_passL1Et",                  pair(pair(126,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta2_Barrel_passL1EtId",                pair(pair(127,n_bins_Ph),pt_bins_Ph)},

    {"hPh_pteta2_Endcaps_totalL1",                  pair(pair(128,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta2_Endcaps_totalL1Et",                pair(pair(129,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta2_Endcaps_totalL1EtId",              pair(pair(130,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta2_Endcaps_passL1",                 pair(pair(131,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta2_Endcaps_passL1Et",                 pair(pair(132,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta2_Endcaps_passL1EtId",               pair(pair(133,n_bins_Ph),pt_bins_Ph)},

    {"hPh_pteta3_Barrel_totalL1",                   pair(pair(134,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta3_Barrel_totalL1Et",                 pair(pair(135,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta3_Barrel_totalL1EtId",               pair(pair(136,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta3_Barrel_passL1",                    pair(pair(137,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta3_Barrel_passL1Et",                  pair(pair(138,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta3_Barrel_passL1EtId",                pair(pair(139,n_bins_Ph),pt_bins_Ph)},

    {"hPh_pteta3_Endcaps_totalL1",                  pair(pair(140,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta3_Endcaps_totalL1Et",                pair(pair(141,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta3_Endcaps_totalL1EtId",              pair(pair(142,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta3_Endcaps_passL1",                   pair(pair(143,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta3_Endcaps_passL1Et",                 pair(pair(144,n_bins_Ph),pt_bins_Ph)},
    {"hPh_pteta3_Endcaps_passL1EtId",               pair(pair(145,n_bins_Ph),pt_bins_Ph)},

  };
  vector<TH1F*> TriggerHists;
private:
  bool tagTriggerPass;
};

#endif

