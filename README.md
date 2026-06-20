# Metric Traveling Salesman Problem - algorithms

## Subject of studie

PThis program was written for the university course Designing Effective Algorithms. The main goal was to implement various algorithms, design experiments, and empirically analyze the time complexity of the studied algorithms. This repository only contains the implemented algorithms and a program which enables the user to perform tests using configuration files in .json format.

## Implemented algorithms

### Brute-force
Calculates the path weight for every Hamiltonian cycle to determine which one is optimal. Subsequent solutions are generated using a custom algorithm. The time complexity is O((n-1)!) for TSP and ATSP (the algorithm does not check mirror paths).  

### Greedy - Nearest-Neighbour - Repetetive-Nearest-Neighbour
Builds a solution vertex by vertex, always choosing the one connected by the cheapest edge. The user can change the behavior of the algorithm by adjusting the starting vertex, enabling starting from every vertex to select the best path (Repetitive-NN), and changing the way in which ties are settled (when two or more edges have the same cost, multiple paths are checked).

### Rand
The algorithm uses the `<random>` library and the `std::mt19937` generator to build a random path. The best solution found during the algorithm's run is stored. The user can choose how long the algorithm should run. 

### Branch & Bound
The algorithm builds a "solutions tree" vertex by vertex, calculating the current path cost. When the cost in one of the nodes exceeds or equals the upper bound (the best complete solution found during the run), this branch is cut. B&B was implemented in three ways: DFS, BFS, and Best-First-Search (also known as lowest-cost). To create and store solutions, `std::vector` objects were used. Due to a suboptimal implementation, every new branch of the tree allocates a vector object on the fly. In BFS, `std::queue` was used as the queue. In DFS, `std::vector` was used as the stack. In lowest-cost, `std::priority_queue` was used as the priority queue.

### Tabu Search
The algorithm searches through the solution space, combining exploatation (hill-climb like behavior) and exploration (tabu list). Tabu list was implemented as matrix NxN, where N is the number of vertices in graph. Cells in the matrix store information regarding up to wich iteration a move (switching two vertices) will be locked. This implementation eliminates the need to updated (decrement) the cadence of the move on the tabu list in every iteration. 2-opt is currently being used, but swap was also implemented and is availabe in the code. For better exploration, an oscilating tabu list size was implemented. 

### Genetic Algorithm
Metaheuristic algorithm using evolutionary approach to solve certain problems. Selection was implemented in form of a tournament. The succesion type is (&mu; + &lambda;), where &mu; is generated offspring and &lambda; is previous population. To create the new population from previos one, `std::sort` was used. PMX was used for crossing parents and create offspring (every pair of parents create pair of offspring). For mutation 2-opt operation was used.

## Guide

To use the program, it first needs to be built using CMake. 

### Linux

```text
mkdir build
cd build

cmake ..
cmake --build
```

### Windows

```text
mkdir build-win
cd build-win

cmake .. \
 -DCMAKE_SYSTEM_NAME=Windows \
 -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
 -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++

cmake --build .
```

### Starting the program

Program requires two parameters: path to the config file and path to the output .csv file in wich user wants to save their data. The config file has ordered structure.

```text
{
    "tests":[
    {
      "description" : "graph: path to graph (can be relative), repetitions: number of repetitions for test, algorithms: algorithms that are being tested",
      "graph": "example/path/to/graph/somegraph1234.tsp",
      "repetitions": 4,
      "algorithms": [
        {
          "description" : "genetic_algorithm, requires : executionTime, populationSize, parentSize, offspringSize, mutationRate, crossoverRate, tournamentSize, iterationsWithNoImprovement, acceptanceLevel, opt ",
          "name": "genetic_algorithm",
          "params":{
            "executionTime":"900",
            "populationSize":"500",
            "parentSize":"300",
            "offspringSize":"500",
            "mutationRate":"0.65",
            "crossoverRate":"1.00",
            "tournamentSize":"3",
            "iterationsWithNoImprovement":"10000",
            "acceptanceLevel":"0.0",
            "opt":"234256"
          }
        },
        {
          "description" : "genetic_algorithm, requires : executionTime, populationSize, parentSize, offspringSize, mutationRate, crossoverRate, tournamentSize, iterationsWithNoImprovement, acceptanceLevel, opt ",
          "name": "genetic_algorithm",
          "params":{
            "executionTime":"900",
            "populationSize":"500",
            "parentSize":"300",
            "offspringSize":"500",
            "mutationRate":"0.65",
            "crossoverRate":"1.00",
            "tournamentSize":"3",
            "iterationsWithNoImprovement":"10000",
            "acceptanceLevel":"0.0",
            "opt":"234256"
          }
        }
      ]
    },
    {
      "description" : "graph: path to graph (can be relative), repetitions: number of repetitions for test, algorithms: algorithms that are being tested",
      "graph": "example/path/to/graph/somegraph1234.tsp",
      "repetitions": 4,
      "algorithms": [
        {
          "description" : "genetic_algorithm, requires : executionTime, populationSize, parentSize, offspringSize, mutationRate, crossoverRate, tournamentSize, iterationsWithNoImprovement, acceptanceLevel, opt ",
          "name": "genetic_algorithm",
          "params":{
            "executionTime":"900",
            "populationSize":"500",
            "parentSize":"300",
            "offspringSize":"500",
            "mutationRate":"0.65",
            "crossoverRate":"1.00",
            "tournamentSize":"3",
            "iterationsWithNoImprovement":"10000",
            "acceptanceLevel":"0.0",
            "opt":"234256"
          }
        }
      ]
    }
    ]
}
```

Different tests can be stacked and combined as shown above. Also, one tested graph can have many tested algorithms. Full config, in wich every algorithm's parameters are described, is included in the repository.

### Graph reader
Graphs that were used during studie were in TSPLIB format. Parser supports following representations:

* Graphs types:
    * TSP
    * ATSP
* Matrix Formats (`EDGE_WEIGHT_SECTION`):
    * `FULL_MATRIX` - Full NxN weight matrix
    * `LOWER_DIAG_ROW` - Lower triangular matrix includinf the main diagonal.
    * `LOWER_ROW` - Lower triangular matrix excluding the main diagonal.
* Coordinate formats (`NODE_COORD_SECTION`):
    * `GEO` - Geographical coordinates (computed using the official TSPLIB great-circle distance formula).
    * `2D_EUCLIDEAN` - Standard Cartesian coordinates (default, computed using the standard round-to-nearest integer Euclidean distance formula).