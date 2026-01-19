# Residual Analysis Framework

Framework for JEC residual corrections (L2, L3) and JER scale factors for CMS Heavy Ion physics.

## Prerequisites

- CMSSW environment (tested with CMSSW_15_1_0_patch3)
- ROOT (provided by CMSSW)
- HTCondor (for batch processing)

## Quick Start

### Setup Environment
```bash
cd /eos/home-b/bharikri/lxplus_private/EGamma/CMSSW_15_1_0_patch3/src
cmsenv
cd /eos/home-b/bharikri/lxplus_private/EGamma/residualanalysis/fillhistograms
```

### Compile Support Classes
```bash
root -l -b -q compile.C
```

### Run Analysis

#### Single File (Legacy Era Mode)
```bash
root -l -b -q 'analyse_PhotonJet.cc("PHOTONHP", "output_tag", false, false)'
```

#### Directory Mode (Multiple Files)
```bash
root -l -b -q 'analyse_PhotonJet.cc("/path/to/directory", "output_tag", false, false, "directory", 100, 10000, "/output/dir")'
```

#### Filelist Mode
```bash
root -l -b -q 'analyse_PhotonJet.cc("/path/to/filelist.txt", "output_tag", false, false, "filelist", -1, -1, "/output/dir")'
```

### Batch Processing

See batch/README.md for HTCondor batch submission.

```bash
python3 batch/submit_condor.py \
    --era PHOTONHP_FULL \
    --input /path/to/input/directory \
    --output-dir /path/to/output \
    --files-per-job 50 \
    --analysis PhotonJet
```

## Analysis Scripts

| Script | Purpose | Output |
|--------|---------|--------|
| analyse.cc | Dijet analysis | L2 residual histograms |
| analyse_PhotonJet.cc | Photon+Jet analysis | L3 residual histograms |
| analyse_JER.cc | JER scale factor | JER SF histograms |
| dofits_L3.C | Fit L3 residuals, optional raw MC/Data and alpha-extrapolation plots | L3 text/plots |

### dofits_L3 helper flags
- `plotRawResponses` (default false): also draw MC and Data balance vs pT (no ratio/fits) in `L3fits_<tag>/raw/`.
- `saveAlphaExtrap` (default false): build MC/Data/ratio vs alpha for each (pT,|eta|), fit a linear alpha→0 extrapolation, and save PNGs in `L3fits_<tag>/alpha_extrap/`.

Example:
```bash
root -l -b -q 'dofits_L3.C("L3_derived.root", 30, 150, "L3Res_photonjet", false, "2024ppRef", "pp Reference", true, true)'
```

## Function Parameters

```cpp
void analyse_PhotonJet(
    string input,           // Era name, file path, directory, or filelist
    string outputfiletag,   // Tag for output filename
    bool isMC,              // Is Monte Carlo
    bool checkjetid,        // Apply jet ID cuts
    string inputType,       // "era", "file", "directory", "filelist"
    int maxFiles,           // Max files to process (-1 = all)
    int maxEvents,          // Max events to process (-1 = all)
    string outputDir,       // Output directory
    int batchIndex,         // Batch job index (-1 = non-batch)
    int totalBatches        // Total number of batches
)
```

## Directory Structure

```
residualanalysis/
├── fillhistograms/     # Main analysis code
│   ├── analyse*.cc     # Analysis scripts
│   ├── histograms.h/C  # Histogram definitions
│   ├── input_config.h  # Multi-file input utilities
│   ├── chain_builder.h # TChain construction
│   └── jecfiles/       # JEC correction files
├── batch/              # Batch submission
├── L2Residual/         # L2 derivation
├── L3Residual/         # L3 derivation
└── JER/                # JER analysis
```

## Configuration

- configurations.h - Era-to-file mapping (legacy, still supported)
- settings.h - Global settings (JEC files, binning, cuts)

At the moment direct balance method with tag-and-probing a dijet system is used.

## Required steps:
```
cd fillhistograms/jecfiles/
wget https://github.com/cms-jet/JECDatabase/blob/master/jet_veto_maps/Summer23BPixPrompt23/Summer23BPixPrompt23_RunD_v1.root
wget https://indico.cern.ch/event/1592138/contributions/6711482/attachments/3141457/5575822/Spring23_HI_V1_MC_L2Relative_AK4PF.txt
```

## Quick start

-> First: root -l compile.C 
-> 

Then, to run the analysis:

1. analyse.cc fills histograms you need.
Flags:

At the moment this is run with hadded ntuples per dat set (MC, HP0, HP1, HP2, ZB0, etc.)

You can do a test run with simply root -l analyse.cc

-> to run locally: run.sh

1. To fill histograms for L2residuals:

- Code is in directory L2Residual.

-> call this code separarely for different datasets. you need cmsenv to be able to call functions to apply jec (and jer sf). shouldn't really matter which cmssw you use.
-> in settings.h you can manually set which JEC files to use. The JEC (and JER SF) files are always read from the path set up in here.


When you have the output from the former
2. run deriveL2_from3D.C

3. Fit the response ratios vs. alpha: dofits.C

4. plotresponses.C can be used to plot the responses

5. Produce txt files:


JET pT/eta/phi RESOLUTION AND JER SF:

To fill histograms for JER SF (the tag-and-probe conditions are slightly different): turn on flag "" in analyse.cc
-> you need to apply the L2 residual JEC before deriving the SF, remember check that too


When you have the outputs from that, you can run MC-checks:
--- JER/MCJER.C -> pt resolution
--- JER/MCJPR.C -> eta/phi resolition (wip?)
--- JER/MCRESP.C -> this plots the MC response <pT(reco)/pT(gen)>
--- doTxtMCJER.C -> print txt files of resolution fit parameters

For SF there are a couple of scripts:
1. These are alternatives:
--- JERSF_fits.C -> Extracts resoluiton by fitting gaussian to the dijet asymmetry distributions
--- JERSF_RMS.C -> Extracts resolution from trunct RMS of dijet asymmetry distributions
2. Do fits against alpha:
--- JERSF_fits_vsalpha.C -> this needs as an input the output from the previous step
3. Produce txt files
... JERSF_printtxt.C

To look at trigger turn-ons:
(this is has been used to merge results from different datasets/triggers, for 2023 it has been zero bias and hard probes datasets)

Plot things:
