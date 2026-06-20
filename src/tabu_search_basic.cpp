#include "tabu_search_basic.h"
#include "utils.cpp"


//Podczas algorytmu należy pamiętać o tym, żeby nie ruszać pierwszego wierzchołka
alghSolution tabuSearchBasicAlgorithm(const TSPLIBGraph& graph, int timeLimitINT, int cadentionTime, bool computeUB, int iterationsWithNoImprovement, float acceptanceLevel, int LB){
    using clock = std::chrono::steady_clock;

    auto start = clock::now();
    auto timeLimit = std::chrono::seconds(timeLimitINT);

    alghSolution solutionFound;
    //Ustalenie parametrów pracy algorytmu
    tabu_search_basic_state state;
    tabu_search_results results;
    const int dynamicChange = 1000;
    const int saveCurrentSolution = 1000;
    int saveCurrentSolutionCount = 0;
    state.cadentionTime = cadentionTime;
    state.tabuListSize = cadentionTime;
    state.tabuListSizeChanged = false;
    state.timeLimit = timeLimit;
    state.nodesCount = graph.size();
    state.bestSolutionYetCost = __INT_MAX__;
    state.currentIteration = 0;
    state.iterationsWithNoImprovement = 0;
    state.iterationsUntilSizeChange = dynamicChange;
    //Wyznaczenie LB - Tutaj jeszcze algorytm do poprawy bo jest bardzo niskie, przez co warunek procentowy nigdy nie spełniony
    //state.lowerBound = primMST(graph);
    state.lowerBound = LB;

    //Przygotowanie listy tabu
    for(int i = 0; i < state.nodesCount; i++){
        for(int j = 0; j < state.nodesCount; j++){
            state.tabuList[i][j] = 0;
        }
    }
    //Stworzenie rozwiązania początkowego - można losowo, albo wybrać pierwsze z brzegu - jak będzie UB to wybieram UB
    for(int i = 0; i < state.nodesCount; i++){
        state.currentSolution[i] = i;
    }
    copySolutionValue(state.currentSolution, state.bestSolutionYet, state.nodesCount); // kopiowanie zawartości tablic
    //Wyliczenie kosztu ścieżki
    state.currentSolutionCost = calculatePathWeightStatic(graph, state.currentSolution);
    state.bestSolutionYetCost = state.currentSolutionCost;
    
    //Wyznaczenie UB za pomocą NN - w późniejszej wersji
    if(computeUB){
        alghSolution UB = nearestNeighbour(graph, 0, true, false, false);
        state.bestSolutionYetCost = UB.pathWeight;
        copySolutionValue(UB.path, state.bestSolutionYet, state.nodesCount);
        //Ustawienie rozwiązania UB jako obecne
        state.currentSolutionCost = state.bestSolutionYetCost;
        copySolutionValue(state.bestSolutionYet, state.currentSolution, state.nodesCount);
        std::cout << "Znaleziono UB i koszt wynosi : " << state.bestSolutionYetCost << std::endl;
    }

    int iter = 0;
    //Warunek końcowy pracy algorytmu - na razie tylko czas
    while(!isTimeLimitExceeded(iter, start, timeLimit)){
    //while(iter < 10){
        ////Generowanie sąsiedztwa - zamiana wierzchołków dla każdej kombinacji - (n-1)*(n-2) / 2 sąsiadów , pomijamy pierwszy wierzchołek
        state.bestLegalSwapCost = __INT_MAX__;
        state.bestPunishedMoveCost = __INT_MAX__;
        state.legalSwapMade = false;
        state.bestSolutionYetChanged = false;
        for(int i = 1; i < state.nodesCount - 1; i++){
            for(int j = i+1; j < state.nodesCount; j++){
                //Tę część można zoptymalizować - lepiej przypisywać i wydobywać wierzchołki
                int positionA = i;
                int positionB = j;
                int swapCost = calculateVertexExchangeQuality(graph, positionA, positionB, state.currentSolution, state.nodesCount);
                //std::cout << state.currentSolution[i] << " : " << state.currentSolution[j] << " : " << swapCost << std::endl;
                int newSolutionCost = state.currentSolutionCost + swapCost;
                //Wydobycie zamienianych wierzchołków
                int vertexA = state.currentSolution[positionA];
                int vertexB = state.currentSolution[positionB];

                //Sprawdzenie warunku ruchu legalnego, albo spełnia kryterium aspiracji, albo kryterium akceptacji
                bool isAspirational = newSolutionCost < state.bestSolutionYetCost;
                bool isNotTabu = notTabuTimers(vertexA, vertexB, state.tabuList, state.currentIteration);

                if((isAspirational || isNotTabu) && swapCost < state.bestLegalSwapCost){
                    state.bestLegalSwapCost = swapCost;
                    state.bestLegalSwap[0] = positionA;
                    state.bestLegalSwap[1] = positionB;
                    state.legalSwapMade = true;
                }
                else{
                    //W przypadku, gdy żaden legalny ruch nie został wykonany, przygotuj najlepszy ruch zakazany (z najmniejszą karą)
                    if(!state.legalSwapMade){
                        int punishedCost = calculatePunishedValue(positionA, positionB, swapCost, state.tabuList);
                        if(punishedCost < state.bestPunishedMoveCost){
                            state.bestPunishedMoveCost = swapCost;
                            state.bestPunishedMove[0] = positionA;
                            state.bestPunishedMove[1] = positionB;
                        }
                    }
                }
            }
        }
        //Aktualizacja nowego wybranego rozwiązania
        int tabuA, tabuB;
        if(!state.legalSwapMade){
            tabuA = state.currentSolution[state.bestPunishedMove[0]];
            tabuB = state.currentSolution[state.bestPunishedMove[1]];
            std::cout << "Best punished Swap - "<< state.currentSolution[state.bestPunishedMove[0]] << " : " << state.currentSolution[state.bestPunishedMove[1]] << " : " << state.bestPunishedMoveCost << std::endl;
            exchangeVerticies(state.bestPunishedMove[0], state.bestPunishedMove[1], state.currentSolution);
            state.currentSolutionCost = state.currentSolutionCost + state.bestPunishedMoveCost;
        }else{
            tabuA = state.currentSolution[state.bestLegalSwap[0]];
            tabuB = state.currentSolution[state.bestLegalSwap[1]];
            //std::cout << "Best Swap - "<< state.currentSolution[state.bestLegalSwap[0]] << " : " << state.currentSolution[state.bestLegalSwap[1]] << " : " << state.bestLegalSwapCost << std::endl;
            exchangeVerticies(state.bestLegalSwap[0], state.bestLegalSwap[1], state.currentSolution);
            state.currentSolutionCost = state.currentSolutionCost + state.bestLegalSwapCost;
        }

        //Aktualizacja listy tabu - zakazanie zamiany pary miast, a nie pozycji
        makeMoveTabuConstantTimer(tabuA, tabuB, state.tabuList, state.cadentionTime, state.currentIteration);


        if(state.currentSolutionCost < state.bestSolutionYetCost){
            state.bestSolutionYetCost = state.currentSolutionCost;
            copySolutionValue(state.currentSolution, state.bestSolutionYet, state.nodesCount);
            state.bestSolutionYetChanged = true;
            std::cout << "Znalazłem lepsze rozwiązanie : " << state.bestSolutionYetCost << std::endl;
        }

        //Warunek dynamicznej zmiany kadencji co jakąś liczbę iteracji
        // if(state.iterationsUntilSizeChange <= 0){
        //     state.iterationsUntilSizeChange = dynamicChange;
        //     if(!state.cadentionTimeChanged){
        //         state.cadentionTime = state.cadentionTime*2;
        //         state.tabuListSize = state.cadentionTime;
        //         state.cadentionTimeChanged = true;
        //         //std::cout << "Zmiana rozmiaru listy Tabu = " << state.tabuListSize << std::endl;
        //     }else{
        //         state.cadentionTime = cadentionTime;
        //         changeTabuListSize(state.tabuList, state.nodesCount, &state.tabuListSize, cadentionTime);
        //         //std::cout << "Zmiana rozmiaru listy Tabu = " << state.tabuListSize << std::endl;
        //         state.cadentionTimeChanged = false;
        //     }
        // }

        //updateTabuListWithSize(state.tabuList, state.nodesCount, &state.tabuListSize);
        
        //showTabuList(state.tabuList, state.nodesCount);
        //std::cout << "Best Swap - "<< state.bestLegalSwap[0] << " : " << state.bestLegalSwap[1] << " : " << state.bestLegalSwapCost << std::endl;
        //std::cout << "Current solution cost " << state.currentSolutionCost << std::endl;
        //Sprawdzenie warunku progresu
        if(state.currentSolutionCost == state.lowerBound){
            std::cout << "Osiągnięto optimum" << std::endl;
            break;
        }
        if(state.bestSolutionYetChanged){
            state.iterationsWithNoImprovement = 0;
            //sprawdzenie warunku osiągnięcia pewnego akceptowalnego poziomu błędu względnego
            float relativeError = ((float)(state.bestSolutionYetCost - state.lowerBound) / (float)state.lowerBound) * 100.0;
            if(relativeError <= acceptanceLevel){
                std::cout << "Osiągnięto wynik mieszczący się w zakresie błędu względnego : " << relativeError <<std::endl;
                break; 
            }
        }else{
            state.iterationsWithNoImprovement++;
            if(state.iterationsWithNoImprovement >= iterationsWithNoImprovement){
                std::cout << "Osiągnięto limit iteracji bez poprawy :" << state.iterationsWithNoImprovement << std::endl;
                break;
            }
        }

        if(saveCurrentSolutionCount >= saveCurrentSolution){
            saveCurrentSolutionCount = 0;
            results.solutions.push_back(state.currentSolutionCost);
        }
        
        results.solutions.push_back(state.currentSolutionCost);
        state.currentIteration++;
        saveCurrentSolutionCount++;
        //state.iterationsUntilSizeChange--;
    }
    std::cout << "Best solution found yet: " << state.bestSolutionYetCost << std::endl;
    std::cout << "Lower Bound = " << state.lowerBound << std::endl;
    std::cout << "Iterations Total = " << state.currentIteration << std::endl;

    //Trzeba pamiętać o zapisaniu wyniku
    solutionFound.path.assign(state.bestSolutionYet, state.bestSolutionYet + state.nodesCount);
    solutionFound.pathWeight = state.bestSolutionYetCost;
    solutionFound.solutionsInTime = results.solutions;
    solutionFound.interationsTotal = state.currentIteration;
    solutionFound.tabuListSize = state.tabuListSize;
    solutionFound.cadentionTime = state.cadentionTime;

    return solutionFound;
}

int * generateSartingSolution(int solution [], int solutionSize){}

//Funkcja do obliczania różnicy jakości kosztów między starą trasą a nową
//Z wraca różnicę między kosztem częściowym połączenia (A1-B2-A3 + B1-A2-B3) - (A1-A2-A3 + B1-B2-B3)
//Gdy nowa trasa sjest lepsza wartość jest ujemna, gdy jest gorsza dodatnia
int calculateVertexExchangeQuality(const TSPLIBGraph& graph, int vertexAPosition, int vertexBPosition, int currentPath [], int pathSize){
    //zainicjowanie zmiennych
    int ALeft = currentPath[(vertexAPosition - 1)]; // Zawsze nie ujemne
    int ARight = currentPath[(vertexAPosition + 1) % pathSize];
    int BLeft = currentPath[(vertexBPosition - 1)];
    int BRight = currentPath[(vertexBPosition + 1) % pathSize];
    //odkodowanie wierzchołków z pozycji
    int AVertex = currentPath[vertexAPosition];
    int BVertex = currentPath[vertexBPosition];

    int beforeCost;
    int afterCost;
        
    //gdy wierzchołki ze sobą sąsiadują
    if(ARight == BVertex){
        beforeCost = graph.weight(ALeft, AVertex) + graph.weight(AVertex, BVertex) + graph.weight(BVertex, BRight);
        afterCost = graph.weight(ALeft, BVertex) + graph.weight(BVertex, AVertex) + graph.weight(AVertex, BRight);
    }else{
        beforeCost = partialCost(graph, ALeft, AVertex, ARight, BLeft, BVertex, BRight);
        afterCost = partialCost(graph, ALeft, BVertex, ARight, BLeft, AVertex, BRight); // Zamiana A z B
    }
    //obliczenie kosztu przed i po zamianie
    return afterCost - beforeCost;
}

int calculateVertexExchangeQuality(const TSPLIBGraph& graph, int vertexAPosition, int vertexBPosition, std::vector<int> currentPath, int pathSize){
    //zainicjowanie zmiennych
    int ALeft = currentPath[(vertexAPosition - 1)]; // Zawsze nie ujemne
    int ARight = currentPath[(vertexAPosition + 1) % pathSize];
    int BLeft = currentPath[(vertexBPosition - 1)];
    int BRight = currentPath[(vertexBPosition + 1) % pathSize];
    //odkodowanie wierzchołków z pozycji
    int AVertex = currentPath[vertexAPosition];
    int BVertex = currentPath[vertexBPosition];

    int beforeCost;
    int afterCost;
        
    //gdy wierzchołki ze sobą sąsiadują
    if(ARight == BVertex){
        beforeCost = graph.weight(ALeft, AVertex) + graph.weight(AVertex, BVertex) + graph.weight(BVertex, BRight);
        afterCost = graph.weight(ALeft, BVertex) + graph.weight(BVertex, AVertex) + graph.weight(AVertex, BRight);
    }else{
        beforeCost = partialCost(graph, ALeft, AVertex, ARight, BLeft, BVertex, BRight);
        afterCost = partialCost(graph, ALeft, BVertex, ARight, BLeft, AVertex, BRight); // Zamiana A z B
    }
    //obliczenie kosztu przed i po zamianie
    return afterCost - beforeCost;
}

// int calculate2OPTQuality(const TSPLIBGraph& graph, int vertexAPosition, int vertexBPosition, int currentPath [], int pathSize){
//     int ALeft = currentPath[(vertexAPosition - 1)]; // Zawsze nie ujemne
//     int BRight = currentPath[(vertexBPosition + 1) % pathSize];
//     //odkodowanie wierzchołków z pozycji
//     int AVertex = currentPath[vertexAPosition];
//     int BVertex = currentPath[vertexBPosition];

//     int beforeCost;
//     int afterCost;

//     if(!graph.isATSP()){
//         beforeCost = partialCost2OPT(graph, ALeft, AVertex, BVertex, BRight);
//         afterCost = partialCost2OPT(graph, ALeft, BVertex, AVertex, BRight); // Zamiana A z B
//     }else{
//         beforeCost = 0;
//         afterCost = 0;
//         for(int i = 0; i < (vertexBPosition - vertexAPosition + 2); i++){
//             beforeCost += graph.weight(currentPath[vertexAPosition - 1 + i], currentPath[vertexAPosition + i]);
//             afterCost += graph.weight(currentPath[vertexBPosition + 1 - i], currentPath[vertexBPosition - i]);
//         }
//     }

//     return afterCost - beforeCost;
// }

int calculate2OPTQuality(const TSPLIBGraph& graph, int i, int j, int currentPath[], int n) {
    // Zakładamy, że i < j oraz i > 0 (nie ruszamy pierwszego wierzchołka)
    int A_prev = currentPath[i - 1];
    int A = currentPath[i];
    int B = currentPath[j];
    int B_next = currentPath[(j + 1) % n];

    if (!graph.isATSP()) {
        // Dla grafów symetrycznych (STSP) - Złożoność O(1)
        // Usuwamy krawędzie: (A_prev -> A) i (B -> B_next)
        // Dodajemy krawędzie: (A_prev -> B) i (A -> B_next)
        // Krawędzie wewnątrz odwróconego segmentu [i...j] nie zmieniają kosztu!
        int before = graph.weight(A_prev, A) + graph.weight(B, B_next);
        int after = graph.weight(A_prev, B) + graph.weight(A, B_next);
        return after - before;
    } else {
        // Dla grafów niesymetrycznych (ATSP) - Musimy uwzględnić zmianę kierunku wewnątrz
        int delta = 0;
        // 1. Zmiana na końcach (jak wyżej)
        delta -= graph.weight(A_prev, A);
        delta -= graph.weight(B, B_next);
        delta += graph.weight(A_prev, B);
        delta += graph.weight(A, B_next);

        // 2. Zmiana kierunku wszystkich krawędzi wewnątrz odwracanego segmentu
        // To jest kosztowne O(n), używaj tylko gdy graph.isATSP() == true
        for (int k = i; k < j; k++) {
            delta -= graph.weight(currentPath[k], currentPath[k + 1]);   // usuwamy oryginał
            delta += graph.weight(currentPath[k + 1], currentPath[k]);   // dodajemy odwrotność
        }
        return delta;
    }
}

int calculate2OPTQuality(const TSPLIBGraph& graph, int i, int j, std::vector<int>& currentPath, int n) {
    // Zakładamy, że i < j oraz i > 0 (nie ruszamy pierwszego wierzchołka)
    int A_prev = currentPath[i - 1];
    int A = currentPath[i];
    int B = currentPath[j];
    int B_next = currentPath[(j + 1) % n];

    if (!graph.isATSP()) {
        // Dla grafów symetrycznych (STSP) - Złożoność O(1)
        // Usuwamy krawędzie: (A_prev -> A) i (B -> B_next)
        // Dodajemy krawędzie: (A_prev -> B) i (A -> B_next)
        // Krawędzie wewnątrz odwróconego segmentu [i...j] nie zmieniają kosztu!
        int before = graph.weight(A_prev, A) + graph.weight(B, B_next);
        int after = graph.weight(A_prev, B) + graph.weight(A, B_next);
        return after - before;
    } else {
        // Dla grafów niesymetrycznych (ATSP) - Musimy uwzględnić zmianę kierunku wewnątrz
        int delta = 0;
        // 1. Zmiana na końcach (jak wyżej)
        delta -= graph.weight(A_prev, A);
        delta -= graph.weight(B, B_next);
        delta += graph.weight(A_prev, B);
        delta += graph.weight(A, B_next);

        // 2. Zmiana kierunku wszystkich krawędzi wewnątrz odwracanego segmentu
        // To jest kosztowne O(n), używaj tylko gdy graph.isATSP() == true
        for (int k = i; k < j; k++) {
            delta -= graph.weight(currentPath[k], currentPath[k + 1]);   // usuwamy oryginał
            delta += graph.weight(currentPath[k + 1], currentPath[k]);   // dodajemy odwrotność
        }
        return delta;
    }
}


void exchangeVerticies(int vertexAPosition, int vertexBPosition, int currentPath []){
    int tmp = currentPath[vertexAPosition];
    currentPath[vertexAPosition] = currentPath[vertexBPosition];
    currentPath[vertexBPosition] = tmp;
}

void R2OPT(int vertexAPosition, int vertexBPosition, int currentPath []){
    int dist = vertexBPosition - vertexAPosition;
    for(int i = 0; i < (dist + 1) / 2; i++){
        int swapA = vertexAPosition + i;
        int swapB = vertexBPosition - i;
        int tmp = currentPath[swapA];
        currentPath[swapA] = currentPath[swapB];
        currentPath[swapB] = tmp;
    }  
}

void R2OPT(int vertexAPosition, int vertexBPosition, std::vector<int>& currentPath){
    int dist = vertexBPosition - vertexAPosition;
    for(int i = 0; i < (dist + 1) / 2; i++){
        int swapA = vertexAPosition + i;
        int swapB = vertexBPosition - i;
        int tmp = currentPath[swapA];
        currentPath[swapA] = currentPath[swapB];
        currentPath[swapB] = tmp;
    }  
}

int partialCost(const TSPLIBGraph& graph,int ALeft, int AMiddle, int ARight, int BLeft, int BMiddle, int BRight){
    return graph.weight(ALeft, AMiddle) + graph.weight(AMiddle, ARight) + graph.weight(BLeft, BMiddle) + graph.weight(BMiddle, BRight);
}

int partialCost2OPT(const TSPLIBGraph& graph, int ALeft, int A, int B, int BRight){
    return graph.weight(ALeft, A) + graph.weight(B, BRight);
}

//Chyba najszybszy znany mi wariant
void copySolutionValue(int src [], int dst [], int size){
    std::memcpy(dst, src, size * sizeof(int));
}

void copySolutionValue(std::vector<int> src, int dst [], int size){
    for(int i = 0; i < size; i++){
        dst[i] = src[i];
    }
}


bool notTabu(int moveA, int moveB, int tabuList [][MAX_SIZE]){
    if(moveA < moveB){
        if(tabuList[moveA][moveB] > 0)
            return false;
    }else{
        if(tabuList[moveB][moveA] > 0)
            return false;
    }
    return true;
}

bool notTabuTimers(int moveA, int moveB, int tabuList [][MAX_SIZE], int currentIteration){
    if(moveA < moveB){
        if(tabuList[moveA][moveB] > currentIteration)
            return false;
    }else{
        if(tabuList[moveB][moveA] > currentIteration)
            return false;
    }
    return true;
}

bool notTabuCumulative(int moveA, int moveB, TabuEntry tabuList[][MAX_SIZE], int currentIteration) {
    int u = (moveA < moveB) ? moveA : moveB;
    int v = (moveA < moveB) ? moveB : moveA;

    if (tabuList[u][v].currentTenure == 0) return true;

    // Ruch wygasa, gdy obecna iteracja minie iterację dodania + indywidualną kadencję
    return (currentIteration - tabuList[u][v].iterationAdded) >= tabuList[u][v].currentTenure;
}

void makeMoveTabuAdditive(int moveA, int moveB, int tabuList [][MAX_SIZE], int cadentionTime){
    if(moveA < moveB)
        tabuList[moveA][moveB] += cadentionTime;
    else
        tabuList[moveB][moveA] += cadentionTime;
}

void makeMoveTabuConstant(int moveA, int moveB, int tabuList [][MAX_SIZE], int cadentionTime){
    if(moveA < moveB)
        tabuList[moveA][moveB] = cadentionTime;
    else
        tabuList[moveB][moveA] = cadentionTime;
}

//Funkcja do obsługi listy tabu jak timerów
void makeMoveTabuAdditiveTimer(int moveA, int moveB, int tabuList [][MAX_SIZE],int currentIteration ,int cadentionTime){
    if(moveA < moveB){
        if(tabuList[moveA][moveB] < currentIteration)
            tabuList[moveA][moveB] = currentIteration + cadentionTime;
        else
            tabuList[moveA][moveB] += cadentionTime;
    }
    else{
        if(tabuList[moveB][moveA] < currentIteration)
            tabuList[moveB][moveA] = currentIteration + cadentionTime;
        else
            tabuList[moveB][moveA] += cadentionTime;
    }
}

void makeMoveTabuConstantTimer(int moveA, int moveB, int tabuList [][MAX_SIZE],int currentIteration ,int cadentionTime){
    if(moveA < moveB)
        tabuList[moveA][moveB] = currentIteration + cadentionTime;
    else
        tabuList[moveB][moveA] = currentIteration + cadentionTime;
}

void makeMoveTabuAdditiveDynamic(int moveA, int moveB, TabuEntry tabuList[][MAX_SIZE], int currentIteration, int baseCadention){
    int u = (moveA < moveB) ? moveA : moveB;
    int v = (moveA < moveB) ? moveB : moveA;

    // Jeżeli od ostatniego zablokowania minęło mało czasu - maksymalnie 2 * tenure
    // to do obecnego tenure dodaj kadencję, czyli po zmianie ruch na liście tabu będzie mieć tenure*2 od momentu bierzącej iteracji.
    if (tabuList[u][v].currentTenure > 0 && 
       (currentIteration - tabuList[u][v].iterationAdded) <= (tabuList[u][v].currentTenure * 2)) {
        
        // Kumulacja: zwiększamy karę
        tabuList[u][v].currentTenure += baseCadention;
    } else {
        // Ruch zapomniany lub nowy - resetujemy do bazowej kadencji
        tabuList[u][v].currentTenure = baseCadention;
    }

    // Zawsze aktualizujemy iterację wykonania ruchu na obecną
    tabuList[u][v].iterationAdded = currentIteration;
}

//Sprawdzamy tylko połowę macierzy - dekrementacja wszystkich niezerowych wartości
void updateTabuList(int tabuList [][MAX_SIZE], int size){
    for(int i = 0; i < size; i++){
        for(int j = i + 1; j < size; j++){
            if(tabuList[i][j] > 0)
                tabuList[i][j]--;
        }
    }
}

//Sprawdzamy tylko połowę macierzy - dekrementacja wszystkich niezerowych wartości
//Rozszerzona wersja aktualizacji macierzy - usuwanie elementów dodanych najpóźniej, jeżeli rozmiar listy został przekroczony
void updateTabuListWithSize(int tabuList [][MAX_SIZE], int size, int* currentSize){
    int minValue = __INT_MAX__; 
    int numberOfElements = 0;
    int x;
    int y;
    for(int i = 0; i < size; i++){
        for(int j = i + 1; j < size; j++){
            int cadentionTimeLeft = tabuList[i][j];
            if(cadentionTimeLeft > 1){
                tabuList[i][j]--;
                numberOfElements++;
                if(cadentionTimeLeft < minValue){
                    x = i;
                    y = j;
                    minValue = cadentionTimeLeft;
                } 
            }
            if(cadentionTimeLeft == 1){
                tabuList[i][j]--;
                numberOfElements++;
                //(*currentSize)--;
            }
        }
    }
    if(numberOfElements > *currentSize){
        tabuList[x][y] = 0;
        //(*currentSize)--;
    }
}

//Funkcja kontrolująca rozmiar przy dynamicznej zmianie listy tabu - odblokowuje wszystkie ruchy o najmniejszesz kadencji
//Używana przy zmniejszaniu listyTabu
void changeTabuListSize(int tabuList [][MAX_SIZE], int size, int *currentSize, int newSize){
    //std::cout << "Zaczynam zamianę "<< std::endl;

    std::vector<int> cadentionTimes;
    for(int i = 0; i < size; i++){
        for(int j = i + 1; j < size; j++){
            int value = tabuList[i][j];

            if(value > 0){
                cadentionTimes.push_back(tabuList[i][j]);
            }
        }
    }

    //std::cout << "Jestem po znalezieniu wszyskich niezerowych kadencji"<< std::endl;

    //Sortowanie niemalejące
    std::sort(cadentionTimes.begin(), cadentionTimes.end());
    int splitValue = cadentionTimes.at(newSize - 1);

    //std::cout << "Posortowałem wszystkie kadencje i wybrałem wartość podziału : "<< splitValue <<std::endl;

    for(int i = 0; i < size; i++){
        for(int j = i + 1; j < size; j++){
            int value = tabuList[i][j];
            if(value <= splitValue){
                tabuList[i][j] = 0;
            }
        }
    }

    //std::cout << "Usunąłem wszystkie wartości mniejsze lub równe splitValue "<<std::endl;

    *currentSize = newSize;

    //std::cout << "Zmieniłem rozmiar tablicy :"<< *currentSize <<std::endl;
}


void showTabuList(int tabuList [][MAX_SIZE], int size){
    std::cout << "Tabu list state";
    for(int i = 0; i < size; i++){
        std::cout << " : " << tabuList[i];
    }
    std::cout << std::endl;
}

int calculatePunishedValue(int vertexA, int vertexB, int exchangeCost, int tabuList [][MAX_SIZE]){
    return (tabuList[vertexA][vertexB] + 1) * 1000  + exchangeCost;
}

int calculatePunishedValueCumulative(int vertexA, int vertexB, int exchangeCost, TabuEntry tabuList[][MAX_SIZE], int currentIteration) {
    int u = (vertexA < vertexB) ? vertexA : vertexB;
    int v = (vertexA < vertexB) ? vertexB : vertexA;
    
    int iterationAdded = tabuList[u][v].iterationAdded;
    int tenure = tabuList[u][v].currentTenure;

    // Ile iteracji kary jeszcze zostało?
    int iterationsLeft = tenure - (currentIteration - iterationAdded);
    if (iterationsLeft < 0) iterationsLeft = 0;

    return (iterationsLeft * 1000) + exchangeCost; 
}

void shrinkCumulativeTabuList(TabuEntry tabuList[][MAX_SIZE], int nodesCount, int maxAllowedCadention) {
    for (int i = 0; i < nodesCount; i++) {
        for (int j = i + 1; j < nodesCount; j++) {
            // Jeśli skumulowana kara jest większa niż nowa dopuszczalna, przytnij ją
            if (tabuList[i][j].currentTenure > maxAllowedCadention) {
                tabuList[i][j].currentTenure = maxAllowedCadention;
            }
        }
    }
}



