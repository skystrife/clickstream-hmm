#!/bin/bash
set -euf -o pipefail

BUILD_DIR=/srv/data/mooc/clickstream-hmm/build

CLICKSTREAM_HMM=$BUILD_DIR/clickstream-hmm
RETROFIT_HMM=$BUILD_DIR/retrofit-hmm
PRINT_HMM=$BUILD_DIR/print-hmm

for dir in "$@"
do
  echo "Processing sequences in $dir..."

  mkdir -p $dir/results/two-layer

  pushd $dir/results/two-layer
  cat ../../sequences_10h_all.json | $CLICKSTREAM_HMM 4
  cat ../../sequences_10h_se_asia.json | $RETROFIT_HMM hmm-model.gz hmm-model_se_asia.gz
  $PRINT_HMM json hmm-model.gz > states.json
  $PRINT_HMM json-trans hmm-model.gz > all_trans.json
  $PRINT_HMM json-trans hmm-model_se_asia.gz > se_asia_trans.json
  popd

  echo "Done with $dir..."
done
