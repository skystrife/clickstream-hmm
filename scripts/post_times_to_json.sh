#!/bin/bash

MYSQL=/srv/data/mooc/mysql.sh
POSTS_TO_JSON=/srv/data/mooc/clickstream-hmm/scripts/post_times_to_json.py

query="select 1000*forum_posts.post_time as post_time, forum_posts.original, \
              hash_mapping.session_user_id \
       FROM forum_posts \
       INNER JOIN hash_mapping \
       ON forum_posts.user_id = hash_mapping.user_id \
       ORDER BY post_time ASC;"

echo "$query" | $MYSQL $1 | python $POSTS_TO_JSON
