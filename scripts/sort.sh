#!/bin/bash
set -euf -o pipefail

SORTBIN=/srv/data/mooc/clickstream-hmm/build/sort

rm -rf tmp
if [ $# -ne 1 ]; then
  echo usage: $0 filename
  exit 1
fi

FILENAME=$1
OUTPUT_FILENAME=${FILENAME%.gz}.sorted.gz

if [ -e $OUTPUT_FILENAME ]; then
  echo $OUTPUT_FILENAME already exists!
  exit 1
fi

echo Sorting $FILENAME into $OUTPUT_FILENAME...
pv $FILENAME | zcat | $SORTBIN 4 | pigz > $OUTPUT_FILENAME
