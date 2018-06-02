import json
import csv
import fileinput
import sys

for line in fileinput.input():
    nodes = json.loads(line)

    writer = csv.writer(sys.stdout)
    writer.writerow(['name', 'init'] + [node['name'] for node in nodes])

    for node in nodes:
        writer.writerow([node['name'], node['init']] + [edge for edge in node['edges']])
