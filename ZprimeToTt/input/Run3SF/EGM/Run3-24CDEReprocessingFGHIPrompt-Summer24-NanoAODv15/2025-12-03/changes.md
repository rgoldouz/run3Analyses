## Changes: 2025-12-03 (Update electron and photon reco and ID scale factors)

Merge Request: [!2](https://gitlab.cern.ch/cms-analysis-corrections/EGM/Run3-24CDEReprocessingFGHIPrompt-Summer24-NanoAODv15/-/merge_requests/2)

## Photon SFs
- regroups ID and veto SF in a single file (like in the other periods)
- Added low pt ID SF for photons (as an additional WP, a change of format will follow in a future update)

## Electron SFs
- Added low pt<20 GeV reconstruction SF. 
- Added low pt<20 GeV ID SF
- regrouped both reco and ID electron SF in a single file (like in the other periods)
- Extrapolated standard ID SF to high energies with a new strategy:
  - For Endcaps: Use the SF from the pt 200–300 bin and apply it inclusively for pt > 200 and use the higher pt bins to assign an uncertainty growing with pt. Very conservative uncertainty of 3% at 300 GeV growing up to 7.5% at 1 TeV.
  - For Barrel: Use the SF from the pt 300–500 bin and apply it inclusively for pt > 300 and use the higher pt bins to assign an uncertainty growing with pt. Very conservative uncertainty of 2% at 300 GeV growing up to 5 % at 1 TeV.
  
- include additional overflow bin for high pt (>1000 GeV) with 1+-0.2 scale factor for standard ID. (The HEEP ID should be use in this high energy region)
- include stat and syst error maps
- include effMC and effData maps
