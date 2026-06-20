#pragma once

#include "tsplib_graph.h"
#include "algh_solution.h"

alghSolution nearestNeighbour(const TSPLIBGraph& graph, int startVertices, bool repetetive, bool randomStart, bool ties);