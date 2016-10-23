import cairocffi as cairo
from igraph import *
import fileinput
import json
import math

layout = None
for line in fileinput.input():
    nodes = json.loads(line)
    graph = Graph(directed=True)

    for i, node in enumerate(nodes):
        graph.add_vertex(i, label=node['name'], weight=node['init'])

    for i, node in enumerate(nodes):
        for j, edge in enumerate(node['edges']):
            graph.add_edge(i, j, weight=edge)

    plot(graph,
         bbox=(500, 500),
         edge_width=[5 * weight for weight in graph.es['weight']],
         vertex_size=[200 * weight for weight in
             graph.personalized_pagerank(weights='weight', reset='weight')],
         target="trans.png",
         layout=graph.layout_circle(), margin=100)

    break
