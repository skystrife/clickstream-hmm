#!/bin/bash
set -euf -o pipefail

for dir in "$@"
do
    pushd $dir/two-layer

    python ../../plot_models.py states.json
    python ../../model_json_to_csv.py states.json > states.csv

    python ../../plot_trans.py all_trans.json
    mv trans.png all_trans.png
    python ../../model_json_to_csv.py all_trans.json > all_trans.csv

    python ../../plot_trans.py se_asia_trans.json
    mv trans.png se_asia_trans.png
    python ../../model_json_to_csv.py se_asia_trans.json > se_asia_trans.csv

    popd
done
