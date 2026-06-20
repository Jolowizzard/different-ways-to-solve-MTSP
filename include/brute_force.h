#pragma once

#include "algh_solution.h"
#include "tsplib_graph.h"


alghSolution bruteForce(const TSPLIBGraph& graph);

void swap(std::vector<int>& tab, int a, int b);