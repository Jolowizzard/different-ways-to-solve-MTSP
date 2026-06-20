#include "genetic_algorithm.h"
#include "utils.cpp"
#include "tabu_search_basic.h"

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
                              int LB){
// Inicjalizacja stanu algorytmu genetycznego
    genetic_algorithm_state state;
    state.nodesCount = graph.size();
    state.population.reserve(populationSize);
    state.parents.reserve(parentSize);
    state.offspring.reserve(offspringSize);
    state.elites.reserve(eliteSize);
    state.newPopulation.reserve(populationSize);
    state.tempPopulation.reserve(populationSize + offspringSize);
    state.bestSolutionCost = std::numeric_limits<int>::max();
    state.start = std::chrono::steady_clock::now();
    state.timeLimit = std::chrono::seconds(timeLimit);
    state.acceptableSolutionCost = static_cast<int>(LB + (LB * acceptanceLevel));
    state.iterationsWithoutImprovement = 0;
    state.totalIterations = 0;

    int testMaxIterations = 0; // Zmienna ustawiona do testowania algorytmu
    
    // Inicjalizacja populacji początkowej (np. losowe rozwiązania lub heurystyka NN)
    initializePopulation(graph, state, populationSize, computeUB);
    //std::cout << "Initial best solution cost: " << state.bestSolutionCost << std::endl;
    //showSpecimens(state.population);
    int iter = 0;
    // Główna pętla algorytmu genetycznego
    while (terminationConditionNotMet(state, iterationsWithNoImprovement, iter)) {
        // Zapamiętanie rozwiązań elitarnych
        //addElites(state, eliteSize);

        // Selekcja rodziców turniejowa
        selectParentsTournament(state, parentSize, tournamentSize);
        //std::cout << "Selected parents for iteration " << state.totalIterations << std::endl;
        //showSpecimens(state.parents);
        // Krzyżowanie rodziców w celu wygenerowania potomków
        generateOffspring(graph, state, crossoverRate);
        //std::cout << "Generated offspring for iteration " << state.totalIterations << std::endl;
        // Mutacja potomków
        mutateOffspring(graph, state, mutationRate);
        //std::cout << "Mutated offspring for iteration " << state.totalIterations << std::endl;
        // Selekcja nowej populacji
        selectNextGeneration(state);
        //std::cout << "Selected next generation for iteration " << state.totalIterations << std::endl;
        // Aktualizacja najlepszego rozwiązania i warunku zakończenia na podstawie iteracji bez poprawy
        if (state.population[0].fitness < state.bestSolutionCost) {
            state.bestSolutionCost = state.population[0].fitness;
            state.bestSolution = state.population[0];
            state.iterationsWithoutImprovement = 0;
            std::cout << "New best solution found: " << state.bestSolutionCost << std::endl;
        } else {
            state.iterationsWithoutImprovement++;
        }

        //std::cout << "Current best solution: " << state.population[0].fitness << std::endl;
        state.totalIterations++;
    }

    // Zwracanie najlepszego rozwiązania znalezionego przez algorytm
    alghSolution result;
    result.path = state.bestSolution.path;
    result.pathWeight = state.bestSolution.fitness;
    result.interationsTotal = state.totalIterations;
    showSpecimens({state.bestSolution});
    return result;

}

void initializePopulation(const TSPLIBGraph& graph, genetic_algorithm_state& state, int populationSize, int withUB){
    int specimensCount = 0;
    if(withUB){
        alghSolution ubSolution = nearestNeighbour(graph, 0, true, false, false);
        specimen ubSpecimen;
        ubSpecimen.path = ubSolution.path;
        ubSpecimen.fitness = static_cast<int>(ubSolution.pathWeight);
        state.population.push_back(ubSpecimen);
        //std::cout << "UB specimen : " << ubSpecimen.fitness << std::endl;
        state.bestSolutionCost = ubSpecimen.fitness;
        state.bestSolution = ubSpecimen;
        specimensCount++;
    }
    //std::cout << "Start of the initialization of population" << std::endl;
    for (int i = specimensCount; i < populationSize; ++i) {
        specimen specimen;
        specimen.path = generateRandomPath(graph.size()); // Funkcja generująca losową ścieżkę
        specimen.fitness = calculatePathWeight(graph, specimen.path); // Obliczanie kosztu ścieżki
        state.population.push_back(specimen);

        if(specimen.fitness < state.bestSolutionCost){
            state.bestSolutionCost = specimen.fitness;
            state.bestSolution = specimen;
        }
    }
    //std::cout << "End of initialization" << std::endl;
}

void evaluateFitness(const TSPLIBGraph& graph, std::vector<specimen>& group){
    for (auto& individual : group) {
        individual.fitness = calculatePathWeight(graph, individual.path);
    }
}

void evaluteFitness(const TSPLIBGraph& graph, specimen& individual){
    individual.fitness = calculatePathWeight(graph, individual.path);
}

std::vector <int> generateRandomPath(int nodesCount){
    std::vector<int> orderedPath(nodesCount);
    std::iota(orderedPath.begin(), orderedPath.end(), 0); // Wypełniamy wektor wartościami od 0 do nodesCount-1
    std::vector<int> randomPath;
    randomPath.reserve(nodesCount);
    randomPath.push_back(orderedPath[0]);
    orderedPath[0] = orderedPath[nodesCount - 1];

    for (int i = 1; i < nodesCount; ++i) { // Zaczynamy od 1, aby nie ruszać pierwszego wierzchołka
        int randomVertex = std::uniform_int_distribution<int>(1, nodesCount- i)(gen);
        randomPath.push_back(orderedPath[randomVertex]);
        orderedPath[randomVertex] = orderedPath[nodesCount- i]; 
    }

    return randomPath;
}

void addElites(genetic_algorithm_state& state, int eliteSize){
    if (eliteSize <= 0 || eliteSize > state.population.size()) {
        return; // Nie dodajemy elit, jeśli rozmiar jest nieprawidłowy
    }
    std::sort(state.population.begin(), state.population.end());
    for (int i = 0; i < eliteSize; ++i) {
        state.elites.push_back(state.population[i]);
    }
}

void selectParentsTournament(genetic_algorithm_state& state,int parentSize, int tournamentSize){
    state.parents.clear();
    std::vector<tournamentParticipant> tournament;
    tournament.reserve(tournamentSize);
    for (int i = 0; i < parentSize; ++i) {
        // Losowanie uczestników turnieju
        //std::cout << "Creating tournament..." << std::endl;
        for(int j = 0; j < tournamentSize; j++){
            int randomSpecimen = std::uniform_int_distribution<int>(0, state.population.size() - 1 - j - i)(gen);
            tournamentParticipant TP {state.population[randomSpecimen],state.population.size() - 1 - j - i};
            //Zapisanie osobnika do turnieju oraz informacji o jego bierzącej pozycji w populacji - po dodaniu trafia na koniec listy
            tournament.push_back(TP);
            //Zachowanie wylosowanego osobnika i ustawienie go na koniec, żeby nie został wylosowany ponownie
            specimen tmp = state.population[randomSpecimen];
            state.population[randomSpecimen] = state.population[state.population.size() - 1 - j - i];
            state.population[state.population.size() - 1 - j - i] = tmp;
        }
        // showSpecimens(tournament);
        // Wybór najlepszego osobnika z turnieju
        int placeInPopulation;
        specimen tournamentWinner;
        tournamentWinner.fitness = std::numeric_limits<int>::max();
        for(int j = 0; j < tournamentSize; j++){
            auto currentSpecimen = tournament[i];
            if(currentSpecimen.specimen.fitness < tournamentWinner.fitness){
                tournamentWinner = currentSpecimen.specimen;
                placeInPopulation = currentSpecimen.placeInPopulation;
            }
        }
        state.parents.push_back(tournamentWinner);
        //Wyrzucenie zwycięzcy z przyszłej puli losowania
        specimen tmp = state.population[placeInPopulation];
        state.population[placeInPopulation] = state.population[state.population.size() - 1 - i];
        state.population[state.population.size() - 1 - i] = tmp;
        //std::cout << "Population after tournament" << std::endl;
        //showSpecimens(state.population);
    }
}

void generateOffspring(const TSPLIBGraph& graph, genetic_algorithm_state& state, float crossoverRate){
    state.offspring.clear();
    while(state.offspring.size() < state.offspring.capacity()){
        int parent1Index = std::uniform_int_distribution<int>(0, state.parents.size() - 1)(gen);
        int parent2Index = std::uniform_int_distribution<int>(0, state.parents.size() - 1)(gen);

        while(parent2Index == parent1Index){
            parent2Index = std::uniform_int_distribution<int>(0, state.parents.size() - 1)(gen);
        }

        if (std::uniform_real_distribution<float>(0.0f, 1.0f)(gen) < crossoverRate) {
            auto offspringPair = crossoverPMX(state.parents[parent1Index], state.parents[parent2Index]);
            offspringPair.first.fitness = calculatePathWeight(graph, offspringPair.first.path);
            state.offspring.push_back(offspringPair.first);

            if(state.offspring.size() < state.offspring.capacity()){
                offspringPair.second.fitness = calculatePathWeight(graph, offspringPair.second.path);
                state.offspring.push_back(offspringPair.second);
            }
        }
    }
}

void mutateOffspring(const TSPLIBGraph& graph, genetic_algorithm_state& state, float mutationRate){
    for (auto& individual : state.offspring) {
        if (std::uniform_real_distribution<float>(0.0f, 1.0f)(gen) < mutationRate) {
            mutate(graph, individual);
            //std::cout << "specimen mutated" << std::endl;
        }
    }
}

void selectNextGeneration(genetic_algorithm_state& state){
    state.tempPopulation.clear();
    // Dodanie potomków do tymczasowej populacji
    for (const auto& offspring : state.offspring) {
        state.tempPopulation.push_back(offspring);
    }
    // Dodanie wcześniejszej populacji do tymczasowej populacji
    for (const auto& currentSpecimen : state.population) {
        state.tempPopulation.push_back(currentSpecimen);
    }
    // Sortowanie tymczasowej populacji i wybór najlepszych osobników do nowej populacji
    std::sort(state.tempPopulation.begin(), state.tempPopulation.end());
    state.newPopulation.clear();
    for (int i = 0; i < state.population.size(); ++i) {
        state.newPopulation.push_back(state.tempPopulation[i]);
    }
    // Zamiana nowej populacji z aktualną populacją
    state.population = state.newPopulation;
}


void mutate(const TSPLIBGraph& graph, specimen& individual){
    int pos1 = std::uniform_int_distribution<int>(1, individual.path.size() - 1)(gen); // Zaczynamy od 1, aby nie ruszać pierwszego wierzchołka
    int pos2 = std::uniform_int_distribution<int>(1, individual.path.size() - 1)(gen);

    while(pos2 == pos1){
        pos2 = std::uniform_int_distribution<int>(1, individual.path.size() - 1)(gen);
    }
    
    if (pos1 > pos2) {
        std::swap(pos1, pos2);
    }

    int swapCost = calculate2OPTQuality(graph, pos1, pos2, individual.path, individual.path.size());
    
    R2OPT(pos1, pos2, individual.path);
    individual.fitness = individual.fitness + swapCost;
}

std::pair<specimen, specimen> crossoverPMX(const specimen& parent1, const specimen& parent2){
    // Ważne jest to, żeby uwzględnić fakt, że pierwszy wierzchołek jest stały, więc punkty cięcia muszą być wybierane z zakresu od 2 do n-1
    int cuttingPoint1 = std::uniform_int_distribution<int>(2, parent1.path.size() - 2)(gen);
    int cuttingPoint2 = std::uniform_int_distribution<int>(cuttingPoint1 + 1, parent1.path.size() - 1)(gen);
    specimen offspring1, offspring2;
    offspring1.path.resize(parent1.path.size());
    offspring2.path.resize(parent1.path.size());

    //showSpecimens({parent1, parent2});
    //showSpecimens({offspring1, offspring2});
    // Kopiowanie segmentu z rodziców do potomków
    for (int i = cuttingPoint1; i <= cuttingPoint2; i++){
        offspring1.path[i] = parent2.path[i];
        offspring2.path[i] = parent1.path[i];
    }
    //std::cout << "After copying segments for offspring:" << std::endl;
    //showSpecimens({offspring1, offspring2});

    // Mapowanie genów z rodzica przed pierwszym punktem cięcia - wciąż pamiętając o tym, że pierwszy wierzchołek jest stały
    crossoverPMXSaveParentMaterial(parent1, parent2, offspring1, offspring2, 1, cuttingPoint1, cuttingPoint1, cuttingPoint2);
    //std::cout << "After mapping genes before the first cutting point for offspring:" << std::endl;
    //showSpecimens({offspring1, offspring2});
    // Mapowanie genów z rodzica po drugim punkcie cięcia - wciąż pamiętając o tym, że pierwszy wierzchołek jest stały
    crossoverPMXSaveParentMaterial(parent1, parent2, offspring1, offspring2, cuttingPoint2 + 1, parent1.path.size(), cuttingPoint1, cuttingPoint2);
    //std::cout << "After mapping genes after the second cutting point for offspring:" << std::endl;
    //showSpecimens({offspring1, offspring2});

    // Potomstwo jest zwracane bez wyliczonego kosztu fitness
    return {offspring1, offspring2};
}


void crossoverPMXSaveParentMaterial(const specimen& parent1, const specimen& parent2, specimen& offspring1, specimen& offspring2, int startPos, int endPos, int cuttingPoint1, int cuttingPoint2) {
    for (int i = startPos ; i < endPos; i++) {
        // --- Naprawa dla offspring1 ---
        int candidate1 = parent1.path[i];
        bool mapped1 = true;
        
        while (mapped1) {
            mapped1 = false;
            // Sprawdzamy, czy obecny kandydat jest już w skopiowanym segmencie offspring1
            for (int j = cuttingPoint1; j <= cuttingPoint2; j++) {
                if (offspring1.path[j] == candidate1) {
                    // Jeśli jest, bierzemy odpowiadający mu gen z parent1
                    candidate1 = parent1.path[j];
                    mapped1 = true; // Flaga w górę - musimy sprawdzić nowego kandydata
                    break;
                }
            }
        }
        offspring1.path[i] = candidate1;

        // --- Naprawa dla offspring2 ---
        int candidate2 = parent2.path[i];
        bool mapped2 = true;
        
        while (mapped2) {
            mapped2 = false;
            for (int j = cuttingPoint1; j <= cuttingPoint2; j++) {
                if (offspring2.path[j] == candidate2) {
                    candidate2 = parent2.path[j];
                    mapped2 = true;
                    break;
                }
            }
        }
        offspring2.path[i] = candidate2;
    }
}

bool terminationConditionNotMet(genetic_algorithm_state& state, int iterationsWithNoImprovement, int& iter){
    if (std::chrono::steady_clock::now() - state.start > state.timeLimit) {
            std::cout << "Time limit exceeded" << std::endl;
            return false;
        }
    if (state.bestSolutionCost <= state.acceptableSolutionCost) {
        std::cout << "Current solution meets requirements" << std::endl;
        return false; // Obecne rozwiązanie jest akceptowalne
    }
    if (state.iterationsWithoutImprovement >= iterationsWithNoImprovement) {
        std::cout << "Reached number of iterations without improvement" << std::endl;
        return false; // Przekroczono limit iteracji bez poprawy
    }
    return true; // Warunki zakończenia nie zostały spełnione
}

void showSpecimens(const std::vector<specimen>& vec){
    for(int i = 0; i < vec.size(); i++){
        std::cout << "Specimen " << i << " Fitness = " << vec[i].fitness << " : Path = ";
        for(int j = 0; j < vec[i].path.size(); j++){
            std::cout << vec[i].path[j] << " ";
        }
        std::cout << std::endl;
    }
}





