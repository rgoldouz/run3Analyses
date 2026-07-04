import sys
import os
import subprocess
import readline
import string
import glob
from joblib import Parallel, delayed
sys.path.append('/groups/mhildret/rgoldouz/run3Analyses/ZprimeToTt/bin/')
import Files_ULall_nano

def f(name):
    if 'ANoutput' not in name:
        print(name)
    os.system(name)

if __name__ == '__main__':
    Fhadd=[]
    SAMPLES = {}
    SAMPLES.update(Files_ULall_nano.UL17)
    
    
    addedFilesData = {"2016preVFP": [],"2016postVFP": [], "2017": [], "2018": [],"2024": []} 
    addedFilesMcDY = {"2016preVFP": [],"2016postVFP": [], "2017": [], "2018": [],"2024": []}
    addedFilesMcttbar0l = {"2016preVFP": [],"2016postVFP": [], "2017": [], "2018": [],"2024": []}
    addedFilesMcttbar1l = {"2016preVFP": [],"2016postVFP": [], "2017": [], "2018": [],"2024": []}
    addedFilesMcttbar2l = {"2016preVFP": [],"2016postVFP": [], "2017": [], "2018": [],"2024": []}
    addedFilesMcDiboson = {"2016preVFP": [],"2016postVFP": [], "2017": [], "2018": [],"2024": []}
    addedFilesMcST = {"2016preVFP": [],"2016postVFP": [], "2017": [], "2018": [],"2024": []}
    addedFilesMcWjets = {"2016preVFP": [],"2016postVFP": [], "2017": [], "2018": [],"2024": []}
    
    os.system('rm *.root')
    dist = "/cms/cephfs/data/store/user/rgoldouz/FullProduction/AnalysisZprimeTT/Analysis_" 
    
    for keyUL, value in SAMPLES.items():
        if "UL24" not in keyUL:
            continue
        key = keyUL
        key = keyUL.replace("UL24", "2024")
        hadd='hadd ' + key + '.root '
#        if not os.path.isdir(dist + keyUL):
#            print dist + keyUL+' does not exist'
#            continue
        for filename in os.listdir(dist + keyUL):
            hadd += dist + keyUL + '/' + filename + ' '
        Fhadd.append(hadd)
    Parallel(n_jobs=40)(delayed(f)(i) for i in Fhadd)
    #    os.system(hadd)
    for keyUL, value in SAMPLES.items():
        key = keyUL
        key = keyUL.replace("UL24", "2024")
        year='2024'
#        year = value[3]
        print (key,year)
        if 'EXT' in key:
            continue
        if value[1]=='data':
            addedFilesData[year].append(key + '.root')
#        elif 'FCNC' in key:
#            continue
        elif 'DY_' in key:
            addedFilesMcDY[year].append( key + '.root')
        elif 'TT_' in key and 'sys' not in key:
            if "TTTo2L2Nu" in key:
                addedFilesMcttbar2l[year].append( key + '.root')
            elif "TTToSemiLeptonic" in key:
                addedFilesMcttbar1l[year].append( key + '.root')
            else:
                addedFilesMcttbar0l[year].append( key + '.root')
        elif 'ST_' in key:
            addedFilesMcST[year].append( key + '.root')
        elif 'VV_' in key:
            addedFilesMcDiboson[year].append( key + '.root')  
        elif 'Wjet_' in key:
            addedFilesMcWjets[year].append( key + '.root')            

for key, value in addedFilesData.items():
        print(key,value)
        if key != '2024':
            continue
        Fmerged=[]
        hadddata = 'hadd -f ' +key+'_data.root ' + ' '.join(addedFilesData[key])
        haddmcDY ='hadd -f ' +key+'_DY.root ' + ' '.join(addedFilesMcDY[key])
        haddmcttbar0l ='hadd -f ' +key+'_ttbar0l.root ' + ' '.join(addedFilesMcttbar0l[key])
        haddmcttbar1l ='hadd -f ' +key+'_ttbar1l.root ' + ' '.join(addedFilesMcttbar1l[key])
        haddmcttbar2l ='hadd -f ' +key+'_ttbar2l.root ' + ' '.join(addedFilesMcttbar2l[key])
        haddmcDiboson ='hadd -f ' +key+'_Diboson.root ' + ' '.join(addedFilesMcDiboson[key])
        haddmcST ='hadd -f ' +key+'_ST.root ' + ' '.join(addedFilesMcST[key])
        haddmcWjets ='hadd -f ' +key+'_Wjets.root ' + ' '.join(addedFilesMcWjets[key])
        Fmerged.append(hadddata)
        Fmerged.append(haddmcDY)
        Fmerged.append(haddmcttbar0l)
        Fmerged.append(haddmcttbar1l)
        Fmerged.append(haddmcttbar2l)
        Fmerged.append(haddmcDiboson)
        Fmerged.append(haddmcWjets)
        Fmerged.append(haddmcST)
        Parallel(n_jobs=10)(delayed(f)(i) for i in Fmerged)
#        os.system('hadd ' +key+'_totalBG.root '+key+'_DY.root ' +key+'_Else.root ' +key+'_ttbar.root ' +key+'_Diboson.root ' +key+'_Triboson.root '+key+'_ST.root ' +key+'_TTZ.root ' +key+'_TTW.root '+key+'_TTH.root '+key+'_Conv.root ')        
#        print '\n \n \n hadddata:' + hadddata + '\n haddmcDY:' + haddmcDY + '\n haddmcElse:' + haddmcElse  + '\n haddmcttbar:' + haddmcttbar + '\n haddmcDiboson:' + haddmcDiboson + '\n haddmcTriboson:' + haddmcTriboson + '\n haddmcTTH:' + haddmcTTH+ '\n haddmcTTW:' + haddmcTTW+ '\n haddmcTTZ:' + haddmcTTZ + '\n haddmcST:' + haddmcST + '\n haddmcConv:' + haddmcConv + '\n haddmcFCNCTCdecay:' + haddmcFCNCTCdecay + '\n haddmcFCNCTCprod:' + haddmcFCNCTCprod + '\n haddmcFCNCTUdecay:' + haddmcFCNCTUdecay + '\n haddmcFCNCTUprod' + haddmcFCNCTUprod

