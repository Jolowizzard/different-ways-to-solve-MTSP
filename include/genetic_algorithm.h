#pragma once
#include <vector>
#include <chrono>
#include "algh_solution.h"
#include "tsplib_graph.h"
#include <cstring>
#include <iostream>
#include <algorithm>
#include "nearest_neighbour.h"
#include "random.h" // Dla funkcji losowych

// Struktura reprezentująca osobnika w populacji
struct specimen {
    std::vector<int> path; // Ścieżka reprezentująca rozwiązanie
    int fitness; // Wartość dopasowania (koszt rozwiązania)

    bool operator<(const specimen& other) const {
        return fitness < other.fitness; // Porównywanie osobników na podstawie wartości dopasowania
    }
};

// Struktura pomocnicza dla przeprowadzania turnieju - pozwala na zamianę osobinika na ostatnie miejsce w populacji, co zapobiegnie ponownemu wylosowaniu
struct tournamentParticipant {
    struct specimen specimen;
    int placeInPopulation; //Zmienna do przechowywania miejsca w populacji
};

struct genetic_algorithm_state {
    int nodesCount; //Liczba wierzchołków w grafie - definiuje rozmiar tablic
    std::vector<specimen> population; // Populacja rozwiązań
    std::vector<specimen> parents; // Rodzice wybrani do krzyżowania
    std::vector<specimen> offspring; // Potomkowie wygenerowani przez krzyżowanie
    std::vector<specimen> elites; // Elitarne rozwiązania zachowane do następnej generacji
    std::vector<specimen> newPopulation; // Nowa populacja po selekcji
    std::vector<specimen> tempPopulation; // Tymczasowa populacja do przechowywania rozwiązań podczas selekcji - suma rozwiązań rodziców i potomków
    specimen bestSolution; // Najlepsze rozwiązanie znalezione do tej pory
    int bestSolutionCost; // Koszt najlepszego rozwiązania
    double averageFitness; // Średnia wartość dopasowania w populacji
    int iterationsWithoutImprovement; // Liczba iteracji bez poprawy najlepszego rozwiązania
    int acceptableSolutionCost; // Koszt akceptowalnego rozwiązania, który może być użyty jako warunek zakończenia
    std::chrono::steady_clock::time_point start; // Czas rozpoczęcia algorytmu
    std::chrono::seconds timeLimit; // Limit czasu dla algorytmu
    int totalIterations; // Całkowita liczba iteracji wykonanych przez algorytm
};

// Funkcja realizująca algorytm genetyczny dla problemu TSP. Przyjmuje:
// graph - graf TSP, 
// timeLimit - limit czasu, 
// populationSize - rozmiar populacji, 
// parentSize - liczba rodziców, 
// offspringSize - liczba potomków, 
// eliteSize - liczba elitarnych rozwiązań, 
// mutationRate - współczynnik mutacji, 
// crossoverRate - współczynnik krzyżowania, 
// tournamentSize - rozmiar turnieju, 
// computeUB - flaga do obliczania górnej granicy, 
// iterationsWithNoImprovement - liczba iteracji bez poprawy, 
// acceptanceLevel - poziom akceptacji, 
// LB - dolna granica. Zwraca najlepsze rozwiązanie znalezione przez algorytm.
alghSolution geneticAlgorithm(const TSPLIBGraph& graph, 
                              int timeLimit, 
                              int populationSize, 
                              int parentSize, 
                              int offspringSize, 
                              int eliteSize, 
                              float mutationRate, 
                              float crossoverRate, 
                              int tournamentSize, 
                              bool computeUB, 
                              int iterationsWithNoImprovement, 
                              float acceptanceLevel, 
                              int LB);

// Funkcja do inicjalizacji populacji początkowej dla algorytmu genetycznego. Przyjmuje:
// graf TSP i rozmiar populacji, a zwraca wektor rozwiązań reprezentujących początkową populację.
// Funkcja dodatokowo oblicza koszt każdego z wygenerowanych rozwiązań
void initializePopulation(const TSPLIBGraph& graph, genetic_algorithm_state& state, int populationSize, int withUB);

// Funkcja wyliczająca wartość funkcji fitness dla każdego osobnika w pewnej grupie
void evaluateFitness(const TSPLIBGraph& graph, std::vector<specimen>& group);

// Przeładowana funkcja wyliczająca wartość funkcji fitness dla pojedynczego osobnika
void evaluteFitness(const TSPLIBGraph& graph, specimen& individual);

// Funkcja dodająca elity do nowej populacji. Przyjmuje:
// state - stan algorytmu genetycznego, zawierający populację,
// eliteSize - liczbę elitarnych rozwiązań do dodania. Funkcja kopiuje najlepsze rozwiązania z populacji do nowej populacji
void addElites(genetic_algorithm_state& state, int eliteSize);

// Funkcja selekcji rodziców do krzyżowania metodą turniejową. Przyjmuje:
// state - stan algorytmu genetycznego, zawierający populację,
// parentSize - liczba rodziców do wybrania, 
// tournamentSize - rozmiar turnieju.
void selectParentsTournament(genetic_algorithm_state& state, int parentSize, int tournamentSize);

// Funkcja krzyżowania dwóch rodziców metodą PMX (Partially Mapped Crossover). Przyjmuje:
// parent1 - pierwszy rodzic,
// parent2 - drugi rodzic,
// Zwraca parę potomków wygenerowanych przez krzyżowanie z nową drogą. Koszt fitness pozostaje nie zmieniony, dlatego należy go potem wyliczyć na nowo.
std::pair<specimen, specimen> crossoverPMX(const specimen& parent1, const specimen& parent2);

// Funkcja pomocnicza do krzyżowania metodą PMX, która zachowuje materiał genetyczny rodziców. Przyjmuje:
// parent1 - pierwszy rodzic,
// parent2 - drugi rodzic,
// offspring1 - pierwszy potomek, który zostanie zmodyfikowany,
// offspring2 - drugi potomek, który zostanie zmodyfikowany.
void crossoverPMXSaveParentMaterial(const specimen& parent1, const specimen& parent2, specimen& offspring1, specimen& offspring2, int startPos, int endPos, int cuttingPoint1, int cuttingPoint2);

// Funkcja tworząca nową generację selekcją (mi + lambda)
void selectNextGeneration(genetic_algorithm_state& state);

void mutateOffspring(const TSPLIBGraph& graph ,genetic_algorithm_state& state, float mutationRate);

//Funkcja sprawdzająca, czy zachodzi mutacja, jeżeli tak, to mutuje osobnika przy pomocy operatora swap.
void mutate(const TSPLIBGraph& graph ,specimen& individual);

// Funkcja wkorzystująca krzyżowanie do wygenerowania nowych potomków. Przyjmuje:
// state - stan algorytmu genetycznego, zawierający populację rodziców, potomków i elit, 
// crossoverRate - współczynnik krzyżowania
// Wylicza koszt fitness dla nowo postawłej pary potomstwa
void generateOffspring(const TSPLIBGraph& graph, genetic_algorithm_state& state, float crossoverRate);

// Funkcja pomocnicza generująca losową ścieżkę. Nie ruszając pierwszego wierzchołka
std::vector <int> generateRandomPath(int nodesCount);

// Funkcja sprawdzająca, czy został przekroczony limit czasu albo limit iteracji bez poprawy albo obecne rozwiązanie jest akceptowalne. Przyjmuje:
bool terminationConditionNotMet(genetic_algorithm_state& state, int iterationsWithNoImprovement, int& iter);

void showSpecimens(const std::vector<specimen>& specimens);