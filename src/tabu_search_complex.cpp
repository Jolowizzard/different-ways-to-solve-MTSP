#include "tabu_search_complex.h"
#include "utils.cpp"

alghSolution tabuSearchComplexAlgorithm(const TSPLIBGraph& graph, int timeLimitINT, int cadentionTime, bool computeUB, int iterationsWithNoImprovement, float acceptanceLevel, int tabuListSize, int iterationWithNoImprovementToChangeSize, int opt){
    using clock = std::chrono::steady_clock;

    auto start = clock::now();
    auto timeLimit = std::chrono::seconds(timeLimitINT);

    alghSolution solutionFound;
    //Ustalenie parametrów pracy algorytmu
    tabu_search_basic_state state;
    tabu_search_results results;
    const int saveCurrentSolution = 150;
    int saveCurrentSolutionCount = 0;
    state.cadentionTime = cadentionTime;
    state.tabuListSize = cadentionTime;
    state.tabuListSizeChanged= false;
    state.timeLimit = timeLimit;
    state.nodesCount = graph.size();
    state.bestSolutionYetCost = __INT_MAX__;
    state.currentIteration = 0;
    state.iterationsWithNoImprovement = 0;
    state.iterationsUntilSizeChange = iterationWithNoImprovementToChangeSize;

    //Wyznaczenie LB - Tutaj jeszcze algorytm do poprawy bo jest bardzo niskie, przez co warunek procentowy nigdy nie spełniony
    state.lowerBound = opt;

    //Przygotowanie listy tabu - w wariancie bez listy dynamicznej
    // for(int i = 0; i < state.nodesCount; i++){
    //     for(int j = 0; j < state.nodesCount; j++){
    //         state.tabuList[i][j] = 0;
    //     }
    // }

    //Przygotowanie dynamicznej listy tabu
    for(int i = 0; i < state.nodesCount; i++){
        for(int j = 0; j < state.nodesCount; j++){
            state.tabuListDynamic[i][j].iterationAdded = 0;
            state.tabuListDynamic[i][j].currentTenure = 0;
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
        std::cout << "Wyliczyłem UB : " << state.bestSolutionYetCost << std::endl;
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
                //int swapCost = calculateVertexExchangeQuality(graph, positionA, positionB, state.currentSolution, state.nodesCount);
                //Dla 2-OPT
                int swapCost = calculate2OPTQuality(graph, positionA, positionB, state.currentSolution, state.nodesCount);
                //std::cout << state.currentSolution[i] << " : " << state.currentSolution[j] << " : " << swapCost << std::endl;
                int newSolutionCost = state.currentSolutionCost + swapCost;

                //Wydobycie zamienianych wierzchołków
                int vertexA = state.currentSolution[positionA];
                int vertexB = state.currentSolution[positionB];

                //Sprawdzenie warunku ruchu legalnego, spełniającego kryterium aspiracji albo nie będącego na liście tabu - ruch musi być lepszy niż wcześniej znaleziony
                bool isAspirational = newSolutionCost < state.bestSolutionYetCost;
                bool isNotTabu = notTabuCumulative(vertexA, vertexB, state.tabuListDynamic, state.currentIteration);
                if((isAspirational || isNotTabu) && swapCost < state.bestLegalSwapCost){
                    state.bestLegalSwapCost = swapCost;
                    state.bestLegalSwap[0] = positionA;
                    state.bestLegalSwap[1] = positionB;
                    state.legalSwapMade = true;
                }else{
                    //W przypadku, gdy żaden legalny ruch nie został wykonany, przygotuj najlepszy ruch zakazany (z najmniejszą karą)
                    if(!state.legalSwapMade){
                        int punishedCost = calculatePunishedValueCumulative(positionA, positionB, swapCost, state.tabuListDynamic, state.currentIteration);
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
            R2OPT(state.bestPunishedMove[0], state.bestPunishedMove[1], state.currentSolution);
            state.currentSolutionCost += state.bestPunishedMoveCost;
        } else {
            tabuA = state.currentSolution[state.bestLegalSwap[0]];
            tabuB = state.currentSolution[state.bestLegalSwap[1]];
            R2OPT(state.bestLegalSwap[0], state.bestLegalSwap[1], state.currentSolution);
            state.currentSolutionCost += state.bestLegalSwapCost;
            //std::cout << "Best Swap - "<< state.bestLegalSwap[0] << " : " << state.bestLegalSwap[1] << " : " << state.currentSolutionCost << std::endl;
        }
        //Aktualizacja listy tabu - zakazanie zamiany pary miast, a nie pozycji
        makeMoveTabuAdditiveDynamic(tabuA, tabuB, state.tabuListDynamic, state.currentIteration, state.cadentionTime);

        if(state.currentSolutionCost < state.bestSolutionYetCost){
            state.bestSolutionYetCost = state.currentSolutionCost;
            copySolutionValue(state.currentSolution, state.bestSolutionYet, state.nodesCount);
            state.bestSolutionYetChanged = true;
            std::cout << "Znalazłem lepsze rozwiązanie : " << state.bestSolutionYetCost << std::endl;
        }
        
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
            //Ustawienie dynamicznego rozmiaru na mniejszy w celu leszej eksploracji
            if(state.tabuListSizeChanged){
                state.cadentionTime = cadentionTime;
                state.tabuListSize = state.cadentionTime;
                shrinkCumulativeTabuList(state.tabuListDynamic, state.nodesCount, cadentionTime);
                //std::cout << "Zmiana rozmiaru listy Tabu = " << state.tabuListSize << std::endl;
                state.tabuListSizeChanged = false;
            }
        }else{
            state.iterationsWithNoImprovement++;
            state.iterationsUntilSizeChange--;
            //Warunek dynamicznej zmiany kadencji co jakąś liczbę iteracji
            if(state.iterationsUntilSizeChange <= 0){
                state.iterationsUntilSizeChange = iterationWithNoImprovementToChangeSize;
                if(!state.tabuListSizeChanged){
                    state.cadentionTime = state.cadentionTime*2;
                    state.tabuListSize = state.cadentionTime;
                    state.tabuListSizeChanged = true;
                    //std::cout << "Zmiana rozmiaru listy Tabu = " << state.tabuListSize << std::endl;
                }else{
                    state.cadentionTime = cadentionTime;
                    state.tabuListSize = state.cadentionTime;
                    shrinkCumulativeTabuList(state.tabuListDynamic, state.nodesCount, cadentionTime);
                    //std::cout << "Zmiana rozmiaru listy Tabu = " << state.tabuListSize << std::endl;
                    state.tabuListSizeChanged = false;
                }
            }

            if(state.iterationsWithNoImprovement >= iterationsWithNoImprovement){
                std::cout << "Osiągnięto limit iteracji bez poprawy :" << state.iterationsWithNoImprovement << std::endl;
                break;
            }
        }

        if(saveCurrentSolutionCount < saveCurrentSolution){
            results.solutions.push_back(state.currentSolutionCost);
        }
        saveCurrentSolutionCount++;
        state.currentIteration++;
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