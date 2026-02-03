# L3 Residual Corrections from Photon+Jet Balancing

This directory contains scripts for deriving and fitting L3 residual corrections using photon+jet balancing events.

## Workflow Overview

The L3 residual correction workflow consists of these main steps:

### Step 0: Plot Balance Distributions and Kinematics
**Script:** `plotresponse_L3.C`

This script reads the photon+jet analysis output directly and creates diagnostic plots of balance distributions and kinematic variables using TDR style.

**Usage:**
```cpp
root -l -b -q 'plotresponse_L3.C("PHOTONHP_AK4_photonjet.root", "Data")'
root -l -b -q 'plotresponse_L3.C("PHOTONMC_AK4_photonjet.root", "MC", "2024ppRef", "pp 480.4 pb^{-1}", true)'

# Default lumi label changed to "pp 480.4 pb^{-1}" to reflect dataset integrated luminosity.
```

```cpp
root -l -b -q 'plotresponse_L3.C("/eos/cms/store/group/phys_heavyions/bharikri/JetMinPOG/L3ResPhotonJet/2026_02_02_PHOTONHP_balance_distroot","2026_02_02_balance_dist_comparison",true,"/eos/cms/store/group/phys_heavyions/bharikri/JetMinPOG/L3ResPhotonJet/2026_02_02_QCDPhoton_balance_dist.root")'
```

**Parameters:**
- `inputFile`: Path to analyse_PhotonJet.cc output ROOT file
- `tag`: Label for output (e.g., "Data", "MC") 
- `runLabel`: Run label for CMS lumi text (default: "2024ppRef")
- `lumiLabel`: Luminosity label (default: "pp Reference")
- `isMC`: Is this MC sample (default: false)

**Output:** Creates `L3plots_{tag}/` directory with:
- `kinematics/`: Photon pT, eta, phi; Jet pT, eta, phi; Δφ, α, pT,avg distributions
- `balance_dist/`: Balance distributions for various pT and α bins
- `balance_dist/balance_map_{tag}.pdf`: 2D map of mean balance vs (pT, α)
- `balance_dist/balance_dist_pt*.pdf`: Balance distributions with Gaussian fits
- `balance_dist/balance_resolution_{tag}.pdf`: Balance RMS vs pT for different α cuts
- `balance_dist/balance_mean_{tag}.pdf`: Mean balance vs pT for different α cuts

**Note:** This uses the new `photonjet_balance_dist` (TH3D) histogram which stores full balance distributions (photon_pT, α, balance_value) without eta binning. To use this, you must reprocess your data with the updated `analyse_PhotonJet.cc`.

### Step 1: Derive L3 Residuals from 3D Balance Profiles
**Script:** `deriveL3_from_photonjet.C`

This script takes photon+jet balance histograms (3D profiles: pT, η, α) from the output of `analyse_PhotonJet.cc` and produces:
- `L3Res_vsa_norm_{ptbin}_{etabin}`: L3 residual (MC/Data balance ratio) normalized to a reference alpha value
- `L3_derived.txt`: Text file with L3 corrections in JEC format
- Other intermediate histograms for debugging

**Usage:**
```cpp
// For L3 pT-only (barrel) studies, use the wide-|eta| profile (single bin 0<|eta|<1.3)
root -l -b -q 'deriveL3_from_photonjet.C("PHOTONMC_output_tag.root", "PHOTONHP_output_tag.root", "L3_derived.root", true, 5, false, true)'
```

**Parameters:**
- `mcFile`: Path to MC photon+jet analysis output
- `dataFile`: Path to Data photon+jet analysis output  
- `outfilename`: Output ROOT file name (default: "L3_derived.root")
- `dodt`: Process data (true/false)
- `alphabin`: Alpha bin to use for L3 computation (5 = alpha < 0.3)
- `useabs`: Use absolute eta binning (true) or signed eta (false)
- `usewideabs`: Use wide absolute eta binning (alternative binning option)

**Output:**
- `L3_derived.root`: ROOT file containing L3Res_vsa_norm histograms
- `L3_derived.txt`: Text file with L3 corrections per eta bin and pT range

### Step 2: Fit L3 Corrections vs Alpha
**Script:** `dofits_L3.C`

This script reads the derived photon+jet balance products (ratios vs pT for each alpha cut, plus the underlying 3D profiles) and:

- Fits the **normalized ratio vs alpha** to extract `k_FSR` and extrapolate to $\alpha \to 0$.
- Builds a **pT-only** correction shape and fits it vs pT to write an L3Residual JEC text.
- The application of `k_FSR` to the pT-shape fit is optional (user flag).

**Usage:**
```cpp
// Run from the repo top-level (recommended) so the default jecfiles paths work
root -l -b -q 'L3Residual/dofits_L3.C("L3_derived.root", 60, 300, "L3Res_photonjet", false, "2024ppRef", "pp 480.4 pb^{-1}", true, true)'
# For balance distribution overlays (requires raw derived files with photonjet_balance_dist):
# root -l -b -q 'L3Residual/dofits_L3.C("L3_derived.root", 60, 300, "L3Res_photonjet", false, "2024ppRef", "pp 480.4 pb^{-1}", true, true, 5, 0.0, 0.4, true, true, "mc_raw.root", "data_raw.root")'
# If raw files not provided, the macro will fall back to generating per-pT balance overlays using the input 3D profiles (saved to raw/balance_dist_overlay/).
// Multi-input combined fit (e.g. photon+jet + Z+jet derived products)
// - First argument is a comma-separated list of derived ROOT files
// - Optional: provide labels (inputLabelsCSV) and per-input pT ranges (inputPtRangesCSV)
root -l -b -q 'L3Residual/dofits_L3.C(
   "L3_derived_photonjet.root,L3_derived_zjet.root",
   60, 1000,
   "L3Res_combined",
   false,
   "2024ppRef",
   "pp Reference",
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

**Parameters:**
- `inFileL3Derived`: Input ROOT file from Step 1 (default: "L3_derived.root")
- `ptminG`, `ptmaxG`: pT range used for the pT-shape fit
- `outfilename`: Output tag used for folder/file naming
- `saveAlphaExtrap`: Enable `k_FSR` extraction and alpha→0 extrapolation (recommended)

**Output:**
- `L3Residual/L3fits_<tag>/textfiles/<tag>.txt`: L3Residual JEC text
- `L3Residual/jecfiles/L3Residuals_<runLabel>_photonjet_AK4PF.txt`: copy of the single-input L3Residual JEC text
- `L3Residual/jecfiles/L3Residuals_<runLabel>_combined_AK4PF.txt`: copy of the multi-input combined-fit L3Residual JEC text
- Optional: `L3Residual/jecfiles/L2L3Residuals_<runLabel>_photonjet_AK4PF.txt` (only if enabled)
- Optional: raw-shape overlays from `photonjet_balance_dist` if you provide the raw MC/data files

## Complete Workflow Example

```bash
cd /path/to/L3Residual/

# Generate input files with analyse_PhotonJet.cc (see ../fillhistograms/)
# (assumes PHOTONHP_AK4_photonjet.root and PHOTONMC_AK4_photonjet.root exist)

# Step 0 (Optional): Plot balance distributions and kinematics for quality checks
root -l -b -q 'plotresponse_L3.C("../PHOTONHP_AK4_photonjet.root", "HP_Data")'
root -l -b -q 'plotresponse_L3.C("../PHOTONMC_AK4_photonjet.root", "QCD_MC", "2024ppRef", "pp Reference", true)'

# Step 1: Derive L3 residuals from balance profiles
root -l -b -q 'deriveL3_from_photonjet.C("../PHOTONMC_AK4_photonjet.root", "../PHOTONHP_AK4_photonjet.root", "L3_derived.root", true, 5, false, true)'

# Step 2: Fit and extract correction factors
root -l -b -q 'dofits_L3.C("L3_derived.root", 30, 100, "L3Res_photonjet", false)'

# Results are in L3fits_L3Res_photonjet/ directory
# Diagnostic plots are in L3plots_HP_Data/ and L3plots_QCD_MC/
```

## Key Differences from L2 (Dijet) Residuals

| Aspect | L2 (Dijet) | L3 (Photon+Jet) |
|--------|-----------|-----------------|
| Balance Definition | (Jet1 + Jet2)/(2*Jet_avg_pT) | Jet_pT / Photon_pT |
| Analysis Event | Dijet events | Photon+Jet events |
| Derivation | `deriveL2_from3D.C` | `deriveL3_from_photonjet.C` |
| Fitting | `dofits.C` | `dofits_L3.C` |
| Histogram Names | `Respvsa_norm_` | `L3Res_vsa_norm_` |
| pT Ranges | 15-25, 25-80, 80-120, 120-1000 GeV | 15-25, 25-80, 80-120, 120-1000 GeV |
| Eta Binning | Multiple options (eta, abseta, wide abseta) | Multiple options (eta, abseta, wide abseta) |

## Understanding the Output

### L3Res_vsa_norm Histograms
These histograms show how the L3 residual correction varies with alpha (3rd jet fraction):
- X-axis: alpha values (0.0 to 0.5)
- Y-axis: L3Res normalized to the alpha=0.3 bin
- Each histogram is for a specific (pT bin, eta bin) pair

### Fit Results
The `dofits_L3.C` script fits these histograms with `pol1` (linear fit) in the range [0.0, 0.4] by default:
- **p0**: Correction factor at alpha reference (~0.3)
- **p1**: Slope (usually close to 0 for well-behaved data)

A pol1 fit gives: Correction(α) = p0 + p1*α

## Notes

1. **Balance Distribution Histogram**: The new `photonjet_balance_dist` (TH3D) histogram stores full balance distributions with axes (photon_pT, alpha, balance_value). This enables detailed analysis of balance shapes and resolutions. The histogram uses fixed binning: 12 pT bins (60-300 GeV), 50 alpha bins (0-0.5), and 200 balance bins (0-2). No eta binning is included - balance is integrated over the full detector acceptance.

2. **Alpha Bin Selection**: Alpha bin 5 corresponds to α < 0.3 (alpha cut). This is the reference alpha value to which other alpha bins are normalized.

2. **Eta Binning**: The script uses absolute eta binning (|η|) with 18 bins spanning 0 to 5.191. The loop stops at eta bin 14 to handle low-statistics regions.

3. **pT Bins**: Four pT bins are used:
   - 15-25 GeV (low pT)
   - 25-80 GeV 
   - 80-120 GeV
   - 120-1000 GeV (high pT)

4. **Warnings in deriveL3**: Warnings about L3Res values > 2.0 indicate regions with potentially problematic data. These corrections are clamped to 1.0 in the text output.

5. **File Sizes**: 
   - L3_derived.root: ~185 KB (contains all 3D profiles and derived histograms)
   - L3kfactor_photonjet.root: ~10 KB (contains final correction factors)

## Troubleshooting

### Missing Histograms
If you get errors about missing `L3Res_vsa_norm_` histograms:
- Ensure `analyse_PhotonJet.cc` was run successfully on your input files
- Check that `photonjet_balance3D` histograms exist in the input ROOT files

### Poor Fit Quality
If fits fail or give unphysical results:
- Check the individual eta bin plots in `L3fits/fits_eta_*.png`
- Adjust fit range with `fitmin` and `fitmax` parameters
- Consider investigating the underlying balance data

### Compilation Issues
- Ensure ROOT environment is set up: `source /cvmfs/sft.cern.ch/lcg/views/LCG_106a/x86_64-el9-gcc14-opt/setup.sh`
- Verify `histograms.h` is accessible in `../fillhistograms/`

## References

- L2 Residual Fitting: See `../L2Residual/dofits.C` and `deriveL2_from3D.C`
- Analysis Code: See `../fillhistograms/analyse_PhotonJet.cc`
- Histogram Definitions: See `../fillhistograms/histograms.h`
