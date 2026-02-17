# L3 Residual Analysis - Framework Map & Feature Reference

This document is a reference for the residualanalysis repository. It contains:
- A short framework map (directories, primary scripts, histogram contracts)
- Detailed usage and modes for the photon+jet (L3) workflow including single-file, directory, and filelist modes.
- Full description of the L3Residual calculation (derivation, alpha-extrapolation, kFSR, pT fit and multi-input combination).

Overview: The L3 residual correction derives absolute pT-dependent corrections for jets using photon+jet balance. L2 residuals are eta-dependent corrections derived with dijet, while L3 corrects the absolute jet energy scale using well-measured photons as reference objects.

**Framework Map (quick agent reference)**
- `fillhistograms/`: analysis code that reads ntuples and fills histograms. Key files: `analyse_PhotonJet.cc`, `histograms.h`, `chain_builder.h`, and `compile.C`.
- `L3Residual/`: derivation and fitting macros: `deriveL3_from_photonjet.C`, `dofits_L3.C`, `plotresponse_L3.C`, and `jecfiles/` for outputs.
- `L2Residual/`, `JER/`: sibling analysis directories for L2 and JER workflows.
- `batch/`: HTCondor submission helpers and filelists. Use `submit_condor.py` to launch `analyse_PhotonJet.cc` across many inputs.

Histograms (important names expected by derivation/fit macros):
- `photonjet_balance3D` (TProfile3D): axes = (photon-jet average pT, jet |η| or η, α), stores mean balance (jet_pT/photon_pT).
- `photonjet_balance3Dabseta`, `photonjet_balance3Dwide`: alternative eta binnings; `photonjet_balance_dist` (TH3D) optional: (photon_pT, alpha, balance_value).

Analysis macros:
- `deriveL3_from_photonjet.C(mcFile, dataFile, outfilename="L3_derived.root", dodt=true, alphabin=5, useabs=true, usewideabs=false)`
- `dofits_L3.C(inFileL3Derived = "L3_derived.root", ptminG=30., ptmaxG=100., outfilename="L3Res_photonjet", closure=false, runLabel="2024ppRef", lumiLabel="pp Reference", plotRawResponses=true, saveAlphaExtrap=false, refAlphaBin=5, fitAlphaMin=0.0, fitAlphaMax=0.4, applyKFSRToPtFit=true, doEtaBinnedAlphaFits=false, useSingleEtaBin=true, etaBinForL3=1, plotEtaMaps=false, plotBalanceDistOverlay=false, mcRawFileForDist="", dataRawFileForDist="", writeL2L3=false, l2ResidualFile="fillhistograms/jecfiles/L2Residuals_2024ppRef_fixed.txt", outBaseDir="L3Residual", jecOutDir="L3Residual/jecfiles", inputLabelsCSV="", doCombinedPtFit=true, inputPtRangesCSV="")`

Notes for agents:
- Prefer invoking macros from the repo top-level so relative `jecfiles/` paths resolve correctly.
- Use `root -l -b -q 'macro.C(args...)'` for batch runs; append `++` for ACLiC compile when iterating.


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

### 1. Input Generation: `analyse_PhotonJet.cc`

What it does:
- Reads photon+jet events from input ROOT files (MC and Data)
- Applies photon and jet selection and fills balance histograms used by the derivation and fitting steps

Input modes (supported by `analyse_PhotonJet.cc`):
- Single-file / Era (legacy) mode (short): useful for quick checks
  ```bash
  root -l -b -q 'analyse_PhotonJet.cc("PHOTONHP", "output_tag", false, false)'
  ```
- Directory mode (process all files in directory):
  ```bash
  root -l -b -q 'analyse_PhotonJet.cc("/path/to/directory", "output_tag", false, false, "directory", 100, 10000, "/output/dir")'
  ```
- Filelist mode (recommended for batch):
  ```bash
  root -l -b -q 'analyse_PhotonJet.cc("/path/to/filelist.txt", "output_tag", false, false, "filelist", -1, -1, "/output/dir")'
  ```

Outputs (per-run):
- `PHOTONMC_<tag>.root` (MC) and `PHOTONHP_<tag>.root` (Data) — ROOT files containing the histograms below

Key histogram products created by `analyse_PhotonJet.cc`:
- `photonjet_balance3D`: TProfile3D (pT_avg, eta, alpha) mean balance
- `photonjet_balance3Dabseta`: same as above with |eta|
- `photonjet_balance3Dwide`: variant with wider |eta| bins (useful for pT-only barrel derivation)
- `photonjet_balance_dist`: optional TH3D storing full balance distributions (photon_pT, alpha, balance) for overlay/shape studies

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

### L3Residual calculation details (step-by-step)

This section describes the full logic implemented across `deriveL3_from_photonjet.C` and `dofits_L3.C` so agents can reproduce or audit the calculation.

1) Inputs: merged MC and Data analysis ROOT files containing `photonjet_balance3D*` profiles (and optionally `photonjet_balance_dist` TH3D).

2) Per-(pT,η) derivation (in `deriveL3_from_photonjet.C`):
  - Extract the profile values $B_{MC}(p_T,\eta,\alpha)$ and $B_{Data}(p_T,\eta,\alpha)$ from the 3D TProfile.
  - Compute the ratio (raw L3 residual) per alpha bin:
    $$ R(p_T,\eta,\alpha) = \frac{B_{MC}(p_T,\eta,\alpha)}{B_{Data}(p_T,\eta,\alpha)}. $$
  - Define a reference alpha bin (e.g. $\alpha_{ref}=0.3$). Build normalized histograms:
    $$ R_{norm}(p_T,\eta,\alpha) = \frac{R(p_T,\eta,\alpha)}{R(p_T,\eta,\alpha_{ref})}.$$ 
  - Store `L3Res_vsa_norm_{ptbin}_{etabin}` (useful for alpha fits) and default `ratio_vsphotonpt_alphaN` histograms.

3) Alpha-extrapolation and kFSR extraction (optional, `saveAlphaExtrap=true` in `dofits_L3.C`):
  - For each pT bin (and optionally each eta bin) build the normalized ratio vs $\alpha$ histogram.
  - Fit a linear function in $\alpha$ over a configurable range (default $[0.0,0.4]$ or narrower) excluding the reference bin. The linear fit is used to extrapolate to $\alpha\rightarrow 0$.
  - Extract $k_{FSR}(p_T)$ as the fit intercept at $\alpha=0$ (this captures residual FSR/ISR modeling differences between MC and Data). Store $k_{FSR}(p_T)$ and its uncertainty.
    - When enabled, `dofits_L3.C` also saves:
      - per-$p_T$ PNGs showing the points and linear fit used to extract $k_{FSR}$ (`.../alpha_extrap/L3Res_<run>_kFSR_alphaFit_pt*.png`)
      - a diagnostic plot of the constructed $\alpha\to 0$ correction histogram (`.../alpha_extrap/L3Res_<run>_corr_alpha0_fromkFSR.png`)

4) Build pT-correction points (final per-input correction histogram):
  - If alpha-extrapolation was performed, form the alpha->0 corrected pT points by applying $k_{FSR}$ to the nominal pT histogram (nominal = ratio at reference alpha):
    $$ C(p_T) = R(p_T,\alpha_{ref}) \times k_{FSR}(p_T) $$
    (propagating relative errors from both factors)
    - This corrected histogram is what is used for the final pT fit (unless an already-built `ratio_vspT_alpha0` / `corr_vspT` is provided, in which case `dofits_L3.C` will not apply $k_{FSR}$ again).
  - Alternatively, `deriveL3_from_photonjet.C` can produce `corr_vspT` or `ratio_vspT_alpha0` directly which `dofits_L3.C` will use.

5) Combined pT fit (single script, `dofits_L3.C`):
  - The macro collects all pT-correction histograms from one or more derived ROOT files. For multi-input workflows (e.g., photon+jet + Z+jet), provide a comma-separated list of derived files as the first argument.
  - Optionally provide `inputPtRangesCSV` with comma-separated `lo-hi` ranges so each input contributes only inside its intended pT window.
   - Build a `TMultiGraph` containing cleaned pT points (optional cleaning removes zero/empty points and outside-fit-range bins) and fit a single function of the form used in the L3Residual JEC text file:
     $$ f(p_T) = [0] + [1]\cdot \log_{10}(0.01\,p_T) $$
     - The expression is centralized in `dofits_L3.C` (see `l3PtFitExpr()`) so it can be changed later without touching multiple blocks.
  - The fit parameters are written to a JEC-style text block and a copy is placed under `L3Residual/jecfiles/` with a tag that indicates `photonjet` (single-input) or `combined` (multi-input).
   - The combined pT-fit canvas is drawn in `log-x` and uses a tight y-range centered on unity by default (the macro sets the frame y-range to `[0.97, 1.05]` and reapplies axis styling after the final fit so titles/labels remain visible).
6) Optional L2L3 combination (`writeL2L3=true`):
  - If requested the macro reads an L2Residual text file, samples the L2 shape over each L2 pt range, multiplies with the fitted L3 shape $f(p_T)$, and fits the product to produce a combined L2L3 correction piece for each eta bin.

7) Outputs summary:
  - ROOT: `L3Residual/L3fits_<tag>/<tag>.root` containing graphs, fits and input histograms.
  - PNG/PDF: plots per-alpha, per-eta, combined pT fit in `L3Residual/L3fits_<tag>/pdf/` and raw shapes in `.../raw/`.
  - Text: `L3Residual/.../textfiles/<tag>.txt` plus copies in `L3Residual/jecfiles/L3Residuals_<runLabel>_photonjet_AK4PF.txt` or `_combined_AK4PF.txt`.

Notes:
- `deriveL3_from_photonjet.C` intentionally does **not** write a JEC text file. Text output is produced by `dofits_L3.C` so the final correction always matches the chosen pT fit function, kFSR treatment, and optional multi-input combination.

### Plotting and diagnostic options

- `plotresponse_L3.C`: quick visualization macro that reads a photon+jet analysis output and produces kinematic and balance-distribution plots. Useful flags: `isMC`, `runLabel`, `lumiLabel`, and `useBalanceTH3` to use the `photonjet_balance_dist` if present.
- `photonjet_balance_dist` (TH3D): optional histogram (axes: photon_pT, alpha, balance) storing the full balance distribution per pT/alpha bin. When present `dofits_L3.C` can overlay normalized balance distributions (MC vs Data) using `plotBalanceDistOverlay=true` and the `mcRawFileForDist`/`dataRawFileForDist` arguments. If those raw files are not provided, `dofits_L3.C` **falls back** to using its internal 3D profiles (`balance3D_mc` / `balance3D_data`) to construct per-pT overlays and saves them in `L3Residual/<tag>/raw/balance_dist_overlay/` (PNG per pt bin). The per-α raw shapes are saved into `.../raw/` as `L3Res_<run>_alphaN_raw.png`.
- `dofits_L3.C` flags of interest:
  - `plotRawResponses`: saves MC and Data balance vs pT raw plots in `.../raw/` for QC.
  - `saveAlphaExtrap`: enable alpha->0 extrapolation and kFSR extraction.
  - `applyKFSRToPtFit`: whether to multiply nominal pT ratios with kFSR(pT) before the pT fit.
  - `inputLabelsCSV`, `inputPtRangesCSV`: used when passing multiple derived inputs to annotate and restrict per-input contributions.



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
