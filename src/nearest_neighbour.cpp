#include "nearest_neighbour.h"
#include <limits>
#include <algorithm>
#include "random.h"

// Dodano parametr branchOnTies sterujący zachowaniem w przypadku remisu
alghSolution solveNN(const TSPLIBGraph& graph, int current, std::vector<bool>& visited, int remaining, bool branchOnTies) {
    if (remaining == 0) {
        return {{}, 0.0}; 
    }

    double minDistance = std::numeric_limits<double>::max();
    std::vector<int> candidates;

    for (int i = 0; i < graph.size(); ++i) {
        if (!visited[i]) {
            double dist = graph.weight(current, i);
            
            // Zawsze nadpisujemy kandydata, jeśli znajdziemy ostro mniejszy dystans
            if (dist < minDistance) {
                minDistance = dist;
                candidates.clear();
                candidates.push_back(i);
            } 
            // W przypadku remisu dodajemy kandydata TYLKO, jeśli branchOnTies == true
            else if (dist == minDistance && branchOnTies) {
                candidates.push_back(i);
            }
        }
    }

    alghSolution bestSubResult;
    bestSubResult.pathWeight = std::numeric_limits<double>::max();

    for (int nextNode : candidates) {
        visited[nextNode] = true;
        // Przekazujemy flagę dalej przy wywołaniu rekurencyjnym
        alghSolution res = solveNN(graph, nextNode, visited, remaining - 1, branchOnTies);
        visited[nextNode] = false;

    if (res.pathWeight + minDistance < bestSubResult.pathWeight) {
        bestSubResult.pathWeight = res.pathWeight + minDistance;
        bestSubResult.path = res.path;
        // Wstawiamy nextNode na początek otrzymanej pod-ścieżki
        bestSubResult.path.insert(bestSubResult.path.begin(), nextNode);
    }
    }

    return bestSubResult;
}

alghSolution nearestNeighbour(const TSPLIBGraph& graph, int startVertex, bool repetitive, bool randomStart, bool ties) {
    int n = graph.size();
    if (n == 0) return {}; // Zabezpieczenie przed pustym grafem

    alghSolution bestOverall;
    bestOverall.pathWeight = std::numeric_limits<double>::max();

    int firstNode, lastNode;

    // 1. Ustalanie zakresu pętli na podstawie flag
    if (randomStart) {
        // Losuj wierzchołek z przedziału [0, n - 1]
        std::uniform_int_distribution<int> dist(0, n - 1);
        int randVertex = dist(gen);
        
        firstNode = randVertex;
        lastNode = randVertex;
    } else if (repetitive) {
        // Przejdź przez wszystkie wierzchołki
        firstNode = 0;
        lastNode = n - 1;
    } else {
        // Użyj tylko konkretnego wierzchołka
        firstNode = startVertex;
        lastNode = startVertex;
    }

    // 2. Wykonanie algorytmu NN dla wyznaczonych wierzchołków
    for (int i = firstNode; i <= lastNode; ++i) {
        std::vector<bool> visited(n, false);
        visited[i] = true;
        
        // Wywołujemy solveNN przekazując 'repetitive' jako sterownik remisów
        alghSolution currentSol = solveNN(graph, i, visited, n - 1, ties);
        
        // Dodaj wierzchołek startowy na początek i domknij cykl
        currentSol.path.insert(currentSol.path.begin(), i);
        double returnEdge = graph.weight(currentSol.path.back(), i);
        currentSol.pathWeight += returnEdge;
        currentSol.path.push_back(i);

        if (currentSol.pathWeight < bestOverall.pathWeight) {
            bestOverall = currentSol;
        }
    }

    return bestOverall;
}