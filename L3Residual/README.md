# L3 Residual Corrections from Photon+Jet Balancing

This directory contains scripts for deriving and fitting L3 residual corrections using photon+jet balancing events.

## Workflow Overview

The L3 residual correction workflow consists of two main steps:

### Step 1: Derive L3 Residuals from 3D Balance Profiles
**Script:** `deriveL3_from_photonjet.C`

This script takes photon+jet balance histograms (3D profiles: pT, η, α) from the output of `analyse_PhotonJet.cc` and produces:
- `L3Res_vsa_norm_{ptbin}_{etabin}`: L3 residual (MC/Data balance ratio) normalized to a reference alpha value
- `L3_derived.txt`: Text file with L3 corrections in JEC format
- Other intermediate histograms for debugging

**Usage:**
```cpp
root -l -b -q 'deriveL3_from_photonjet.C("PHOTONMC_output_tag.root", "PHOTONHP_output_tag.root", "L3_derived.root", true, 5, true, false)'
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

This script reads the `L3Res_vsa_norm_` histograms and fits them as a function of alpha to extract correction factors.

**Usage:**
```cpp
root -l -b -q 'dofits_L3.C("L3_derived.root", 0.15, 0.35, "L3kfactor_photonjet", true)'
```

**Parameters:**
- `inFileL3Derived`: Input ROOT file from Step 1 (default: "L3_derived.root")
- `fitmin`: Minimum alpha value for fit range (default: 0.15)
- `fitmax`: Maximum alpha value for fit range (default: 0.35)
- `outfilename`: Output ROOT file name (default: "L3kfactor_photonjet")
- `doabseta`: Use absolute eta binning (true) or signed eta (false)

**Output:**
- `L3fits/L3kfactor_photonjet.root`: ROOT file containing:
  - `l3factors`: 1D histogram of correction factors vs |η|
  - `corrections_*`: L3 corrections per pT bin
- `L3fits/fits_eta_*.png/pdf`: Individual fit plots for each eta bin
- `L3fits/kfactors.png/pdf`: Summary plot of correction factors vs eta
- `L3fits/corrections.png/pdf`: Final L3 corrections vs eta for different pT ranges

## Complete Workflow Example

```bash
cd /path/to/L3Residual/

# Generate input files with analyse_PhotonJet.cc
# (assumes PHOTONHP_output_tag.root and PHOTONMC_output_tag.root exist)

# Step 1: Derive L3 residuals
root -l -b -q 'deriveL3_from_photonjet.C("../PHOTONMC_output_tag.root", "../PHOTONHP_output_tag.root", "L3_derived.root", true, 5, true, false)'

# Step 2: Fit and extract correction factors
root -l -b -q 'dofits_L3.C("L3_derived.root", 30, 100, "L3kfactor_photonjet", true)'

# Results are in L3fits/ directory
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
The `dofits_L3.C` script fits these histograms with `pol1` (linear fit) in the range [0.15, 0.35]:
- **p0**: Correction factor at alpha reference (~0.3)
- **p1**: Slope (usually close to 0 for well-behaved data)

A pol1 fit gives: Correction(α) = p0 + p1*α

## Notes

1. **Alpha Bin Selection**: Alpha bin 5 corresponds to α < 0.3 (alpha cut). This is the reference alpha value to which other alpha bins are normalized.

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
