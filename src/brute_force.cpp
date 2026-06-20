#include "brute_force.h"
#include <iostream>


void swap(std::vector<int>& tab, int a, int b){
    int temp = tab.at(a);
    tab.at(a) = tab.at(b);
    tab.at(b) = temp;
}

alghSolution bruteForce(const TSPLIBGraph& graph){
    alghSolution solution;
    int liczbaWierzcholkow = graph.size();

    std::vector<int> permutacje(liczbaWierzcholkow);
    std::vector<int> perm(liczbaWierzcholkow, 0);

    for(int i = 0; i < liczbaWierzcholkow; i++){
        permutacje.at(i) = i;
    }

    int c = 3;
    int finalCount = 0;
    int bestPathWeight = __INT_MAX__;
    std::vector<int> bestPath(liczbaWierzcholkow);

    //algorytm działa dla liczby wierzchołków większej od 3
    while(finalCount < (liczbaWierzcholkow - 1) ){

        for(int i = 0; i < 6; i++){
            // for(int i = 0; i < liczbaWierzcholkow; i++){
            //     std::cout << permutacje[i] << " ";
            // }
            // std::cout << std::endl;
            // //Algorytm
            int pathWeight = calculatePathWeight(graph,permutacje);

            if(pathWeight < bestPathWeight){
                bestPathWeight = pathWeight;
                bestPath = permutacje;
            }

            int temp = permutacje[i % 3];
            permutacje[i % 3] = permutacje[(i + 1) % 3];
            permutacje[(i + 1) % 3] = temp;
        }

        for(int i = 0; i < c; i++){
            swap(permutacje, i % (c + 1), (i + 1) % (c + 1));
        }

        perm[c]++;

        if(perm[c] >= c + 1){

            // perm.at(c) = 0;
            // c++;

            // for(int i = 0; i < c; i++){
            //     swap(permutacje, i % (c + 1), (i + 1) % (c + 1));
            // }

            // if(c < liczbaWierzcholkow)
            //     perm.at(c)++;

            while(c < (liczbaWierzcholkow - 1) && perm[c] >= c + 1){
                
                finalCount = perm[c];
                perm[c] = 0;
                c++;

                for(int i = 0; i < c; i++){
                    swap(permutacje, i % (c + 1), (i + 1) % (c + 1));
                }

                perm[c]++;

                // if(c == liczbaWierzcholkow - 1 && previousValue != perm.at(c)){
                //     //std::cout << perm.at(c) << std::endl;
                //     //previousValue = perm.at(c);
                // }
                //finalCount = perm.at(c);
            }

            c = 3;
            
        }
    }

    solution.path = bestPath;
    solution.pathWeight = bestPathWeight;
    return solution;
}