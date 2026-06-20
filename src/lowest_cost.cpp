#include "lowest_cost.h"
#include <chrono>
#include <vector>
#include <queue>
#include "nearest_neighbour.h"
#include <iostream>
#include "utils.cpp"

struct CompareSolution {
    bool operator()(const alghSolution& a, const alghSolution& b){
        return a.pathWeight > b.pathWeight; //zwraca true, jeżeli a jest większe od b, co znaczy, że jeżeli rodzic jest większy
        //to zamień miejscami - wtedy mniejsze wartości idą do góry drzewa
    }
};

alghSolution symetricalLowestCost(const TSPLIBGraph& graph,
                                          std::chrono::steady_clock::time_point start, 
                                          std::chrono::seconds timeLimit,
                                          int memoryLimitMB,
                                          std::priority_queue<alghSolution, std::vector<alghSolution>, CompareSolution>& minHeap,
                                          int& upperBound,
                                          alghSolution solution){
int iterCounter = 0;
long maxMemoryKB = memoryLimitMB * 1024;
 while(minHeap.size() > 0){
        if(isLimitExceeded(iterCounter, start, timeLimit, maxMemoryKB)){
            solution.pathWeight = -1;
            return solution;
        }
        auto currentNode = minHeap.top();
        minHeap.pop();
        int verticiesToExploreCount = currentNode.nodesToExplore.size();
        if(verticiesToExploreCount == 0){
            int finalWeight = currentNode.pathWeight + graph.weight(currentNode.path.back(),currentNode.path.front());
            if(finalWeight < upperBound){
                upperBound = finalWeight;
                currentNode.pathWeight = finalWeight;
                solution = currentNode;
            }
        }
        else{
            for(int j = 0; j < verticiesToExploreCount; j++){
                alghSolution newNode = currentNode;

                int nextCity = newNode.nodesToExplore[j];
                //Mechanizm eliminacji tras lustrzanych
                if(nextCity == 2 && newNode.vistedNode1 == true){
                    continue;
                }
                else if(nextCity == 1){
                    newNode.vistedNode1 = true;
                }
                newNode.pathWeight += graph.weight(newNode.path.back(),nextCity);

                newNode.nodesToExplore[j] = newNode.nodesToExplore.back();
                newNode.nodesToExplore.pop_back();

                if(newNode.pathWeight < upperBound){
                    newNode.path.push_back(nextCity);
                    minHeap.push(newNode);
                }
            }
        }
    }
    return solution;
}

alghSolution asymetricalLowestCost(const TSPLIBGraph& graph,
                                          std::chrono::steady_clock::time_point start, 
                                          std::chrono::seconds timeLimit,
                                          int memoryLimitMB,
                                          std::priority_queue<alghSolution, std::vector<alghSolution>, CompareSolution>& minHeap,
                                          int& upperBound,
                                          alghSolution solution){
int iterCounter = 0;
long maxMemoryKB = memoryLimitMB * 1024;
 while(minHeap.size() > 0){
        if(isLimitExceeded(iterCounter, start, timeLimit, maxMemoryKB)){
            solution.pathWeight = -1;
            return solution;
        }
        auto currentNode = minHeap.top();
        minHeap.pop();
        int verticiesToExploreCount = currentNode.nodesToExplore.size();
        if(verticiesToExploreCount == 0){
            int finalWeight = currentNode.pathWeight + graph.weight(currentNode.path.back(),currentNode.path.front());
            if(finalWeight < upperBound){
                upperBound = finalWeight;
                currentNode.pathWeight = finalWeight;
                solution = currentNode;
            }
        }
        else{
            for(int j = 0; j < verticiesToExploreCount; j++){
                alghSolution newNode = currentNode;

                int nextCity = newNode.nodesToExplore[j];
                //Mechanizm eliminacji tras lustrzanych
                if(nextCity == 2 && newNode.vistedNode1 == true){
                    continue;
                }
                else if(nextCity == 1){
                    newNode.vistedNode1 = true;
                }
                newNode.pathWeight += graph.weight(newNode.path.back(),nextCity);

                newNode.nodesToExplore[j] = newNode.nodesToExplore.back();
                newNode.nodesToExplore.pop_back();

                if(newNode.pathWeight < upperBound){
                    newNode.path.push_back(nextCity);
                    minHeap.push(newNode);
                }
            }
        }
    }
    return solution;
}

alghSolution lowestCost(const TSPLIBGraph& graph, int maxRunningTime, int maxMemoryUsageMB, bool findUpperBound, bool repetetive, bool ties){
    using clock = std::chrono::steady_clock;

    auto start = clock::now();
    auto timeLimit = std::chrono::seconds(maxRunningTime);

    int upperBound = __INT_MAX__; //początkowo górne ograniczenie jest równe nieskończoność
    alghSolution solution;

    if(findUpperBound){
        solution = nearestNeighbour(graph, 1, repetetive, false, ties); // zapisanie wagi upperBound
        upperBound = solution.pathWeight;
        std::cout << "Upper Bound Found" << std::endl;
    }

    int liczbaWierzcholkow = graph.size();
    std::vector<int> wierzcholekStartowy;
    std::vector<int> trasaPoczatkowa;

    //Załadowanie wszystkich wierzchołków, oprócz pierwszego, gdyż jest on wierzchołkiem początkowym
    for(int i = (liczbaWierzcholkow - 2); i >= 0 ; i--){
        wierzcholekStartowy.push_back(i);
    }
    trasaPoczatkowa.push_back(liczbaWierzcholkow-1);
    //kolejność powinna być - 5 - 4 - 3 - 2 - 1 - 0 (dla grafu o 7 wierzchołkach), wtedy można wykorzystać pop_back, przy zmniejszaniu listy, co działa w czasie O(1)

    alghSolution pierwszy;
    pierwszy.path = trasaPoczatkowa;
    pierwszy.nodesToExplore = wierzcholekStartowy;
    pierwszy.vistedNode1 = false;
    pierwszy.pathWeight = 0;

    std::priority_queue<alghSolution, std::vector<alghSolution>, CompareSolution> minHeap;
    minHeap.push(pierwszy);
    
    if(graph.isATSP()){
        solution = asymetricalLowestCost(graph, start, timeLimit, maxMemoryUsageMB, minHeap, upperBound, solution);
    }
    else{
        solution = symetricalLowestCost(graph, start, timeLimit, maxMemoryUsageMB, minHeap, upperBound, solution);
    }

    return solution;
}