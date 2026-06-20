#include "tsplib_graph.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <cmath> 
#include <vector>
#include <algorithm>

// Pomocnicza funkcja do usuwania białych znaków
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

bool TSPLIBGraph::load(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file) return false;

    std::string line;
    std::string edgeWeightFormat = "";
    std::string edgeWeightType = "";

    while (std::getline(file, line))
    {
        if (line.find("TYPE") != std::string::npos && line.find("EDGE") == std::string::npos) {
            if (line.find("ATSP") != std::string::npos) atsp = true;
        }
        else if (line.find("DIMENSION") != std::string::npos) {
            dimension = std::stoi(line.substr(line.find(":") + 1));
            weights.assign(dimension, std::vector<int>(dimension, 0));
        }
        else if (line.find("EDGE_WEIGHT_TYPE") != std::string::npos) {
            edgeWeightType = trim(line.substr(line.find(":") + 1));
        }
        else if (line.find("EDGE_WEIGHT_FORMAT") != std::string::npos) {
            edgeWeightFormat = trim(line.substr(line.find(":") + 1));
        }
        else if (line.find("EDGE_WEIGHT_SECTION") != std::string::npos) {
            readEdgeWeightSection(file, edgeWeightFormat);
            break; 
        }
        else if (line.find("NODE_COORD_SECTION") != std::string::npos) {
            readNodeCoordSection(file, edgeWeightType);
            break;
        }
    }
    return true;
}

void TSPLIBGraph::readEdgeWeightSection(std::ifstream& file, const std::string& format)
{
    // Obsługa brg180 (LOWER_DIAG_ROW) i innych formatów macierzowych
    for (int i = 0; i < dimension; i++) {
        int limit = (format == "FULL_MATRIX" || atsp) ? dimension : 
                    (format == "LOWER_DIAG_ROW") ? i + 1 : 
                    (format == "LOWER_ROW") ? i : 0;

        for (int j = 0; j < limit; j++) {
            int w;
            if (!(file >> w)) break;
            weights[i][j] = w;
            if (!atsp) weights[j][i] = w; // Symetria
        }
    }
}

void TSPLIBGraph::readNodeCoordSection(std::ifstream& file, const std::string& type)
{
    std::vector<double> x(dimension), y(dimension);
    int id;
    for (int i = 0; i < dimension; i++) {
        file >> id >> x[i] >> y[i];
    }

    for (int i = 0; i < dimension; i++) {
        for (int j = 0; j < dimension; j++) {
            if (i == j) {
                weights[i][j] = 0;
                continue;
            }

            if (type == "GEO") {
                weights[i][j] = calculateGeoDistance(x[i], y[i], x[j], y[j]);
            } else {
                double dx = x[i] - x[j];
                double dy = y[i] - y[j];
                weights[i][j] = static_cast<int>(std::round(std::sqrt(dx * dx + dy * dy)));
            }
        }
    }
}

int TSPLIBGraph::calculateGeoDistance(double x1, double y1, double x2, double y2) {
    auto convertToRad = [](double coord) {
        double PI = 3.141592;
        int deg = static_cast<int>(coord);
        double min = coord - deg;
        return PI * (deg + 5.0 * min / 3.0) / 180.0;
    };

    double lat1 = convertToRad(x1), lon1 = convertToRad(y1);
    double lat2 = convertToRad(x2), lon2 = convertToRad(y2);
    double RRR = 6378.388;

    double q1 = cos(lon1 - lon2);
    double q2 = cos(lat1 - lat2);
    double q3 = cos(lat1 + lat2);
    
    // Oficjalny wzór z dokumentacji TSPLIB
    return static_cast<int>(RRR * acos(0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3)) + 1.0);
}

int TSPLIBGraph::size() const
{
    return dimension;
}

int TSPLIBGraph::weight(int u, int v) const
{
    return weights[u][v];
}

const std::vector<std::vector<int>>& TSPLIBGraph::matrix() const
{
    return weights;
}

bool TSPLIBGraph::isATSP() const
{
    return atsp;
}

int calculatePathWeight(const TSPLIBGraph& graph, const std::vector<int>& vertices) {
    if (vertices.empty()) return 0;
    
    int pathWeight = 0;
    for(size_t i = 0; i < vertices.size() - 1; i++){
        pathWeight += graph.weight(vertices[i], vertices[i+1]);
    }
    pathWeight += graph.weight(vertices.back(), vertices.front());
    
    return pathWeight;
}

int calculatePathWeightStatic(const TSPLIBGraph& graph, int vertices []){
    int size = graph.size();
    if (size == 0) return 0;

    int pathWeight = 0;
    for(int i = 0; i < size - 1; i++){
        pathWeight += graph.weight(vertices[i], vertices[i+1]);

    }
    pathWeight += graph.weight(vertices[size - 1], vertices[0]);
    
    return pathWeight;
}
