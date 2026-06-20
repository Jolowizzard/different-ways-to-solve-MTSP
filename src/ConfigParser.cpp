#include "Config.h"
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

std::vector<TestConfig> parseConfigMy(const std::string& filename)
{
    std::ifstream file(filename);

    json j;
    file >> j;

    std::vector<TestConfig> tests;

    for(const auto& testJson : j["tests"])
    {
        TestConfig test;

        test.graphFile = testJson["graph"];
        test.repetitions = testJson["repetitions"];

        for(const auto& algJson : testJson["algorithms"])
        {
            AlgorithmConfig alg;

            alg.name = algJson["name"];

            if(algJson.contains("params"))
            {
                for(auto& [key,value] : algJson["params"].items())
                {
                    alg.params[key] = value;
                }
            }

            test.algorithms.push_back(alg);
        }

        tests.push_back(test);
    }

    return tests;
}