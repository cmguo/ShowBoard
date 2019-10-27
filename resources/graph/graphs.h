#ifndef GRAPHS_H
#define GRAPHS_H

#include "graph2d.h"
#include "circle.h"
#include "ellipse.h"

REGISTER_RESOURCE_VIEW_FACTORY(Graph2DFactory, Graph2D, "graph2d")
REGISTER_GRAPH_2D(Ellipse, "ellipse")
REGISTER_GRAPH_2D(Circle, "circle")

#endif // GRAPHS_H
