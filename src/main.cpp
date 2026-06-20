#include <iostream>
#include "json.hpp"
#include <chrono>
#include <tsplib_graph.h>
#include <Config.h>
#include "ConfigParser.h"
#include "brute_force.h"
#include "algh_solution.h"
#include "nearest_neighbour.h"
#include <fstream>
#include <string>
#include "rand_search.h"
#include <optional>
//#include "breath_first_search.h"
#include "depth_first_search.h"
#include "lowest_cost.h"
#include "tabu_search_basic.h"
#include "tabu_search_complex.h"
#include "genetic_algorithm.h"

template<typename F>
double measureTime(F func)
{
    auto start = std::chrono::high_resolution_clock::now();

    func();

    auto stop = std::chrono::high_resolution_clock::now();

    return std::chrono::duration<double,std::milli>(stop-start).count();
}

alghSolution runAlgorithm(const AlgorithmConfig& alg,
                  const TSPLIBGraph& graph)
{
    alghSolution solution;
    // if(alg.name == "bruteForce"){
    //      solution = bruteForce(graph);
    // }

    // else if(alg.name == "breadth_first_search")
    // {
    //     int time = std::stoi(alg.params.at("maxRunningTime"));
    //     int maxMemoryUsageMB = std::stoi(alg.params.at("maxMemoryUsageMB"));
    //     bool findUpperBound = std::stoi(alg.params.at("findUpperBound"));
    //     //int vertexNumber = std::stoi(alg.params.at("startVertex"));
    //     bool repetetive = std::stoi(alg.params.at("repetetive"));
    //     //bool randomStart = std::stoi(alg.params.at("randomStart"));
    //     bool ties = std::stoi(alg.params.at("ties"));
    //     solution = breathFirstSearch(graph, time, maxMemoryUsageMB , findUpperBound, repetetive, ties);
    // }
    // else if(alg.name == "depth_first_search"){
    //     int time = std::stoi(alg.params.at("maxRunningTime"));
    //     int maxMemoryUsageMB = std::stoi(alg.params.at("maxMemoryUsageMB"));
    //     bool findUpperBound = std::stoi(alg.params.at("findUpperBound"));
    //     bool repetetive = std::stoi(alg.params.at("repetetive"));
    //     bool ties = std::stoi(alg.params.at("ties"));
    //     solution = depthFirstSearch(graph, time, maxMemoryUsageMB , findUpperBound, repetetive, ties);
    // }
    // else if(alg.name == "lowest_cost_search"){
    //     int time = std::stoi(alg.params.at("maxRunningTime"));
    //     int maxMemoryUsageMB = std::stoi(alg.params.at("maxMemoryUsageMB"));
    //     bool findUpperBound = std::stoi(alg.params.at("findUpperBound"));
    //     bool repetetive = std::stoi(alg.params.at("repetetive"));
    //     bool ties = std::stoi(alg.params.at("ties"));
    //     solution = lowestCost(graph, time, maxMemoryUsageMB, findUpperBound, repetetive, ties);
    // }

    if(alg.name == "tabu_search")
    {
        int time = std::stoi(alg.params.at("executionTime"));
        int cadentionTime = std::stoi(alg.params.at("cadentionTime"));
        bool findUB = std::stoi(alg.params.at("findUB"));
        int IWNI = std::stoi(alg.params.at("iterationsWithNoImprovement"));
        float al = std::stof(alg.params.at("acceptanceLevel"));
        int LB = std::stoi(alg.params.at("opt"));
        solution = tabuSearchBasicAlgorithm(graph, time, cadentionTime, findUB, IWNI, al, LB);
    }
    else if(alg.name == "tabu_search_complex"){
        int time = std::stoi(alg.params.at("executionTime"));
        int cadentionTime = std::stoi(alg.params.at("cadentionTime"));
        bool findUB = std::stoi(alg.params.at("findUB"));
        int IWNI = std::stoi(alg.params.at("iterationsWithNoImprovement"));
        float al = std::stof(alg.params.at("acceptanceLevel"));
        int tabuSize = std::stoi(alg.params.at("tabuListSize"));
        int iterations = std::stoi(alg.params.at("iterationsToChangeSize"));
        int LB = std::stoi(alg.params.at("opt"));

        solution = tabuSearchComplexAlgorithm(graph, time, cadentionTime, findUB, IWNI, al, tabuSize, iterations, LB);
    }
    else if(alg.name == "genetic_algorithm"){
        int time = std::stoi(alg.params.at("executionTime"));
        int populationSize = std::stoi(alg.params.at("populationSize"));
        int parentSize = std::stoi(alg.params.at("parentSize"));
        int offspringSize = std::stoi(alg.params.at("offspringSize"));
        int eliteSize = 0;
        float mutationRate = std::stof(alg.params.at("mutationRate"));
        float crossoverRate = std::stof(alg.params.at("crossoverRate"));
        int tournamentSize = std::stoi(alg.params.at("tournamentSize"));
        bool computeUB = false;
        int IWNI = std::stoi(alg.params.at("iterationsWithNoImprovement"));
        float al = std::stof(alg.params.at("acceptanceLevel"));
        int LB = std::stoi(alg.params.at("opt"));

        solution = geneticAlgorithm(graph, time, populationSize, parentSize, offspringSize, eliteSize, mutationRate, crossoverRate, tournamentSize, computeUB, IWNI, al, LB);
    }
    else
    {
        std::cout<<"Unknown algorithm\n";
    }

    return solution;
}

void saveIterationsCSV(const std::string& graphName, 
                       const std::string& algorithm, 
                       int repetition, 
                       const std::vector<int>& solutionsInTime) 
{
    // Generowanie unikalnej nazwy pliku, np: "iterations_tabu_search_rep0.csv"
    // Możesz też dodać nazwę grafu, jeśli chcesz mieć to lepiej pogrupowane.
    std::string fileName = "iter_" + algorithm + "_rep" + std::to_string(repetition) + ".csv";
    std::ofstream file(fileName);

    if (!file.is_open()) {
        std::cerr << "Blad: Nie udalo sie otworzyc pliku iteracji: " << fileName << "\n";
        return;
    }

    file << "Iteration,Cost\n";

    int totalIterations = solutionsInTime.size();
    if (totalIterations == 0) return;

    // Dynamiczny dobór kroku - celujemy w max 5000 punktów danych dla Excela
    int step = (totalIterations > 5000) ? (totalIterations / 5000) : 1; 

    for(int i = 0; i < totalIterations; i += step) {
        file << i << "," << solutionsInTime[i] << "\n";
    }
    
    // Upewniamy się, że zapisaliśmy absolutnie ostatnią iterację algorytmu
    if ((totalIterations - 1) % step != 0) {
        file << (totalIterations - 1) << "," << solutionsInTime.back() << "\n";
    }

    file.close();
}

void saveCSV(std::ofstream& file,
             const std::string& graph,
             const std::string& algorithm,
             int size,
             alghSolution solution,
             double time)
{
    file << graph << ","
         << algorithm << ","
         << solution.tabuListSize << ','
         << solution.cadentionTime << ','
         << size << ","
         << solution.pathWeight << ","
         << solution.interationsTotal << ","
         << time << ",";

    if(algorithm == "random"){
        for(int i = 0; i < solution.solutionsInTime.size(); i++){
            file << solution.solutionsInTime[i] << ",";
        }
    }
    
    //Zapisanie najlepszej ścieżki
    std::string path = {};
    for(int i = 0; i < solution.path.size(); i++){
        path = path + std::to_string(solution.path[i]) + "->";
    }
    file << path << "," << "END" << "\n";
    file.flush();
}


std::vector<TestConfig> parseConfigMy(const std::string&);

int main(int argc, char* argv[]){

    if (argc != 3) {
        std::cerr << "Użycie programu: " << argv[0] << " <sciezka_do_config.json> <sciezka_do_wynikow.csv>\n";
        return 1; // Zwrócenie kodu błędu
    }

    std::string configPath = argv[1];
    std::string csvPath = argv[2];

    auto tests = parseConfigMy(configPath);

    std::ofstream results(csvPath);

    if (!results.is_open()) {
        std::cerr << "Blad: Nie mozna otworzyc pliku do zapisu: " << csvPath << "\n";
        return 1; 
    }

    results << "Graph,Algorithm,TabuListSize,CadentionTime,Size,Solution,IterationsTotal,Time\n";

    for(const auto& test : tests)
    {

        TSPLIBGraph graph;

        if(!graph.load(test.graphFile))
        {
            std::cout<<"Cannot load graph\n";
            continue;
        }

        int size = graph.size();
        
        std::cout << "Loaded graph : " << test.graphFile << std::endl;

        for(const auto& alg : test.algorithms)
        {
            std::cout << "#######################" << std::endl;

            std::cout << "Testing " << alg.name << std::endl;

            for(int i=0;i<test.repetitions;i++)
            {
                alghSolution solution;
                double time = measureTime([&]()
                {
                   solution = runAlgorithm(alg,graph);
                });

                saveCSV(results,
                        test.graphFile,
                        alg.name,
                        size, 
                        solution,
                        time);
                if(alg.name == "tabu_search" || alg.name == "tabu_search_complex" || alg.name == "random"){
                    saveIterationsCSV(test.graphFile, alg.name, i, solution.solutionsInTime);
                }
                std::cout << i << " repetition of " << alg.name << " ended. Lasted :" << time << std::endl; 

                if(solution.pathWeight == -1){
                    std::cout << "Skipping other repetitions" << std::endl;
                    break;
                }
            }
        }
    }

    std::cout<<"Tests finished\n";

    return 0;
}