#!/bin/bash
# Merge batch output files using hadd
# Usage: ./merge_outputs.sh <INPUT_DIR> <era> <tag>

INPUT_DIR=$1
ERA=$2
TAG=$3
OUTPUT_DIR=$4


if [ -z "$INPUT_DIR" ] || [ -z "$ERA" ] || [ -z "$TAG" ]; then
    echo "Usage: $0 <INPUT_DIR> <era> <tag>"
    echo "Example: $0 /path/to/output PHOTONHP_FULL photonjet_v1"
    exit 1
fi

cd "$INPUT_DIR" || exit 1

# Find all batch files matching pattern
PATTERN="${ERA}_${TAG}_batch*_of_*.root"
BATCH_FILES=$(ls $PATTERN 2>/dev/null)

if [ -z "$BATCH_FILES" ]; then
    echo "ERROR: No batch files found matching: $PATTERN"
    exit 1
fi

NFILES=$(echo "$BATCH_FILES" | wc -w)
echo "Found $NFILES batch files to merge"

mkdir -p "$OUTPUT_DIR"

# Create merged output
MERGED="${OUTPUT_DIR}/${ERA}_${TAG}_merged.root"
echo "Merging to: $MERGED"

hadd -f "$MERGED" $BATCH_FILES

if [ $? -eq 0 ]; then
    echo ""
    echo "SUCCESS: Merged output: ${OUTPUT_DIR}/${MERGED}"
    echo ""
    echo "To remove batch files:"
    echo "  rm ${INPUT_DIR}/${PATTERN}"
else
    echo "ERROR: hadd failed!"
    exit 1
fi
