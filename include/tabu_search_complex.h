#pragma once
#include "tabu_search_basic.h"

//Algorytm różni się od podstawowego : innym warunkiem akceptacji, addytywnym systemem kadencji (można jeszcze pomyśleć nad multiplikatywnym)
alghSolution tabuSearchComplexAlgorithm(const TSPLIBGraph& graph, int timeLimit, int cadentionTime, bool computeUB, int iterationsWithNoImprovement, float acceptanceLevel, int tabuListSize, int iterationWithNoImprovementToChangeSize, int opt);
