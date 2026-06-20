#include "breath_first_search.h"
#include "nearest_neighbour.h"
#include <chrono>
#include <vector>
#include <list>
#include <iostream>
#include <queue>
#include <sys/resource.h> //To zadziała tylko na linuxie
#include "utils.cpp"

// BFS - kolejka FIFO - musi być ograniczenie, co do rozmiaru listy rozpatrywanych obecnie rozwiązań

alghSolution asymetricalBreathFirstSearch(const TSPLIBGraph& graph,
                                          std::chrono::steady_clock::time_point start, 
                                          std::chrono::seconds timeLimit,
                                          int memoryLimitMB,
                                          std::queue<alghSolution>& FIFO,
                                          int& upperBound,
                                          alghSolution solution){
int iterCounter = 0;
long maxMemoryKB = memoryLimitMB * 1024;
while(FIFO.size() > 0){
        if(isLimitExceeded(iterCounter, start, timeLimit, maxMemoryKB)){
            solution.pathWeight = -1;
            return solution;
        }
        auto currentNode = FIFO.front();
        FIFO.pop();
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
                
                //Bez mechanizmu eliminacji tras lustrzanych

                newNode.pathWeight += graph.weight(newNode.path.back(),nextCity);

                newNode.nodesToExplore[j] = newNode.nodesToExplore.back();
                newNode.nodesToExplore.pop_back();

                if(newNode.pathWeight < upperBound){
                    newNode.path.push_back(nextCity);
                    FIFO.push(newNode);
                }
            }
        }
    }
    return solution;
}

alghSolution symetricalBreathFirstSearch(const TSPLIBGraph& graph,
                                          std::chrono::steady_clock::time_point start, 
                                          std::chrono::seconds timeLimit,
                                          int memoryLimitMB,
                                          std::queue<alghSolution>& FIFO,
                                          int& upperBound,
                                          alghSolution solution){

int iterCounter = 0;
long maxMemoryKB = memoryLimitMB * 1024;
while(FIFO.size() > 0){
        if(isLimitExceeded(iterCounter, start, timeLimit, maxMemoryKB)){
            solution.pathWeight = -1;
            return solution;
        }
        auto currentNode = FIFO.front();
        FIFO.pop();
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
                    FIFO.push(newNode);
                }
            }
        }
    }
    return solution;
}

alghSolution breathFirstSearch(const TSPLIBGraph& graph, int maxRunningTime, int maxMemoryUsageMB, bool findUpperBound, bool repetetive, bool ties){
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

    std::queue<alghSolution> FIFO;
    alghSolution pierwszy;
    pierwszy.nodesToExplore = wierzcholekStartowy;
    pierwszy.path = trasaPoczatkowa;
    pierwszy.pathWeight = 0;
    pierwszy.vistedNode1 = false;
    FIFO.push(pierwszy);

    //Rozróżenie do łatwiejszego testowania
    if(graph.isATSP()){
        solution = asymetricalBreathFirstSearch(graph,start,timeLimit,maxMemoryUsageMB,FIFO,upperBound,solution);
    }
    else{
        solution = symetricalBreathFirstSearch(graph,start,timeLimit,maxMemoryUsageMB,FIFO,upperBound,solution);
    }
    return solution;

}