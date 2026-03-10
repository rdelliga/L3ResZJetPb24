# Batch

Batch mode is only for histogram production. Merge first, then run the L2, JER, or L3 derivation and fit macros on the merged ROOT file.

## Minimal flow

| Step | Helper | Result |
| --- | --- | --- |
| submit jobs | `batch/submit_condor.py` | many histogram-filler jobs |
| merge shards | `batch/merge_outputs.sh` | one merged ROOT file |
| validate locally | `batch/test_batch.sh` | dry-run submission check |

## Typical submission

```bash
python3 batch/submit_condor.py \
  --era PHOTONHP \
  --input batch/input/filelist_forests_2024ppRef_HP_ALL.txt \
  --input-type filelist \
  --output-dir /path/to/output \
  --output-tag photonjet \
  --files-per-job 50 \
  --analysis PhotonJet
```

## Merge

```bash
./batch/merge_outputs.sh /path/to/output PHOTONHP photonjet
```

## Notes

| Item | Meaning |
| --- | --- |
| input modes | same `era`, `file`, `directory`, and `filelist` modes already supported by the analysis macros |
| filelists | stored under `batch/input/` |
| naming | keep output tags stable so MC and data files are easy to pair later |