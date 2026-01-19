# L3 Residual Analysis - Complete Workflow Documentation

## Overview

The L3 residual correction derives absolute pT-dependent corrections for jets using photon+jet balance. Unlike L2 residuals (relative pT-dependent) which are derived from dijet events, L3 residuals correct the absolute jet energy using photons as reference objects.

## Physics Background

**Photon+Jet Balance Method:**
- Select events with one photon and one jet back-to-back in azimuth (Δφ > 3.0)
- Measure balance: B = p_T^(jet) / p_T^(γ)
- In perfect detectors, B = 1 for unbiased photons
- In data: B_data may differ from B_MC due to jet energy scale differences
- L3 Residual correction: C_L3 = B_MC / B_data

**Alpha Dependence:**
- α = p_T^(3rd jet) / p_T^(leading objects) measures additional radiation
- L3 corrections typically depend on α (ISR/FSR effects)
- Reference alpha cut: α < 0.3

## Detailed Workflow

### 1. Input Generation: analyse_PhotonJet.cc

**What it does:**
- Reads raw photon+jet events from data and MC
- Applies photon and jet selection cuts
- Fills 3D balance profiles: (p_T^(avg), η_jet, α)

**Input:**
- ROOT files from NANOAOD or similar format with:
  - Photon 4-vectors and quality variables
  - Jet 4-vectors and properties
  - Event weights and triggers

**Output:**
- `PHOTONMC_output_tag.root`: MC photon+jet histograms
- `PHOTONHP_output_tag.root`: Data photon+jet histograms

**Key 3D Histograms Created:**
- `photonjet_balance3D`: Balance vs (p_T^(avg), η_jet, α)
- `photonjet_balance3Dabseta`: Balance vs (p_T^(avg), |η_jet|, α)
- `photonjet_balance3Dwide`: Wider eta binning variant

### 2. Derivation Step: deriveL3_from_photonjet.C

**What it does:**
- Loads 3D balance profiles from analyse_PhotonJet output
- For each (pT, eta) bin:
  - Extracts balance vs α for MC and Data
  - Computes ratio: L3Res(α) = B_MC(α) / B_Data(α)
  - Normalizes to reference alpha: L3Res_norm(α) = L3Res(α) / L3Res(α_ref)

**Key Operations:**

```cpp
// For each pT bin and eta bin:
for (int abin = 1; abin <= nalphabins; ++abin) {
    B_mc = mc3d->GetBinContent(ptbin, etabin, abin);
    B_dt = data3d->GetBinContent(ptbin, etabin, abin);
    
    L3Res(abin) = B_mc / B_dt;
    L3Res_norm(abin) = L3Res(abin) / L3Res(alpha_ref_bin);
}
```

**Output Histograms:**
- `L3Res_vsa_norm_{ptbin}_{etabin}`: Normalized L3 residuals vs α
  - X-axis: Alpha values [0.0, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5]
  - Y-axis: L3Res relative to reference
  - Typically flat near 1.0 with ~5% variations across α range

**Example Output Files:**
- `L3Res_vsa_norm_1_1`: pT bin 1 (15-25 GeV), eta bin 1 (0.0-0.261)
- `L3Res_vsa_norm_3_7`: pT bin 3 (80-120 GeV), eta bin 7 (1.479-1.653)

### 3. Fitting Step: dofits_L3.C

**What it does:**
- Reads `L3Res_vsa_norm_` histograms for each eta bin
- For each eta bin:
  - Loads multiple pT bins
  - Fits combined data with linear function: p0 + p1*α
  - Extracts p0 (correction factor) in fit range [0.15, 0.35]
- Compiles eta-dependent correction factors
- Optionally (when `saveAlphaExtrap=true`) rebuilds MC/Data/ratio vs α for every (pT, η) bin from the 3D profiles, fits a linear function to extrapolate to α→0, and stores PNGs under `L3fits_<tag>/alpha_extrap/`.
- Optionally (when `plotRawResponses=true`) saves the individual MC and Data balance vs pT (no ratio/fits) into `L3fits_<tag>/raw/` for visual QC.

**Fitting Strategy:**

```cpp
// For each eta bin:
for (int ptbin = 1; ptbin <= 4; ++ptbin) {
    histogram = L3Res_vsa_norm_{ptbin}_{etabin};
    // Create TGraph from histogram bins
    graphs[ptbin] = TGraphErrors(...);
    multifit->Add(graphs[ptbin], "P");
}

// Fit all pT bins together
TF1 f1("f1", "pol1", 0.15, 0.35);
multifit->Fit("f1", "R");
factors[etabin] = f1->GetParameter(0); // Correction factor
```

**Output:**

1. **Per-eta-bin plots** (`fits_eta_N.png`):
   - Individual pT bin symbols with different colors
   - Best-fit line through all points
   - Fit parameters p0, p1 displayed
   - Used for quality control

2. **Correction factors vs η** (`kfactors.png`):
   - 1D histogram with fit results
   - Smooth fit function: f(|η|) = p0 + p1*cosh(|η|)/(1+p2*cosh(|η|))
   - Shows eta dependence of absolute corrections

3. **Final L3 corrections** (`corrections.png`):
   - L3 corrections per pT bin after applying fit factors
   - Multiple pT ranges shown separately
   - Ready for use in jet energy calibration

## Test Results

**Input Files Used:**
- `PHOTONMC_output_tag.root`: 4 pT bins × 18 η bins
- `PHOTONHP_output_tag.root`: 4 pT bins × 18 η bins
- Total: 72 (pT, η) combinations analyzed

**Derivation Results:**
- Generated 72 × 9 = 648 `L3Res_vsa_norm_` histograms
- Warnings for 14 bins with L3Res > 2.0 (clamped to 1.0 in output)
- Output text file: `L3_derived.txt` with corrections in JEC format

**Fitting Results:**
- Successfully fitted 14 eta bins
- Fit ranges: [0.15, 0.35]
- Correction factors range: ~0.9 to ~1.7
- Smooth eta dependence observed

**Output Files:**
- `L3_derived.root`: 185 KB
- `L3_derived.txt`: Text format JEC corrections
- `L3kfactor_photonjet.root`: 10 KB with final factors
- `L3fits/`: 32 PNG/PDF plots + summary plots

## Understanding the Numbers

### Correction Factor Interpretation

If L3 correction factor = 1.15 at |η| = 0.5:
- Measured jet pT in data is 15% lower than in MC
- Apply correction: p_T^corrected = 1.15 × p_T^measured
- Corrects data jets to match MC energy scale

### Alpha Dependence

The fit parameter p1 (slope) indicates:
- p1 ≈ 0: L3 correction is independent of α (clean ISR/FSR modeling)
- p1 > 0: Correction increases with α (data has more ISR/FSR)
- p1 < 0: Correction decreases with α (MC has more ISR/FSR)

### Eta Dependence

The smooth fit f(|η|) captures how absolute corrections vary:
- Barrel (|η| < 1.5): typically 0.95-1.05
- Endcaps (|η| > 1.5): can vary more significantly
- Forward region (|η| > 2.5): larger corrections due to detector effects

## Quality Control Checks

### 1. Visual Inspection
- Plot `fits_eta_*.png` files
- Check that fits pass through data points
- Look for outliers or unusual trends

### 2. Consistency Tests
```bash
# Compare with L2 residuals
root -e 'TFile f1("L3fits/L3kfactor_photonjet.root"); 
          TFile f2("../L2Residual/L2fits/L2kfactor.root");
          auto h1 = (TH1D*)f1->Get("l3factors");
          auto h2 = (TH1D*)f2->Get("factors");
          // Should show similar patterns'
```

### 3. Physical Reasonableness
- L3 corrections should be 0.8 - 1.3 (not extreme)
- Should be smooth function of η (no discontinuities)
- Should be relatively flat vs α (ISR/FSR effects are small)

## Common Issues and Solutions

### Issue: Poor Fit Quality
**Symptoms:** High χ²/NDF, large fit errors
**Causes:** 
- Low statistics in certain bins
- Real physics effects (ISR/FSR structure)
- Data/MC differences

**Solutions:**
- Adjust fit range (fitmin, fitmax)
- Combine adjacent eta bins
- Check for detector effects in specific regions

### Issue: Unphysical Corrections (< 0.8 or > 1.3)
**Symptoms:** Correction factor out of reasonable range
**Causes:**
- Data quality issues
- MC generation problems
- Photon purity problems

**Solutions:**
- Check photon selection cuts
- Review MC sample generation
- Apply additional quality filters

### Issue: Large Error Bars on Fit Results
**Symptoms:** p0 error > 0.1
**Causes:**
- Insufficient statistics
- Large variations across α bins

**Solutions:**
- Use wider eta bins for high-|η| regions
- Combine lower pT bins if needed
- Increase input statistics

## Running on Different Data Samples

### Using Alternative Eta Binning
```cpp
// Use wide absolute eta binning (8 bins instead of 18)
root -l -b -q 'deriveL3_from_photonjet.C(..., true, 5, false, true)'
               // Last three params: dodt, useabs, usewideabs
```

### Different Alpha Reference Bin
```cpp
// Use alpha < 0.4 instead of alpha < 0.3
root -l -b -q 'deriveL3_from_photonjet.C(..., true, 6, true, false)'
               // Parameter 5: alphabin = 6 (0.3-0.4)
```

### Batch Processing
```bash
#!/bin/bash
for year in 2023 2024; do
    root -l -b -q "deriveL3_from_photonjet.C('MC_${year}.root', 'DATA_${year}.root', 'L3_${year}.root')"
    root -l -b -q "dofits_L3.C('L3_${year}.root', 0.15, 0.35, 'L3_${year}_fits')"
done
```

## Advanced: Extracting Text Format JEC

The `L3_derived.txt` file produced by deriveL3 contains L3 corrections in CMS JEC format:

```
{eta_min eta_max} N pt_min pt_max correction ...
{0 0.261} 14 15 1000 15 25 1 25 80 1 80 120 1 120 1000 1.54585
```

To use in CMSSW:
```python
# In your CMSSW config
process.jec = cms.ESSource("PoolDBESSource",
    CondDB = cms.PSet(
        connect = cms.string('sqlite:./L3_derived.db'),
        timetype = cms.untracked.string('timestamp'),
        toGet = cms.VPSet(
            cms.PSet(
                record = cms.string('JetCorrectionsRecord'),
                tag = cms.string('JetCorrectorParametersCollection_L3_PhotonJet'),
                label= cms.untracked.string('AK4PF')
            )
        )
    )
)
```

## References and Further Reading

- CMS Jet Energy Scale documentation: https://cds.cern.ch/record/2138504
- Previous L3 residual measurements: CMS-PAS-JME-19-004
- Photon+Jet balance method: JHEP 12 (2016) 033
- Absolute scale determination: CMS-PAS-JME-15-001
