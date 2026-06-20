#pragma once

#include "algh_solution.h"
#include "tsplib_graph.h"

alghSolution depthFirstSearch(const TSPLIBGraph& graph, int maxRunningTime, int maxMemoryUsage, bool findUpperBound, bool repetetive, bool ties);