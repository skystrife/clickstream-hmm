import json
import fileinput

first = True
for line in fileinput.input():
    # skip the header
    if first:
        first = False
        continue

    cols = line.strip().split('\t')

    # output some hallucinated clickstream event in the format we will
    # eventually expect
    print(json.dumps({
        "key": "pageview",
        "value": "https://www.coursera.org/quiz/submit",
        "username": cols[1],
        "timestamp": int(cols[0])
    }))
