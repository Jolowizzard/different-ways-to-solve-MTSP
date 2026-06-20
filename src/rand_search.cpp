#include "rand_search.h"
#include <chrono>
#include <iostream>

//Istnieje pare sposobów na generowanie "losowych" ścieżek.
alghSolution RAND(const TSPLIBGraph& graph, int runningTime){
    using clock = std::chrono::steady_clock;

    auto start = clock::now();
    auto timeLimit = std::chrono::seconds(runningTime);
    auto nextCheckpoint = start + std::chrono::seconds(20);

    alghSolution solution;
    solution.pathWeight = __INT_MAX__;
    int n = graph.size();
    int pathWeight = 0;

    std::uniform_int_distribution<> dist(0,n-1);
    using param_t = std::uniform_int_distribution<int>::param_type;

    std::vector<int> progress;
    std::vector<int> openVertices(n);
    std::vector<int> path(n);
    std::vector<int> bestPath(n);

    for(int i = 0; i < n; i++)
        openVertices[i] = i;

    while(clock::now() - start < timeLimit){
        std::vector<int> copyOpenVertices = openVertices;

        //Wygenerowanie losowej ścieżki
        for(int i = 0; i < n; i++){
            dist.param(param_t(0,(n-1-i)));// Ustawienie generatora na zakres odpowiedni dla liczby wierzchołków
            int randomVertex = dist(gen);
            path[i] = copyOpenVertices[randomVertex];

            copyOpenVertices[randomVertex] = copyOpenVertices[n-1-i];// Zastąpienie wylosowanego wierzchołka, ostatnim wierzchołkiem z tablicy
            //Nie trzeba przechowywać informacji o tym wierzchołku, gdyż został on już wylosowany
        }

        pathWeight = calculatePathWeight(graph, path);

        if(pathWeight < solution.pathWeight){
            solution.pathWeight = pathWeight;
            solution.path = path;
        }
        auto now = clock::now();
        if(now >= nextCheckpoint){
            progress.push_back(solution.pathWeight);
            nextCheckpoint += std::chrono::seconds(20);
        }
    }

    solution.solutionsInTime = progress;

    return solution;
}
