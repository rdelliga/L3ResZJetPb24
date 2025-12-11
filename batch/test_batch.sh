#!/bin/bash
# Test batch submission system

set -e

BASE_DIR="/eos/home-b/bharikri/lxplus_private/EGamma/residualanalysis"
TEST_OUTPUT="/eos/cms/store/group/phys_heavyions/bharikri/JetMinPOG/L3ResPhotonJet/batch_test"

echo "Creating test output directory..."
mkdir -p $TEST_OUTPUT

echo "Running dry-run batch submission (3 jobs, 2 files each)..."
python3 $BASE_DIR/batch/submit_condor.py \
    --era TEST_BATCH \
    --input /eos/cms/store/group/phys_heavyions/nbarnett/Forests/2024ppRef/PPRefHardProbes0 \
    --input-type directory \
    --output-dir $TEST_OUTPUT \
    --output-tag test_v1 \
    --files-per-job 2 \
    --events-per-job 500 \
    --analysis PhotonJet \
    --dry-run

echo ""
echo "Dry run successful!"
echo "To actually submit, remove --dry-run flag"
