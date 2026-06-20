#include "depth_first_search.h"
#include <chrono>
#include "nearest_neighbour.h"
#include <iostream>
#include "utils.cpp"

void tspDFS(const TSPLIBGraph& graph,
                  int currentCity,
                  int currentWeight, 
                  std::vector<int>& currentPath, 
                  std::vector<bool>& visited, 
                  double& upperBound, 
                  std::vector<int>& bestPath,
                  std::chrono::steady_clock::time_point start, 
                  std::chrono::seconds timeLimit, 
                  bool& outOfTime,
                  int& iterCount){
    
    if(outOfTime){
        return;
    }
    iterCount++;
    if ((iterCount & 4095) == 0) {
        if (std::chrono::steady_clock::now() - start > timeLimit) {
            outOfTime = true;
            return;
        }
    }
    int n = graph.size();

    // Baza rekurencji: jeśli długość ścieżki jest równa liczbie miast, zamykamy cykl
    if (currentPath.size() == n) {
        int finalWeight = currentWeight + graph.weight(currentCity, currentPath.front());
        if (finalWeight < upperBound) {
            upperBound = finalWeight;
            bestPath = currentPath; // To JEDYNE miejsce w kodzie, gdzie kopiujemy cały wektor ścieżki!
        }
        return;
    }

    // Iterujemy po wszystkich wierzchołkach, próbując dobrać kolejne miasto
    for (int nextCity = 0; nextCity < n; nextCity++) {
        if (!visited[nextCity]) {
            
            // Mechanizm eliminacji tras lustrzanych
            // Skoro mamy tablicę 'visited', nie potrzebujemy osobnej flagi!
            // Jeśli próbujemy wstawić miasto '2', ale miasto '1' jest już odwiedzone - tniemy.
            if (nextCity == 2 && visited[1] == true) {
                continue;
            }

            int nextWeight = currentWeight + graph.weight(currentCity, nextCity);

            // Branch & Bound: wchodzimy głębiej tylko, jeśli ma to sens
            if (nextWeight < upperBound) {
                
                // --- KROK DO PRZODU ---
                visited[nextCity] = true;
                currentPath.push_back(nextCity);

                // --- REKURENCJA ---
                tspDFS(graph, nextCity, nextWeight, currentPath, visited, upperBound, bestPath, start, timeLimit, outOfTime, iterCount);

                // --- NAWRÓT (BACKTRACKING) ---
                // Tu dzieje się magia: po powrocie z głębszej gałęzi, usuwamy miasto 
                // ze ścieżki i oznaczamy jako nieodwiedzone, by pętla mogła spróbować innego.
                currentPath.pop_back();
                visited[nextCity] = false;
            }
        }
    }

}

alghSolution symetricalDepthFirstSearch(const TSPLIBGraph& graph,
                                          std::chrono::steady_clock::time_point start, 
                                          std::chrono::seconds timeLimit,
                                          int memoryLimitMB,
                                          std::vector<alghSolution>& LIFO,
                                          int& upperBound,
                                          alghSolution solution){
int iterCounter = 0;
long maxMemoryKB = memoryLimitMB * 1024;
while(LIFO.size() > 0){
        if(isLimitExceeded(iterCounter, start, timeLimit, maxMemoryKB)){
            solution.pathWeight = -1;
            return solution;
        }
        alghSolution current = LIFO.back();
        LIFO.pop_back();

        int nodesToExploreCount = current.nodesToExplore.size();
        if(nodesToExploreCount == 0){
            int finalWeight = current.pathWeight + graph.weight(current.path.back(), current.path.front());
            if(finalWeight < upperBound){
                current.pathWeight = finalWeight;
                upperBound = finalWeight;
                solution = current;
            }
        }
        else{
            for(int j = 0; j < nodesToExploreCount; j++){
                alghSolution next = current;
                
                int nextCity = next.nodesToExplore[j];

                //Mechanizm eliminacji tras lustrzanych
                if(nextCity == 2 && next.vistedNode1 == true){
                    continue;
                }
                else if(nextCity == 1){
                    next.vistedNode1 = true;
                }

                next.pathWeight += graph.weight(next.path.back(), nextCity);

                next.nodesToExplore[j] = next.nodesToExplore.back();
                next.nodesToExplore.pop_back();

                if(next.pathWeight < upperBound){
                    next.path.push_back(nextCity);
                    LIFO.push_back(next);
                }

            }
        }
    }
    return solution;
}

alghSolution asymetricalDepthFirstSearch(const TSPLIBGraph& graph,
                                          std::chrono::steady_clock::time_point start, 
                                          std::chrono::seconds timeLimit,
                                          int memoryLimitMB,
                                          std::vector<alghSolution>& LIFO,
                                          int& upperBound,
                                          alghSolution solution){
int iterCounter = 0;
long maxMemoryKB = memoryLimitMB * 1024;
while(LIFO.size() > 0){
        if(isLimitExceeded(iterCounter, start, timeLimit, maxMemoryKB)){
            solution.pathWeight = -1;
            return solution;
        }
        alghSolution current = LIFO.back();
        LIFO.pop_back();

        int nodesToExploreCount = current.nodesToExplore.size();
        if(nodesToExploreCount == 0){
            int finalWeight = current.pathWeight + graph.weight(current.path.back(), current.path.front());
            if(finalWeight < upperBound){
                current.pathWeight = finalWeight;
                upperBound = finalWeight;
                solution = current;
            }
        }
        else{
            for(int j = 0; j < nodesToExploreCount; j++){
                alghSolution next = current;
                
                int nextCity = next.nodesToExplore[j];

                //Brak mechanizmu do eliminacji tras lustrzanych

                next.pathWeight += graph.weight(next.path.back(), nextCity);

                next.nodesToExplore[j] = next.nodesToExplore.back();
                next.nodesToExplore.pop_back();

                if(next.pathWeight < upperBound){
                    next.path.push_back(nextCity);
                    LIFO.push_back(next);
                }

            }
        }
    }
    return solution;
}

alghSolution depthFirstSearch(const TSPLIBGraph& graph, int maxRunningTime, int maxMemoryUsageMB, bool findUpperBound, bool repetetive, bool ties){
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

    // int liczbaWierzcholkow = graph.size();
    
    // // Przygotowujemy globalne struktury dla rekurencji
    // std::vector<int> currentPath;
    // std::vector<int> bestPath;
    // std::vector<bool> visited(liczbaWierzcholkow, false); // Zastępuje Twój 'nodesToExplore'
    // bool outOfTime = false;
    // int iterCounter = 0;

    // // Ustawiamy wierzchołek startowy (zgodnie z Twoim wcześniejszym kodem to ostatni element)
    // int startCity = liczbaWierzcholkow - 1;
    // currentPath.push_back(startCity);
    // visited[startCity] = true;

    // // Startujemy rekurencję z miasta początkowego, obecna waga = 0
    // tspDFS(graph, startCity, 0, currentPath, visited, upperBound, bestPath, start, timeLimit, outOfTime, iterCounter);

    // // Po zakończeniu pakujemy wynik
    // if (outOfTime) {
    //     std::cout << "Out of time limit" << std::endl;
    //     solution.pathWeight = -1;
    // } else {
    //     solution.path = bestPath;
    //     solution.pathWeight = upperBound;
    // }

    // return solution;

    //Załadowanie wszystkich wierzchołków, oprócz pierwszego, gdyż jest on wierzchołkiem początkowym
    for(int i = (liczbaWierzcholkow - 2); i >= 0 ; i--){
        wierzcholekStartowy.push_back(i);
    }
    trasaPoczatkowa.push_back(liczbaWierzcholkow-1);
    //kolejność powinna być - 5 - 4 - 3 - 2 - 1 - 0 (dla grafu o 7 wierzchołkach), wtedy można wykorzystać pop_back, przy zmniejszaniu listy, co działa w czasie O(1)

    alghSolution pierwszy;
    pierwszy.path = trasaPoczatkowa;
    pierwszy.nodesToExplore = wierzcholekStartowy;
    pierwszy.pathWeight = 0;
    pierwszy.vistedNode1 = false;
    std::vector<alghSolution> LIFO;
    LIFO.push_back(pierwszy);

    if(graph.isATSP()){
        std::cout << "sprawdzam ATSP" << std::endl;
        solution = asymetricalDepthFirstSearch(graph,start,timeLimit,maxMemoryUsageMB,LIFO,upperBound,solution);
    }
    else{
        solution = symetricalDepthFirstSearch(graph,start,timeLimit,maxMemoryUsageMB,LIFO,upperBound,solution);
    }
    // pętla do kontrolowania czasu wykonywania algorytmu
    //Sam algorytm nie różni się bradzo od algorytmu BFS, tylko miejsce, skąd brane są węzły
    return solution;
}