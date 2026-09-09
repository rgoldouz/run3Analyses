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
import ctypes

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
file = ROOT.TFile.Open(HistAddress + nameyear + '_topTagBG.root')

#tTagProbQcdAll = fileQcd.Get('tTagProbQcdAll')
tTagProbQcdB = file.Get('tTag_noL_B')
tTagProbQcdNonB = file.Get('tTag_noL_nonB')
tTagProbQcdB.Scale(1/tTagProbQcdB.Integral())
tTagProbQcdNonB.Scale(1/tTagProbQcdNonB.Integral())

EtTag_LfromTop_B = file.Get('EtTag_LfromTop_B')
EtTag_LfromTop_nonB = file.Get('EtTag_LfromTop_nonB')
EtTag_LfromNonTop_B = file.Get('EtTag_LfromNonTop_B')
EtTag_LfromNonTop_nonB = file.Get('EtTag_LfromNonTop_nonB')
EtTag_LnonPrompt_B = file.Get('EtTag_LnonPrompt_B')
EtTag_LnonPrompt_nonB = file.Get('EtTag_LnonPrompt_nonB')

EtTag_LfromTop_B.Scale(1/EtTag_LfromTop_B.Integral())
EtTag_LfromTop_nonB.Scale(1/EtTag_LfromTop_nonB.Integral())
EtTag_LfromNonTop_B.Scale(1/EtTag_LfromNonTop_B.Integral())
EtTag_LfromNonTop_nonB.Scale(1/EtTag_LfromNonTop_nonB.Integral())
EtTag_LnonPrompt_B.Scale(1/EtTag_LnonPrompt_B.Integral())
EtTag_LnonPrompt_nonB.Scale(1/EtTag_LnonPrompt_nonB.Integral())

MtTag_LfromTop_B = file.Get('MtTag_LfromTop_B')
MtTag_LfromTop_nonB = file.Get('MtTag_LfromTop_nonB')
MtTag_LfromNonTop_B = file.Get('MtTag_LfromNonTop_B')
MtTag_LfromNonTop_nonB = file.Get('MtTag_LfromNonTop_nonB')
MtTag_LnonPrompt_B = file.Get('MtTag_LnonPrompt_B')
MtTag_LnonPrompt_nonB = file.Get('MtTag_LnonPrompt_nonB')

MtTag_LfromTop_B.Scale(1/MtTag_LfromTop_B.Integral())
MtTag_LfromTop_nonB.Scale(1/MtTag_LfromTop_nonB.Integral())
MtTag_LfromNonTop_B.Scale(1/MtTag_LfromNonTop_B.Integral())
MtTag_LfromNonTop_nonB.Scale(1/MtTag_LfromNonTop_nonB.Integral())
MtTag_LnonPrompt_B.Scale(1/MtTag_LnonPrompt_B.Integral())
MtTag_LnonPrompt_nonB.Scale(1/MtTag_LnonPrompt_nonB.Integral())

EtTagMVA_LfromTop_B = file.Get('EtTagMVA_LfromTop_B')
EtTagMVA_LfromTop_nonB = file.Get('EtTagMVA_LfromTop_nonB')
EtTagMVA_LfromNonTop_B = file.Get('EtTagMVA_LfromNonTop_B')
EtTagMVA_LfromNonTop_nonB = file.Get('EtTagMVA_LfromNonTop_nonB')
EtTagMVA_LnonPrompt_B = file.Get('EtTagMVA_LnonPrompt_B')
EtTagMVA_LnonPrompt_nonB = file.Get('EtTagMVA_LnonPrompt_nonB')

EtTagMVA_LfromTop_B.Scale(1/EtTagMVA_LfromTop_B.Integral())
EtTagMVA_LfromTop_nonB.Scale(1/EtTagMVA_LfromTop_nonB.Integral())
EtTagMVA_LfromNonTop_B.Scale(1/EtTagMVA_LfromNonTop_B.Integral())
EtTagMVA_LfromNonTop_nonB.Scale(1/EtTagMVA_LfromNonTop_nonB.Integral())
EtTagMVA_LnonPrompt_B.Scale(1/EtTagMVA_LnonPrompt_B.Integral())
EtTagMVA_LnonPrompt_nonB.Scale(1/EtTagMVA_LnonPrompt_nonB.Integral())

MtTagMVA_LfromTop_B = file.Get('MtTagMVA_LfromTop_B')
MtTagMVA_LfromTop_nonB = file.Get('MtTagMVA_LfromTop_nonB')
MtTagMVA_LfromNonTop_B = file.Get('MtTagMVA_LfromNonTop_B')
MtTagMVA_LfromNonTop_nonB = file.Get('MtTagMVA_LfromNonTop_nonB')
MtTagMVA_LnonPrompt_B = file.Get('MtTagMVA_LnonPrompt_B')
MtTagMVA_LnonPrompt_nonB = file.Get('MtTagMVA_LnonPrompt_nonB')

MtTagMVA_LfromTop_B.Scale(1/MtTagMVA_LfromTop_B.Integral())
MtTagMVA_LfromTop_nonB.Scale(1/MtTagMVA_LfromTop_nonB.Integral())
MtTagMVA_LfromNonTop_B.Scale(1/MtTagMVA_LfromNonTop_B.Integral())
MtTagMVA_LfromNonTop_nonB.Scale(1/MtTagMVA_LfromNonTop_nonB.Integral())
MtTagMVA_LnonPrompt_B.Scale(1/MtTagMVA_LnonPrompt_B.Integral())
MtTagMVA_LnonPrompt_nonB.Scale(1/MtTagMVA_LnonPrompt_nonB.Integral())

comparePlot([EtTag_LnonPrompt_nonB, EtTag_LnonPrompt_B, EtTag_LfromNonTop_nonB, EtTag_LfromNonTop_B, EtTag_LfromTop_nonB, EtTag_LfromTop_B],["Ak8 jet [nonprompt-e, non-b]","Ak8 jet [nonprompt-e, b]","Ak8 jet [non-top prompt-e, non-b]", "Ak8 jet [non-top prompt-e, b]", "Ak8 jet [top prompt-e, non-b]", "Ak8 jet [top prompt-e, b]"], "partTele", "partT top tagger")
comparePlot([MtTag_LnonPrompt_nonB, MtTag_LnonPrompt_B, MtTag_LfromNonTop_nonB, MtTag_LfromNonTop_B, MtTag_LfromTop_nonB, MtTag_LfromTop_B],["Ak8 jet [nonprompt-#mu, non-b]","Ak8 jet [nonprompt-#mu, b]","Ak8 jet [non-top prompt-#mu, non-b]", "Ak8 jet [non-top prompt-#mu, b]", "Ak8 jet [top prompt-#mu, non-b]", "Ak8 jet [top prompt-#mu, b]"], "partTmuon", "partT top tagger")
comparePlot([EtTagMVA_LnonPrompt_nonB, EtTagMVA_LnonPrompt_B, EtTagMVA_LfromNonTop_nonB, EtTagMVA_LfromNonTop_B, EtTagMVA_LfromTop_nonB, EtTagMVA_LfromTop_B],["Ak8 jet [nonprompt-e, non-b]","Ak8 jet [nonprompt-e, b]","Ak8 jet [non-top prompt-e, non-b]", "Ak8 jet [non-top prompt-e, b]", "Ak8 jet [top prompt-e, non-b]", "Ak8 jet [top prompt-e, b]"], "MVAele", "BDT top tagger")
comparePlot([MtTagMVA_LnonPrompt_nonB, MtTagMVA_LnonPrompt_B, MtTagMVA_LfromNonTop_nonB, MtTagMVA_LfromNonTop_B, MtTagMVA_LfromTop_nonB, MtTagMVA_LfromTop_B],["Ak8 jet [nonprompt-#mu, non-b]","Ak8 jet [nonprompt-#mu, b]","Ak8 jet [non-top prompt-#mu, non-b]", "Ak8 jet [non-top prompt-#mu, b]", "Ak8 jet [top prompt-#mu, non-b]", "Ak8 jet [top prompt-#mu, b]"], "MVAmuon", "BDT top tagger")


eff(MtTagMVA_LnonPrompt_nonB,0.01,"1%")
print(f"top tagging efficiency (for MtTagMVA_LnonPrompt_nonB): {MtTagMVA_LnonPrompt_nonB.Integral(70,110)}")
print(f"top tagging efficiency (for MtTagMVA_LnonPrompt_B): {MtTagMVA_LnonPrompt_B.Integral(70,110)}")
print(f"top tagging efficiency (for MtTagMVA_LfromNonTop_nonB): {MtTagMVA_LfromNonTop_nonB.Integral(70,110)}")
print(f"top tagging efficiency (for MtTagMVA_LfromNonTop_B): {MtTagMVA_LfromNonTop_B.Integral(70,110)}")
print(f"top tagging efficiency (for MtTagMVA_LfromTop_nonB): {MtTagMVA_LfromTop_nonB.Integral(70,110)}")
print(f"top tagging efficiency (for MtTagMVA_LfromTop_B): {MtTagMVA_LfromTop_B.Integral(70,110)}")
#eff(tTagProbLep,0.1,"10%")
eff(EtTagMVA_LnonPrompt_nonB,0.01,"1%")
print(f"top tagging efficiency (for EtTagMVA_LnonPrompt_nonB): {EtTagMVA_LnonPrompt_nonB.Integral(88,110)}")
print(f"top tagging efficiency (for EtTagMVA_LnonPrompt_B): {EtTagMVA_LnonPrompt_B.Integral(88,110)}")
print(f"top tagging efficiency (for EtTagMVA_LfromNonTop_nonB): {EtTagMVA_LfromNonTop_nonB.Integral(88,110)}")
print(f"top tagging efficiency (for EtTagMVA_LfromNonTop_B): {EtTagMVA_LfromNonTop_B.Integral(88,110)}")
print(f"top tagging efficiency (for EtTagMVA_LfromTop_nonB): {EtTagMVA_LfromTop_nonB.Integral(88,110)}")
print(f"top tagging efficiency (for EtTagMVA_LfromTop_B): {EtTagMVA_LfromTop_B.Integral(88,110)}")

cats=["lPb","lPnonb","nonlPb"]
channels=["E2l_eeInZ_0b","M2l_mmInZ_0b","E2l_em_1b","M2l_em_1b"]
regions=["all","pass","fail"]
regionsName=["all","pass","fail"]
variables=["jetPt","jetMass","jetEta","jetPhi","jetLepFrac","lepPtOjetPt","jetMassPtbin1","jetMassPtbin2","jetMassPtbin3","jetMassPtbin4"]
variablesName=["jetPt","jetMass","jetEta","jetPhi","jetLepFrac","lepPtOjetPt","jetMassPtbin1","jetMassPtbin2","jetMassPtbin3","jetMassPtbin4"]

Samples = ['data.root','ttbar2l.root','ttbar1l.root','ST.root','TTG.root']
SamplesName = ['Data','t#bar{t} (2l)', 't#bar{t} (1l)','Single top','TTG']

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
                    for cat in cats:
                        if f==0 and cat!="lPnonb":
                            continue
                        hist_key = "{}_{}_{}_{}".format(cat,ch,reg,var)
                        #print (hist_key)
                        h = file.Get('topTagSF/'+hist_key)
                        h.SetFillColor(colors[f])
                        h.SetLineColor(colors[f])
                        # Store nominal histogram
                        Hists[nameyear][sample_key][hist_key] = h.Clone()

for numyear, nameyear in enumerate(year):
    for numch, namech in enumerate(channels):
        for numreg, namereg in enumerate(regions):
            for numvar, namevar in enumerate(variables):
                hist_keylPb = "{}_{}_{}_{}".format("lPb",namech,namereg,namevar)
                hist_keylPnonb = "{}_{}_{}_{}".format("lPnonb",namech,namereg,namevar)
                hist_keynonlPb = "{}_{}_{}_{}".format("nonlPb",namech,namereg,namevar)
                HH=[]
                HHsignal=[]
                SN=[]
                SNsignal=[]
                for f in range(len(Samples)):
                    if 'data' in Samples[f]:
                        HH.append(Hists[nameyear][Samples[f]][hist_keylPnonb])
                        SN.append(SamplesName[f])
                    elif 'Signal' in Samples[f]:
                        Hists[nameyear][Samples[f]][hist_key].SetLineColor(1)
                        Hists[nameyear][Samples[f]][hist_key].Scale(1000)
                        HHsignal.append(Hists[nameyear][Samples[f]][hist_key])
                        SNsignal.append(SamplesName[f])
                    else:
                        Hists[nameyear][Samples[f]][hist_keylPb].SetFillColor(f+1)
                        Hists[nameyear][Samples[f]][hist_keylPb].SetLineColor(f+1)
                        Hists[nameyear][Samples[f]][hist_keynonlPb].SetFillColor(2*f+10)
                        Hists[nameyear][Samples[f]][hist_keynonlPb].SetLineColor(2*f+10)
                        HH.append(Hists[nameyear][Samples[f]][hist_keylPb])
                        SN.append(SamplesName[f] + "(lPb)")
                        HH.append(Hists[nameyear][Samples[f]][hist_keylPnonb])
                        SN.append(SamplesName[f] + "(lPnonb)")
                        HH.append(Hists[nameyear][Samples[f]][hist_keynonlPb])
                        SN.append(SamplesName[f] + "(nonlPb)")
                stackPlotsNoSys(HH, HHsignal, SN, SNsignal, 'topTagSF',namech, namereg, nameyear,namevar,variablesName[numvar])
os.system('tar -cvf topTagSF.tar  topTagSF')


for numyear, nameyear in enumerate(year):
    for numch, namech in enumerate(channels):
        if "InZ" not in namech:
            continue
        for numvar, namevar in enumerate(variables):
            hist_num = "{}_{}_{}_{}".format("lPnonb",namech,"pass",namevar)
            hist_den = "{}_{}_{}_{}".format("lPnonb",namech,"all",namevar)
            hist_num_sub = "{}_{}_{}_{}".format("lPb",namech,"pass",namevar)
            hist_den_sub = "{}_{}_{}_{}".format("lPb",namech,"all",namevar)
            H1Den=Hists[nameyear][Samples[0]][hist_den]
            H1Nom=Hists[nameyear][Samples[0]][hist_num]
            H1Den_sub1=Hists[nameyear][Samples[Samples.index('ttbar2l.root')]][hist_den_sub]
            H1Nom_sub1=Hists[nameyear][Samples[Samples.index('ttbar2l.root')]][hist_num_sub]
            H1Den_sub2=Hists[nameyear][Samples[Samples.index('ttbar1l.root')]][hist_den_sub]
            H1Nom_sub2=Hists[nameyear][Samples[Samples.index('ttbar1l.root')]][hist_num_sub]
            H1Den_sub3=Hists[nameyear][Samples[Samples.index('ST.root')]][hist_den_sub]
            H1Nom_sub3=Hists[nameyear][Samples[Samples.index('ST.root')]][hist_num_sub]            
            H1Nom.Add(H1Nom_sub1,-1)
            H1Den.Add(H1Den_sub1,-1)
            H1Nom.Add(H1Nom_sub2,-1)
            H1Den.Add(H1Den_sub2,-1)
            H1Nom.Add(H1Nom_sub3,-1)
            H1Den.Add(H1Den_sub3,-1)            
            comparePlotWithRatio([H1Den,H1Nom],["all", "pass"],namech+"_"+namevar, namevar)
SFhists=[]
Shapes=[]
ShapesName=[]
for numyear, nameyear in enumerate(year):
    for numch, namech in enumerate(channels):
        if "InZ" in namech:
            continue
        for numvar, namevar in enumerate(variables):           
            SFhists=[]
            Shapes=[]
            SN=[]
            hist_num = "{}_{}_{}_{}".format("lPnonb",namech,"pass",namevar)
            hist_den = "{}_{}_{}_{}".format("lPnonb",namech,"all",namevar)
            hist_num_sub = "{}_{}_{}_{}".format("lPnonb",namech,"pass",namevar)
            hist_den_sub = "{}_{}_{}_{}".format("lPnonb",namech,"all",namevar)
            S1=Hists[nameyear][Samples[Samples.index('ttbar2l.root')]]["{}_{}_{}_{}".format("lPb",namech,"fail",namevar)].Clone()
            S2=Hists[nameyear][Samples[Samples.index('ttbar2l.root')]]["{}_{}_{}_{}".format("lPnonb",namech,"fail",namevar)].Clone()
            S3=Hists[nameyear][Samples[Samples.index('ttbar1l.root')]]["{}_{}_{}_{}".format("lPb",namech,"fail",namevar)].Clone()
            S4=Hists[nameyear][Samples[Samples.index('ttbar1l.root')]]["{}_{}_{}_{}".format("lPnonb",namech,"fail",namevar)].Clone()
            hData=Hists[nameyear][Samples[Samples.index('data.root')]]["{}_{}_{}_{}".format("lPnonb",namech,"fail",namevar)].Clone()
            hData.Add(S1,-1)
            hData.Add(S3,-1)
            mc = ROOT.TObjArray()
            mc.Add(S2)
            mc.Add(S4)
            SN.append("tt(2l)" + "(lPnonb)")
            SN.append("tt(1l)" + "(lPnonb)")
            if "jetMass" in namevar:
                fit = ROOT.TFractionFitter(hData, mc)
                status = fit.Fit()
                if status == 0:
                    print("Fit converged.\n")
                    dataIntegral = hData.Integral()
                    for i, h in enumerate([S2,S4]):
                        frac = ctypes.c_double()
                        err  = ctypes.c_double()
                    
                        fit.GetResult(i, frac, err)
                    
                        fitted_yield = frac.value * dataIntegral
                        scale_factor = fitted_yield / h.Integral()
                    
                        print(f"{namech},{namevar}:BG{i}-{SN[i]}")
                        print(f"  Fraction     = {frac.value:.6f} ± {err.value:.6f}")
                        print(f"  Yield        = {fitted_yield:.1f}")
                        print(f"  Scale factor = {scale_factor:.3f}")                    
                else:
                    print("Fit failed with status =", status)

            if(S1.Integral()>0):
                S1.Scale(1/S1.Integral())
            if(S2.Integral()>0):    
                S2.Scale(1/S2.Integral())
            if(S3.Integral()>0):
                S3.Scale(1/S3.Integral())
            if(S4.Integral()>0):
                S4.Scale(1/S4.Integral())
            Shapes.append(S1)
            ShapesName.append("tt(2l)" + "(lPb)")
            Shapes.append(S2)
            ShapesName.append("tt(2l)" + "(lPnonb)")
            Shapes.append(S3)
            ShapesName.append("tt(1l)" + "(lPb)")            
            Shapes.append(S4)
            ShapesName.append("tt(1l)" + "(lPnonb)")            

            H1Den=Hists[nameyear][Samples[0]][hist_den]
            H1Nom=Hists[nameyear][Samples[0]][hist_num]
            H1Den_sub1=Hists[nameyear][Samples[Samples.index('ttbar1l.root')]][hist_den_sub]
            H1Nom_sub1=Hists[nameyear][Samples[Samples.index('ttbar1l.root')]][hist_num_sub]
            H1Nom.Add(H1Nom_sub1,-1)
            H1Den.Add(H1Den_sub1,-1)
            H1Den_sub2=Hists[nameyear][Samples[Samples.index('ttbar2l.root')]][hist_den_sub]
            H1Nom_sub2=Hists[nameyear][Samples[Samples.index('ttbar2l.root')]][hist_num_sub]
            H1Nom.Add(H1Nom_sub2,-1)
            H1Den.Add(H1Den_sub2,-1)           
            H1Den_sub3=Hists[nameyear][Samples[Samples.index('ST.root')]][hist_den_sub]
            H1Nom_sub3=Hists[nameyear][Samples[Samples.index('ST.root')]][hist_num_sub]
            H1Nom.Add(H1Nom_sub3,-1)
            H1Den.Add(H1Den_sub3,-1)            
            H1Nom.Divide(H1Den)
            hist_num_mc = "{}_{}_{}_{}".format("lPb",namech,"pass",namevar)
            hist_den_mc = "{}_{}_{}_{}".format("lPb",namech,"all",namevar)
            H1Den_mc=Hists[nameyear][Samples[Samples.index('ttbar2l.root')]][hist_den_mc]
            H1Nom_mc=Hists[nameyear][Samples[Samples.index('ttbar2l.root')]][hist_num_mc]
            H1Nom_mc.Divide(H1Den_mc)
            SFh=H1Nom.Clone()
            SFh.Divide(H1Nom_mc)
            SFh.SetName(nameyear+"_"+namech+"_"+namevar)
            SFhists.append(SFh)
            comparePlotWithRatio([H1Nom_mc,H1Nom],["MC", "Data"],namech+"_"+namevar, namevar)            
            comparePlot(Shapes,ShapesName,"shapesFail_"+namech+"_"+namevar, namevar)
myfile = ROOT.TFile( 'topTagSF.root', 'RECREATE')
for H in SFhists:
    H.Write()
myfile.Close()
###for numyear, nameyear in enumerate(year):
###    for numvar, namevar in enumerate(variablesTtag):
###        HH=[]
###        HHsignal=[]
###        SN=[]
###        SNsignal=[]
###        for f in range(len(Samples)):
###            HH.append(Hists[nameyear][Samples[f]][namevar])
###            SN.append(SamplesName[f])
###        stackPlotsNoSys(HH, HHsignal, SN, SNsignal, 'topTagPlots','', '', '',namevar,variablesTtagName[numvar])
###os.system('tar -cvf topTagPlots.tar topTagPlots')
###
###for var in variablesTtagName:
###    H1Den=Hists[nameyear][Samples[0]][f"tTag_ee_{var}_Den"]
###    H1Nom=Hists[nameyear][Samples[0]][f"tTag_ee_{var}_Num"]
###    pEff = ROOT.TEfficiency(H1Nom,H1Den);
###    comparePlot([pEff],[f"ee_{var}"], f"ee_{var}", var)
###
###    H1Den=Hists[nameyear][Samples[0]][f"tTag_mm_{var}_Den"]
###    H1Nom=Hists[nameyear][Samples[0]][f"tTag_mm_{var}_Num"]
###    pEff = ROOT.TEfficiency(H1Nom,H1Den);
###    comparePlot([pEff],[f"mm_{var}"], f"mm_{var}", var)    
###
