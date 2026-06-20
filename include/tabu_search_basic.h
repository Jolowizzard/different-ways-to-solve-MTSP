#pragma once
#include <vector>
#include <chrono>
#include "algh_solution.h"
#include "tsplib_graph.h"
#include <cstring>
#include <iostream>
#include <algorithm>
#include "nearest_neighbour.h"

constexpr int MAX_SIZE = 500;
constexpr int VERTICES_SWAP_COUNT = 2;
// Struktura przechowująca historię wyników algorytmu tabu_search
struct tabu_search_results
{
    std::vector<int> solutions;
};

struct TabuEntry {
    int iterationAdded = 0; // Kiedy ostatnio wykonano ten ruch
    int currentTenure = 0;  // Jak długo ten ruch ma być zakazany
};

struct tabu_search_basic_state{
    int nodesCount; //Liczba wierzchołków w grafi - definiuje rozmiar tablic
    //wsm znamy największą badną instancję, więc możemy dać tutaj na sztywno rozmiar - chyba wtedy będzie szybciej działać
    int currentSolution [MAX_SIZE];
    int currentSolutionCost;
    int bestSolutionYet [MAX_SIZE];
    int bestSolutionYetCost;
    bool bestSolutionYetChanged; // flaga do optymalizacji liczby operacji
    int bestLegalSwap[VERTICES_SWAP_COUNT]; //Nie ma na liście tabu, albo spełnia warunek akceptacji
    int bestLegalSwapCost;
    bool legalSwapMade;
    //Gdy nie możliwym jest wybranie żadnego ruchu, wybrany zostanie najlepszy ruch zakazany
    int bestPunishedMove[VERTICES_SWAP_COUNT];
    int bestPunishedMoveCost;
    //Definicja pamięci krótkotrwałej listy tabu
    int tabuList [MAX_SIZE][MAX_SIZE];
    int tabuListSize;
    int cadentionTime;
    TabuEntry tabuListDynamic [MAX_SIZE][MAX_SIZE];
    //Dynamiczna zmiana wielkości listy tabu
    int iterationsUntilSizeChange;
    bool tabuListSizeChanged;
    //śledzenie przebiegu
    long int currentIteration;
    int iterationsWithNoImprovement;
    //LB
    int lowerBound; //Rozwiązanie dla rozwiązywanej instancji
    int acceptanceLevel; // Poziom akceptacji nadany przez użytkownika
    //czas jako warunek stopu
    std::chrono::steady_clock::time_point start;
    std::chrono::seconds timeLimit;
};

alghSolution tabuSearchBasicAlgorithm(const TSPLIBGraph& graph, int timeLimit, int cadentionTime, bool computeUB, int iterationsWithNoImprovement, float acceptanceLevel, int LB);

//Wygenerowanie losowego rozwiązania początkowego
int * generateSartingSolution(int solution [], int solutionSize);

//Pierwszy pomysł na liczenie jakości
int calculateVertexExchangeQuality(const TSPLIBGraph& graph, int vertexAPosition, int vertexBPosition, int currentPath [], int pathSize);

int calculateVertexExchangeQuality(const TSPLIBGraph& graph, int vertexAPosition, int vertexBPosition, std::vector<int> currentPath, int pathSize);

//Liczenie jakości ruchu 2opt
int calculate2OPTQuality(const TSPLIBGraph& graph, int vertexAPosition, int vertexBPosition, int currentPath [], int pathSize);

int calculate2OPTQuality(const TSPLIBGraph& graph, int vertexAPosition, int vertexBPosition, std::vector<int> &currentPath, int pathSize);


void exchangeVerticies(int vertexAPosition, int vertexBPosition, int currentPath []);

void R2OPT(int vertexAPosition, int vertexBPosition, int currentPath []);

void R2OPT(int vertexAPosition, int vertexBPosition, std::vector<int>& currentPath);

int partialCost(const TSPLIBGraph& graph,int ALeft, int AMiddle, int ARight, int BLeft, int BMiddle, int BRight);

int partialCost2OPT(const TSPLIBGraph& graph, int ALeft, int A, int B, int BRight);

void copySolutionValue(int src [], int dst [], int size);

void copySolutionValue(std::vector<int> src, int dst [], int size);

bool notTabu(int moveA, int moveB, int tabuList [][MAX_SIZE]);

bool notTabuTimers(int moveA, int moveB, int tabuList [][MAX_SIZE], int currentIteration);

bool notTabuCumulative(int moveA, int moveB, TabuEntry tabuList[][MAX_SIZE], int currentIteration);

void makeMoveTabuAdditive(int moveA, int moveB, int tabuList [][MAX_SIZE], int cadentionTime);

void makeMoveTabuConstant(int moveA, int moveB, int tabuList [][MAX_SIZE], int cadentionTime);

void makeMoveTabuAdditiveTimer(int moveA, int moveB, int tabuList [][MAX_SIZE],int currentIteration ,int cadentionTime);

void makeMoveTabuConstantTimer(int moveA, int moveB, int tabuList [][MAX_SIZE],int currentIteration ,int cadentionTime);

void makeMoveTabuAdditiveDynamic(int moveA, int moveB, TabuEntry tabuList[][MAX_SIZE], int currentIteration, int baseCadention);

void updateTabuList(int tabuList [][MAX_SIZE], int size);

void updateTabuListWithSize(int tabuList [][MAX_SIZE], int size, int *currentSize);

void changeTabuListSize(int tabuList [][MAX_SIZE], int size, int *currentSize, int newSize);

//funkcja stworzona strikte pod debbuging
void showTabuList(int tabuList [][MAX_SIZE], int size);

int calculatePunishedValue(int vertexA, int vertexB, int exchangeCost, int tabuList [][MAX_SIZE]);

int calculatePunishedValueCumulative(int vertexA, int vertexB, int exchangeCost, TabuEntry tabuList[][MAX_SIZE], int currentIteration);

void shrinkCumulativeTabuList(TabuEntry tabuList[][MAX_SIZE], int nodesCount, int maxAllowedCadention);