#!/bin/bash
set -euf -o pipefail

for dir in "$@"
do
    pushd $dir

    python ../plot_models.py plain_mm_all.json
    python ../model_json_to_csv.py plain_mm_all.json > plain_mm_all.csv
    mv state0.png plain_mm_all.png

    python ../plot_models.py plain_mm_respondents.json
    python ../model_json_to_csv.py plain_mm_respondents.json > plain_mm_respondents.csv
    mv state0.png plain_mm_respondents.png

    python ../plot_models.py plain_mm_se_asia.json
    python ../model_json_to_csv.py plain_mm_se_asia.json > plain_mm_se_asia.csv
    mv state0.png plain_mm_se_asia.png

    popd
done
