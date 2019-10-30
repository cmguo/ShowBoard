#ifndef GRAPHS_H
#define GRAPHS_H

#include "graph2d.h"
#include "circle.h"
#include "ellipse.h"
#include "triangle.h"
#include "righttriangle.h"
#include "rectangle.h"
#include "square.h"
#include "polygon.h"

REGISTER_RESOURCE_VIEW_FACTORY(Graph2DFactory, Graph2D, "graph2d")
REGISTER_GRAPH_2D(Ellipse, "ellipse")
REGISTER_GRAPH_2D(Circle, "circle")
REGISTER_GRAPH_2D(Triangle, "triangle")
REGISTER_GRAPH_2D(RightTriangle, "righttriangle")
REGISTER_GRAPH_2D(Rectangle, "rectangle")
REGISTER_GRAPH_2D(Square, "square")
REGISTER_GRAPH_2D(Polygon, "polygon")

#endif // GRAPHS_H
