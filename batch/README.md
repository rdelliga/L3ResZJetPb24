# Batch Submission for Residual Analysis

## Quick Start

### Submit jobs
```bash
python3 batch/submit_condor.py \
    --era PHOTONHP_FULL \
    --input /eos/cms/store/group/phys_heavyions/nbarnett/Forests/2024ppRef/PPRefHardProbes0 \
    --input-type directory \
    --output-dir /eos/cms/store/group/phys_heavyions/bharikri/JetMinPOG/L3ResPhotonJet/batch_output \
    --output-tag photonjet_v1 \
    --files-per-job 50 \
    --analysis PhotonJet
```

### Monitor jobs
```bash
condor_q
```

### Merge outputs
```bash
./batch/merge_outputs.sh /path/to/output PHOTONHP_FULL photonjet_v1
```

## Options

| Option | Description | Default |
|--------|-------------|---------|
| `--era` | Era/dataset name for output naming | Required |
| `--input` | Input path (directory/file/filelist) | Required |
| `--input-type` | `directory`, `file`, or `filelist` | `directory` |
| `--output-dir` | Output directory | Required |
| `--output-tag` | Tag for output files | `batch` |
| `--files-per-job` | Files processed per job | 50 |
| `--events-per-job` | Max events per job (-1=all) | -1 |
| `--mc` | Process as MC | False |
| `--jet-tree` | Jet tree path (e.g. `ak4PFJetAnalyzer/t`, `ak4PFJetAnalyzerSDZcut1/t`) | `ak4PFJetAnalyzer/t` |
| `--analysis` | `PhotonJet`, `Dijet`, or `JER` | `PhotonJet` |
| `--flavour` | HTCondor job duration | `workday` |
| `--dry-run` | Create files without submitting | False |

## Available Jet Trees

**Data (2024 ppRef):** `ak4PFJetAnalyzer/t`

**MC (QCD-Photon):** Multiple radii available:
- `ak2PFJetAnalyzer/t`, `ak2PFJetAnalyzerSDZcut1/t`, etc.
- `ak4PFJetAnalyzerSDZcut1/t`, `ak4PFJetAnalyzerSDZcut2/t`, etc.
- `ak6PFJetAnalyzerSDZcut1/t`, `ak8PFJetAnalyzerSDZcut1/t`, etc.

## Output Files

Individual job outputs are named:
```
{era}_{tag}_batch{N}_of_{M}.root
```

After merging:
```
{era}_{tag}_merged.root
```
