#!/bin/bash

MYSQL=/srv/data/mooc/mysql.sh
QUIZ_TO_JSON=/srv/data/mooc/clickstream-hmm/scripts/quiz_times_to_json.py

query="select 1000*submission_time as submission_time_ms, session_user_id
       FROM quiz_submission_metadata \
       ORDER BY submission_time_ms ASC;"

echo "$query" | $MYSQL $1 | python $QUIZ_TO_JSON
