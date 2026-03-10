# Residual Analysis Framework

Run instructions for producing JEC residual corrections (L2, L3) and JER scale factors.

The inputs for these macros are HiForest ntuples are 2023 ppRef and 2024 ppRef HiForest ntuples.

For architecture, data products, histogram contracts, and implementation details, see DOCUMENTATION.md.

## Environment
- CMSSW environment (tested with CMSSW_15_1_0_patch3)
- ROOT (provided by CMSSW)
- HTCondor (for batch processing)

## Quick Start

### Setup Environment
```bash
cd /eos/home-b/bharikri/lxplus_private/EGamma/CMSSW_15_1_0_patch3/src
cmsenv
```


## Build helpers (for histogram, binning or JEC file changes)
```bash
cd /eos/home-b/bharikri/lxplus_private/EGamma/residualanalysis/fillhistograms
root -l -b -q compile.C
```

## L3 (photon+jet) end-to-end

The photon+jet histogram production code lives in `fillhistograms/`.

1) Produce photon+jet histograms (MC and Data):
```bash
cd /eos/home-b/bharikri/lxplus_private/EGamma/residualanalysis/fillhistograms

# Filelist mode
root -l -b -q 'analyse_PhotonJet.cc("/path/to/filelist.txt", "output_tag", true,  true,  "filelist", -1, -1, "/output/dir")'
root -l -b -q 'analyse_PhotonJet.cc("/path/to/filelist.txt", "output_tag", false, true,  "filelist", -1, -1, "/output/dir")'
```

2) (Optional) Diagnostic plots directly from analysis output:
```bash
cd /eos/home-b/bharikri/lxplus_private/EGamma/residualanalysis/L3Residual
root -l -b -q 'plotresponse_L3.C("/path/to/PHOTONHP_output_tag.root", "Data")'
root -l -b -q 'plotresponse_L3.C("/path/to/PHOTONMC_output_tag.root", "MC", "2024ppRef", "pp 480.4 pb^{-1}", true)'
```

3) Derive L3 derived products (recommended for pT-only barrel: wide-|eta| bin):
```bash
cd /eos/home-b/bharikri/lxplus_private/EGamma/residualanalysis
root -l -b -q 'L3Residual/deriveL3_from_photonjet.C("PHOTONMC_output_tag.root", "PHOTONHP_output_tag.root", "L3Residual/L3_derived_photonjet.root", true, 5, false, true)'
```

4) Fit (single input):
```bash
cd /eos/home-b/bharikri/lxplus_private/EGamma/residualanalysis
root -l -b -q 'L3Residual/dofits_L3.C("L3Residual/L3_derived_photonjet.root", 60, 300, "L3Res_photonjet", false, "2024ppRef", "pp 480.4 pb^{-1}", true, false)'
```

5) Fit (multi-input combined pT fit, e.g. photon+jet + Z+jet):
```bash
cd /eos/home-b/bharikri/lxplus_private/EGamma/residualanalysis
root -l -b -q 'L3Residual/dofits_L3.C(
  "L3Residual/L3_derived_photonjet.root,L3Residual/L3_derived_zjet.root",
  60, 1000,
  "L3Res_combined",
  false,
  "2024ppRef",
  "pp 480.4 pb^{-1}",
  true,
  false,
  5,
  0.0,
  0.4,
  true,
  false,
  true,
  1,
  false,
  false,
  "",
  "",
  false,
  "fillhistograms/jecfiles/L2Residuals_2024ppRef_fixed.txt",
  "L3Residual",
  "L3Residual/jecfiles",
  "photon+jet,Z+jet",
  true,
  "60-300,300-1000"
)'
```

Outputs are written under `L3Residual/` (plots + root outputs) and `L3Residual/jecfiles/` (JEC text copies).

## L2 (dijet)

Run the L2 workflow from the `L2Residual/` directory:
```bash
cd /eos/home-b/bharikri/lxplus_private/EGamma/residualanalysis/L2Residual
root -l -b -q 'deriveL2_from3D.C("/path/to/input.root")'
root -l -b -q 'dofits.C()'
root -l -b -q 'plotresponses.C()'
root -l -b -q 'doTxt.C()'
```

Typical chain:
- `deriveL2_from3D.C`: build the L2 residual inputs from the histogram outputs
- `dofits.C`: fit the response ratios vs alpha
- `plotresponses.C`: diagnostic response plots
- `doTxt.C`: write txt outputs

## JER and JER scale factors

To fill histograms for JER scale factors, use the dedicated JER/tag-and-probe configuration in the analysis step and apply the L2 residual JEC before deriving the SFs.

MC validation macros:
- `JER/MCJER.C`: pT resolution
- `JER/MCJPR.C`: eta/phi resolution
- `JER/MCRESP.C`: MC response `<pT(reco)/pT(gen)>`
- `textFiles/doTxtMCJER.C`: print txt files of resolution fit parameters

Scale factor workflow:
- `JER/JERSF_fits.C`: extract resolution from Gaussian fits to dijet asymmetry distributions
- `JER/JERSF_RMS.C`: extract resolution from truncated RMS of dijet asymmetry distributions
- `JER/JERSF_fits_vsalpha.C`: fit the extracted resolutions vs alpha
- `JER/JERSF_printtxt.C`: write txt outputs

Trigger turn-on studies are under `triggerstudy/plottriggereff.C`.

## Batch processing

See `batch/README.md` for HTCondor submission, merging, and input conventions.
