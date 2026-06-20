#pragma once

#include "algh_solution.h"
#include "tsplib_graph.h"

alghSolution breathFirstSearch(const TSPLIBGraph& graph, int maxRunningTime, int maxMemoryUsageMB,bool findUpperBound, bool repetetive, bool ties);