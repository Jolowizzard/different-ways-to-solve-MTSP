#pragma once

#include "algh_solution.h"
#include "tsplib_graph.h"

alghSolution branchAndBound(int lookingMethod, const TSPLIBGraph& graph);