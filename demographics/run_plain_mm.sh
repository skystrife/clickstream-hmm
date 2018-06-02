#!/bin/bash
set -euf -o pipefail

PLAIN_MM=/srv/data/mooc/clickstream-hmm/build/plain-mm

for dir in "$@"
do
  echo "Processing sequences in $dir..."
  pushd $dir
  mkdir -p results
  pv sequences_10h_all.json | $PLAIN_MM > results/plain_mm_all.json
  pv sequences_10h_respondents.json | $PLAIN_MM > results/plain_mm_respondents.json
  pv sequences_10h_se_asia.json | $PLAIN_MM > results/plain_mm_se_asia.json
  echo "Done with $dir..."
  popd
done
