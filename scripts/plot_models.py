import cairocffi as cairo
from igraph import *
import fileinput
import json
import math

g = 0
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
         bbox=(1000, 1000),
         edge_width=[5 * weight for weight in graph.es['weight']],
         vertex_size=[200 * weight for weight in
             graph.personalized_pagerank(weights='weight', reset='weight')],
         target="state{}.png".format(g),
         layout=graph.layout_circle(),
         margin=(120, 100, 110, 100),
         vertex_label_dist=1.5,
         vertex_label_size=30,
         vertex_label_angle=-math.pi/2)

    g += 1
