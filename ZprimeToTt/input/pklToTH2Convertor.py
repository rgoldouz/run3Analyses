import pickle
import types
import sys
import gzip
import numpy as np
import ROOT

# Create fake coffea.hist module
hist_module = types.ModuleType('hist')
sys.modules['coffea.hist'] = hist_module
sys.modules['coffea.hist.hist_tools'] = hist_module

# Define dummy classes that match pickled names
class Hist:
    pass

class Bin:
    pass

hist_module.Hist = Hist
hist_module.Bin = Bin
chF=['flip_probs_topcoffea_UL16.pkl.gz',
'flip_probs_topcoffea_UL16APV.pkl.gz',
'flip_probs_topcoffea_UL17.pkl.gz',
'flip_probs_topcoffea_UL18.pkl.gz']
# Load pickle
for ch in chF:
    print(ch.split('.')[0])
    with gzip.open(ch, "rb") as f:
        data = pickle.load(f)
    
    print(data)  # inspect contents
    
    # Extract values
    values = data._sumw[()]
    nx, ny = data._dense_shape
    
    # --- Step 3: Load reference histogram from ROOT file ---
    ref_file = ROOT.TFile.Open("fliprates_frommc_UL18_recorrected.root")
    ref_hist = ref_file.Get("chargeMisId")
    
    if not ref_hist:
        raise RuntimeError("Could not find 'chargeMisId' in fliprates_frommc_UL18_recorrected.root")
    
    # Extract bin edges from reference histogram
    pt_edges = np.array([ref_hist.GetXaxis().GetBinLowEdge(i) for i in range(1, ref_hist.GetNbinsX() + 2)], dtype=float)
    eta_edges = np.array([ref_hist.GetYaxis().GetBinLowEdge(i) for i in range(1, ref_hist.GetNbinsY() + 2)], dtype=float)
    
    print(f"pt_edges: {pt_edges}")
    print(f"eta_edges: {eta_edges}")
    
    # --- Step 4: Create a new TH2F with exact same binning ---
    h = ROOT.TH2F("chargeMisID", "Charge MisID rate",
                  len(pt_edges) - 1, pt_edges,
                  len(eta_edges) - 1, eta_edges)
    ref_file.Close()
    
    # Fill TH2F
#    print(nx)
#    print(ny)
#    print(values)
    arr = np.array(values)  # your 2D array
    cleaned_2d = [
        [v for v in row if not np.isnan(v)]   # remove NaNs from each row
        for row in arr
        if not np.all(np.isnan(row))          # skip rows that are all NaN
    ]
    cleaned_2d = np.array(cleaned_2d)
    print(ch.split('.')[0])
    print(cleaned_2d.tolist())

#for i in range(len(pt_edges - 1)):
#    for j in range(len(eta_edges) - 1):
#        val = cleaned_2d[i, j]
#        print(f"{i}:{j}:{val}")
#        if np.isnan(val):
#            continue  # skip NaNs
#        print (val)
#        if not np.isnan(val):
#            ROOT.TH2.SetBinContent(h, int(i+1), int(j+1), float(val))
#
## Save ROOT file
#fout = ROOT.TFile("flip_probs_UL17.root", "RECREATE")
#h.Write()
#fout.Close()
#print("Saved TH2F to flip_probs_UL17.root")
