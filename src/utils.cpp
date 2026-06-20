#include <chrono>
#include <sys/resource.h>
#include <iostream>
#include <vector>
#include "tsplib_graph.h"

inline bool isLimitExceeded(int& iterCounter, 
                            std::chrono::steady_clock::time_point start, 
                            std::chrono::seconds timeLimit, 
                            long maxMemoryKB){ 

    iterCounter++;
        if ((iterCounter & 4095) == 0) {
            if (std::chrono::steady_clock::now() - start > timeLimit) {
                std::cout << "Przekroczono limit czasu" << std::endl;
                return true; // Zwracamy to co mamy, z flagą błędu
            }

            struct rusage usage;
            getrusage(RUSAGE_SELF, &usage);
            
            // usage.ru_maxrss w Linuxie zwraca wartość w Kilobajtach
            if (usage.ru_maxrss > maxMemoryKB) {
                std::cout << "Przekroczono limit pamieci! Zuzyto: " 
                          << (usage.ru_maxrss / 1024) << " MB" << std::endl;
                return true; // Powrót z błędem
            }
        }
    return false;
}

//Sprawdza, co liczbę iteracji, czas się nie skończył - inkrementuje zmienną iterCounter
inline bool isTimeLimitExceeded(int& iterCounter, 
                                std::chrono::steady_clock::time_point start, 
                                std::chrono::seconds timeLimit){
    iterCounter++;
    if ((iterCounter & 4095) == 0) {
        if (std::chrono::steady_clock::now() - start > timeLimit) {
            std::cout << "Przekroczono limit czasu" << std::endl;
            return true; // Zwracamy to co mamy, z flagą błędu
        }
    }
    return false;            

}

inline int primMST(const TSPLIBGraph& graph){
    int n = graph.size();
    std::vector<int> minKey(n, __INT_MAX__); // Najmniejsza krawędź do danego wierzchołka
    std::vector<bool> inMST(n, false);
    minKey[0] = 0;
    int totalWeight = 0;

    for (int count = 0; count < n; count++) {
        // Znajdź wierzchołek o minimalnej wadze krawędzi, który nie jest w MST
        int u = -1;
        for (int v = 0; v < n; v++)
            if (!inMST[v] && (u == -1 || minKey[v] < minKey[u]))
                u = v;

        inMST[u] = true;
        totalWeight += minKey[u];

        // Aktualizuj klucze sąsiadów
        for (int v = 0; v < n; v++) {
            int w = graph.weight(u, v);
            if (!inMST[v] && w < minKey[v])
                minKey[v] = w;
        }
    }
    return totalWeight;
}