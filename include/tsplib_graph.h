#pragma once

#include <vector>
#include <string>

class TSPLIBGraph {
public:

    bool load(const std::string& filename);

    int size() const;

    int weight(int u, int v) const;

    const std::vector<std::vector<int>>& matrix() const;

    bool isATSP() const;

private:

    int dimension = 0;

    bool atsp = false;

    std::vector<std::vector<int>> weights;

    void readEdgeWeightSection(std::ifstream& file, const std::string& format);

    void readNodeCoordSection(std::ifstream& file, const std::string& type);

    int calculateGeoDistance(double x1, double y1, double x2, double y2); 
};

int calculatePathWeight(const TSPLIBGraph& graph, const std::vector<int>& vertices); // Util method to calculate finale route value - used in every algorithm

int calculatePathWeightStatic(const TSPLIBGraph& graph, int verices []); // Util method to calculate finale route value - used in every algorithm
