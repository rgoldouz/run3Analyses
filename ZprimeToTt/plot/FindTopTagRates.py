import math
import gc
import sys
import ROOT
import numpy as np
import copy
import os
ROOT.gROOT.SetBatch(ROOT.kTRUE)
ROOT.gROOT.ProcessLine("gErrorIgnoreLevel = 1;")
ROOT.TH1.AddDirectory(ROOT.kFALSE)
ROOT.gStyle.SetOptStat(0)
import array
from ROOT import TColor
from ROOT import TGaxis
from ROOT import THStack
import gc
from copy import deepcopy
TGaxis.SetMaxDigits(2)
from my_utils import *

def eff(h,wp,name):
    total = h.Integral()           # Total integral (excluding under/overflow)
    target = wp * total
    cumulative = 0.0
    bin_num = h.GetNbinsX()
    for i in range(h.GetNbinsX(), 0, -1):
        cumulative += h.GetBinContent(i)
        if cumulative >= target:
            bin_num = i
            break
    
    print(f"Bin number: {bin_num}")
    print(f"Bin low edge: {h.GetBinLowEdge(bin_num)}")
    print(f"Bin center: {h.GetBinCenter(bin_num)}")
    print(f"Tail integral: {cumulative}")

nameyear='2024'
fileQcd = ROOT.TFile.Open(HistAddress + nameyear + '_EXT_TTto4Q.root')

tTagProbQcdAll = fileQcd.Get('tTagProbQcdAll')
tTagProbQcdB = fileQcd.Get('tTagProbQcdB')
tTagProbQcdNonB = fileQcd.Get('tTagProbQcdNonB')

fileTT = ROOT.TFile.Open(HistAddress + nameyear + '_ttbar2l.root')
tTagProbTop = fileTT.Get('tTagProbTop')

tTagProbQcdAll.Scale(1/tTagProbQcdAll.Integral())
tTagProbQcdB.Scale(1/tTagProbQcdB.Integral())
tTagProbQcdNonB.Scale(1/tTagProbQcdNonB.Integral())
tTagProbTop.Scale(1/tTagProbTop.Integral())

comparePlot([tTagProbQcdAll,tTagProbQcdB,tTagProbQcdNonB,tTagProbTop],["Ak8 jet (u,d,c,s,b,g)","Ak8 jet (b)","Ak8 jet (u,d,c,s,g)","Ak8 jet (lb)"], "sample", "partT tagger")

eff(tTagProbQcdB,0.01,"1%")
print(f"top tagging efficiency: {tTagProbTop.Integral(89,110)}")
