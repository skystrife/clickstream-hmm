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

    output = {"key": "pageview",
              "username": cols[2],
              "timestamp": int(cols[0])}

    if cols[1] == "1":
        output["value"] = "https://www.coursera.org/forum/posted_thread"
    else:
        output["value"] = "https://www.coursera.org/forum/posted_reply"

    print(json.dumps(output))
