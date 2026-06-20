#pragma once
#include <string>
#include <vector>
#include <map>

struct AlgorithmConfig
{
    std::string name;
    std::map<std::string,std::string> params;
};

struct TestConfig
{
    std::string graphFile;
    int repetitions;
    std::vector<AlgorithmConfig> algorithms;
};