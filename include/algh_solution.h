#pragma once

#include <vector>

struct alghSolution
{
    std::vector<int> path;
    double pathWeight;
    bool vistedNode1;
    std::vector<int> solutionsInTime; // Wykorzystywany do śledzenia zmian rozwiązań w czasie
    std::vector<int> nodesToExplore;
    long int interationsTotal;
    int tabuListSize;
    int cadentionTime;
};