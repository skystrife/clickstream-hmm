#!/bin/bash
set -euf -o pipefail

if [ $# -ne 2 ]; then
  echo "Usage: $0 db-name clickstream-file.gz"
  echo "This should be run from the folder where you wish the data to reside"
  exit 1
fi

SCRIPTDIR=/srv/data/mooc/clickstream-hmm/scripts
POST_TIMES_TO_JSON=$SCRIPTDIR/post_times_to_json.sh
QUIZ_TIMES_TO_JSON=$SCRIPTDIR/quiz_times_to_json.sh
SORTSH=$SCRIPTDIR/sort.sh

echo "Extracting post times..."
$POST_TIMES_TO_JSON $1 > post-times.json
echo "Extracting quiz times..."
$QUIZ_TIMES_TO_JSON $1 > quiz-times.json
echo "Compressing quiz and post times..."
pigz post-times.json quiz-times.json
echo "Incorporating quiz and post times into clickstream..."
pv $2 | zcat - post-times.json.gz quiz-times.json.gz | pigz > clickstream_with_post_and_quiz.json.gz
echo "Sorting..."
$SORTSH clickstream_with_post_and_quiz.json.gz
rm clickstream_with_post_and_quiz.json.gz
echo "Done!"
