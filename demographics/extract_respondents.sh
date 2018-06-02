#!/bin/bash
set -euf -o pipefail

if [ $# -ne 1 ]; then
  echo "Usage: $0 demographics-csv"
  exit 1
fi

SESSIONS_FOR_COUNTRY=/srv/data/mooc/clickstream-hmm/demographics/sessions_for_country.py
EXTRACT_SEQUENCES=/srv/data/mooc/clickstream-hmm/build/extract-sequences

echo "Extracting all respondents..."
python $SESSIONS_FOR_COUNTRY --reside "$1" ".*" > all_respondents.txt
wc -l all_respondents.txt

echo "Extracting se-asia respondents..."
python $SESSIONS_FOR_COUNTRY --reside "$1" "Bangladesh|India|Pakistan|Sri Lanka" > se_asia_respondents.txt
wc -l se_asia_respondents.txt

echo "Constructing sequences for all users (this may take some time)..."
pv clickstream_with_post_and_quiz.json.sorted.gz | zcat | $EXTRACT_SEQUENCES > sequences_10h_all.json

echo "Filtering sequences from all respondents..."
pv sequences_10h_all.json | grep -F -f all_respondents.txt - > sequences_10h_respondents.json

echo "Filtering sequences from se asia respondents..."
pv sequences_10h_all.json | grep -F -f se_asia_respondents.txt - > sequences_10h_se_asia.json

echo "All sequences successfully extracted!"
