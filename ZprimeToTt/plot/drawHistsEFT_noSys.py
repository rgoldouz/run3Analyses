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

#year=['2017']
#channels=["2lssEE", "2lssEM","2lssMM", "2losEE_Weighted", "2losEM_Weighted", "2los_EpEm_CR", "2los_MUpMUm_CR", "2los_EpmMUmp_CR", "3lonZ", "3loffZhigh", "3loffZlow","4l_CR", "2lssOnZ", "2losOnZ_Weighted"]
#channelsFake=["2lssEE_LF", "2lssEE_FF", "2lssEM_LF", "2lssEM_FF","2lssMM_LF", "2lssMM_FF", "3lonZ_LLF", "3lonZ_LFF","3lonZ_FFF","3loffZhigh_LLF", "3loffZhigh_LFF","3loffZhigh_FFF", "3loffZlow_LLF", "3loffZlow_LFF","3loffZlow_FFF", "2lssOnZ_LF", "2lssOnZ_FF"]
#variables=["llM"]

Hists = {}
HistsCFrate = {}
HistsFake = {}

for nameyear in year:
    Hists[nameyear] = {}
    HistsCFrate[nameyear] = {}
    for f, sample in enumerate(Samples):
        sample_key = sample
        Hists[nameyear][sample_key] = {}
        file = ROOT.TFile.Open(HistAddress + nameyear + '_' + sample)
#        HistsCFrate[nameyear][sample_key] = file.Get('mll_SS_Zwindow_0jet')
        for ch in channels:
            for reg in regions:
                for var in variables:
                    hist_key = "{}_{}_{}".format(ch,reg,var)
                    #print (hist_key)
                    h = file.Get(hist_key)
                    h = EFTtoNormal(h, wc1)
                    #print(sample,hist_key,h.Integral())
                    # Apply binning if needed
                    if 'MVA' in var:
                        for key in binsDic:
                            if key in ch:
                                bins = binsDic[key]
                                h = h.Rebin(len(bins) - 1, "", bins)
                                break
                    for b in range(h.GetNbinsX()):
                        if h.GetBinContent(b+1)<0:
                            h.SetBinContent(b+1, 0)
                            h.SetBinError(b+1, 0)
                    h.SetFillColor(colors[f])
                    h.SetLineColor(colors[f])

                    # Store nominal histogram
                    Hists[nameyear][sample_key][hist_key] = h.Clone()


for numyear, nameyear in enumerate(year):
    for numch, namech in enumerate(channels):
        for numreg, namereg in enumerate(regions):
            for numvar, namevar in enumerate(variables):
                hist_key = "{}_{}_{}".format(namech,namereg,namevar)
                HH=[]
                HHsignal=[]
                SN=[]
                SNsignal=[]
                for f in range(len(Samples)):
                    if 'Signal' in Samples[f]:
                        Hists[nameyear][Samples[f]][hist_key].SetLineColor(1)
                        Hists[nameyear][Samples[f]][hist_key].Scale(1000)
                        HHsignal.append(Hists[nameyear][Samples[f]][hist_key])
                        SNsignal.append(SamplesName[f])
                    else:
                        HH.append(Hists[nameyear][Samples[f]][hist_key])
                        SN.append(SamplesName[f])
                    #index = SN.index("DY")
                    #if 'Run2'==nameyear:
                    #    HH[index].Scale((HistsCFrate[year[0]][Samples[0]].Integral()+HistsCFrate[year[1]][Samples[0]].Integral()+HistsCFrate[year[2]][Samples[0]].Integral()+HistsCFrate[year[3]][Samples[0]].Integral())/(HistsCFrate[year[0]]['DY.root'].Integral()+HistsCFrate[year[1]]['DY.root'].Integral()+HistsCFrate[year[2]]['DY.root'].Integral()+HistsCFrate[year[3]]['DY.root'].Integral()))
                    #else:
                    #    HH[index].Scale(HistsCFrate[nameyear][Samples[0]].Integral()/HistsCFrate[nameyear]['DY.root'].Integral())
                   # SN[index]="ChargeFlip"
                stackPlotsNoSys(HH, HHsignal, SN, SNsignal, 'noSysPlots',namech, namereg, nameyear,namevar,variablesName[numvar])
os.system('tar -cvf noSysPlots.tar noSysPlots')

###for numyear, nameyear in enumerate(year):
###    for numch, namech in enumerate(channels):
###        for numreg, namereg in enumerate(regions):
###            for numvar, namevar in enumerate(variables):
###                hist_key = "{}_{}_{}".format(namech,namereg,namevar)
###                HH=[]
###                HHsignal=[]
###                SN=[]
###                SNsignal=[]
###                for f in range(len(Samples)):
###                    if Hists[nameyear][Samples[f]][hist_key].Integral()<0.1:
###                        continue
###                    if 'data' in Samples[f]:
###                        Hists[nameyear][Samples[f]][hist_key].Scale(1/Hists[nameyear][Samples[f]][hist_key].Integral())
###                        HH.append(Hists[nameyear][Samples[f]][hist_key])
###                        SN.append(SamplesName[f])
###                    if 'DY' in Samples[f] or 'ttbar2l' in Samples[f]:
###                        Hists[nameyear][Samples[f]][hist_key].Scale(0.5/Hists[nameyear][Samples[f]][hist_key].Integral())
###                        HH.append(Hists[nameyear][Samples[f]][hist_key])
###                        SN.append(SamplesName[f])                       
###                print(SN,namech,namereg,namevar)        
###                stackPlotsNoSys(HH, HHsignal, SN, SNsignal, 'noSysPlotsNormilised',namech, namereg, nameyear,namevar,variablesName[numvar])
###os.system('tar -cvf noSysPlotsNormilised.tar noSysPlotsNormilised')

le = '\\documentclass{article}' + "\n"
le += '\\usepackage{rotating}' + "\n"
le += '\\usepackage{rotating}' + "\n"
le += '\\begin{document}' + "\n"

#    for numch, namech in enumerate(channels):
#        cutFlowTable(Hists, SamplesNameLatex, regionsName, numch, numyear, nameyear + ' ' + namech, 6 )
#print '\\end{document}' + "\n"


