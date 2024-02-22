#include "H5Cpp.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>
#include <math.h>
#include <filesystem>

#include "test.h"
#include "parameters.hpp"

#define ENUM_STRINGIFY(enum, member) case enum::member: return #member;

using namespace std::chrono;

struct DataSpace
{
    hsize_t offset[2];
    hsize_t size[2];
};

struct Scenario
{
    std::string name;
    DataSpace fileSpace;
    DataSpace testSpace;
    AccessPattern accessPattern;
    int accessAmount;
    int repetitions;
    CacheShape cacheShape;
    int chunkSize;
    uint64_t cacheLimit;
    EvictionStrategy evictionStrategy;
};

struct ProfileResult 
{
    std::vector<int> durations;
    std::vector<bool> validationResults;
};

void runScenario(Scenario scenario, bool isSilent);
void runScenarios(std::vector<Scenario> scenarios, bool isSilent);
std::vector<Scenario> createScenarioPermutation(DataSpace fileSpace, DataSpace testSpace, int repetitions, int accessAmount);
ProfileResult profiledRead(uint8_t buffer[], H5::DataSet dataset, H5::DataType dataType, H5::DataSpace memorySpace, H5::DataSpace dataSpace, int repetitions);
void printAsCSV(uint8_t buffer[], size_t size);
void printAsDat(std::string filePath, std::vector<int> list);
std::string fill(std::string string, char filler, int count);
void printTable(std::vector<int> &durations, hsize_t sizes[]);
void printList(std::vector<int> &durations, int size);
void printGraph(std::vector<int> &durations, int minimum, int maximum);
const std::string toString(EvictionStrategy strategy);
const std::string toString(AccessPattern pattern);
const std::string toString(CacheShape value);
int toValue(CacheChunkSize value);
uint64_t toValue(CacheLimit value, DataSpace dataSpace);

int main(void)
{    
    Scenario s1 = {
        .name = "Standard Line",
        .fileSpace = {
            .offset = {0, 0},
            .size = {16*1024, 16*1024},
        },
        .testSpace = {
            .offset = {0, 0},
            .size = {4*1024, 4*1024},
        },
        .accessPattern = AccessPattern::COHERENT_REGION,
        .accessAmount = 1,
        .repetitions = 3,
        .cacheShape = CacheShape::LINE,
        .chunkSize = 1024,
        .cacheLimit = 2ULL*1024*1024*1024,
        .evictionStrategy = EvictionStrategy::FIFO,
    };
    
    Scenario s2 = {
        .name = "Standard Square",
        .fileSpace = {
            .offset = {0, 0},
            .size = {16*1024, 16*1024},
        },
        .testSpace = {
            .offset = {0, 0},
            .size = {4*1024, 4*1024},
        },
        .accessPattern = AccessPattern::COHERENT_REGION,
        .accessAmount = 1,
        .repetitions = 3,
        .cacheShape = CacheShape::SQUARE,
        .chunkSize = 1024,
        .cacheLimit = 2ULL*1024*1024*1024,
        .evictionStrategy = EvictionStrategy::FIFO,
    };
    
    Scenario t1 = {
        .name = "Test",
        .fileSpace = {
            .offset = {0, 0},
            .size = {16*1, 16*1},
        },
        .testSpace = {
            .offset = {0, 0},
            .size = {4*1, 4*1},
        },
        .accessPattern = AccessPattern::COHERENT_REGION,
        .accessAmount = 1,
        .repetitions = 3,
        .cacheShape = CacheShape::LINE,
        .chunkSize = 1024,
        .cacheLimit = 2ULL*1024*1024*1024,
        .evictionStrategy = EvictionStrategy::FIFO,
    };

    //std::vector<Scenario> scenarios = {t1};
    //std::vector<Scenario> scenarios = {s2, s1};
    DataSpace fileSpace = {
        .offset = {0, 0},
        .size = {16*1024, 16*1024},
    };
    DataSpace testSpace = {
        .offset = {0, 0},
        .size = {4*1024, 4*1024},
    };
    std::vector<Scenario> scenarios = createScenarioPermutation(fileSpace, testSpace, 1, 1);

    runScenarios(scenarios, true);
}

std::vector<Scenario> createScenarioPermutation(DataSpace fileSpace, DataSpace testSpace, int repetitions, int accessAmount)
{
    AccessPattern accessPattern = AccessPattern::FULLY_RANDOM;
    CacheShape cacheShape = CacheShape::SQUARE;
    Layout layout = Layout::ALIGNED;
    CacheChunkSize chunkSize = CacheChunkSize::SMALL;
    CacheLimit cacheLimit = CacheLimit::TOO_LOW_FACTOR_0_9;
    EvictionStrategy evictionStrategy = EvictionStrategy::FIFO;

    std::vector<Scenario> scenarios;
    Scenario s;
    for (accessPattern = AccessPattern::FULLY_RANDOM; accessPattern < AccessPattern::COUNT; ++accessPattern)
    {
        for (cacheShape = CacheShape::SQUARE; cacheShape < CacheShape::COUNT; ++cacheShape)
        {
            for (layout = Layout::ALIGNED; layout < Layout::COUNT; ++layout)
            {
                for (chunkSize = CacheChunkSize::SMALL; chunkSize < CacheChunkSize::COUNT; ++chunkSize)
                {
                    for (cacheLimit = CacheLimit::ENOUGH_FACTOR_5; cacheLimit < CacheLimit::COUNT; ++cacheLimit)
                    {
                        for (evictionStrategy = EvictionStrategy::FIFO; evictionStrategy < EvictionStrategy::COUNT; ++evictionStrategy)
                        {
                            s = {
                                .name = toString(accessPattern) + "-" + toString(cacheShape) + "-" + toString(layout) 
                                    + "-" + toString(chunkSize) + "-" + toString(cacheLimit) + "-" + toString(evictionStrategy),
                                //.name = toString(cacheShape) 
                                //    + "::" + toString(chunkSize) + "::" + toString(cacheLimit) + "::" + toString(evictionStrategy),
                                .fileSpace = fileSpace,
                                .testSpace = testSpace,
                                .accessPattern = accessPattern,
                                .accessAmount = accessAmount,
                                .repetitions = repetitions,
                                .cacheShape = cacheShape,
                                .chunkSize = toValue(chunkSize),
                                .cacheLimit = toValue(cacheLimit, testSpace),
                                .evictionStrategy = evictionStrategy,
                            };

                            scenarios.push_back(s);
                        }
                    }
                }
            }
        }
    }

    return scenarios;
}

void runScenarios(std::vector<Scenario> scenarios, bool isSilent)
{
    for (Scenario s : scenarios)
    {
        runScenario(s, isSilent);
    }    
}

void runScenario(Scenario scenario, bool isSilent)
{
    setenv("STAGING_CACHE_SHAPE", toString(scenario.cacheShape).c_str(), 1);
    setenv("STAGING_CHUNK_SIZE", std::to_string(scenario.chunkSize).c_str(), 1);
    setenv("STAGING_CACHE_LIMIT", std::to_string(scenario.cacheLimit).c_str(), 1);
    setenv("STAGING_EVICTION_STRATEGY", toString(scenario.evictionStrategy).c_str(), 1);

    H5::H5File file = useTestFile(2, scenario.fileSpace.size);
        
    H5::DataSet dataset = file.openDataSet("testData");
    H5::DataType dataType = dataset.getDataType();

    H5::DataSpace dataSpace = dataset.getSpace();
    dataSpace.selectHyperslab(H5S_SELECT_SET, scenario.testSpace.size, scenario.testSpace.offset);

    H5::DataSpace memorySpace(2, scenario.testSpace.size);
    memorySpace.selectHyperslab(H5S_SELECT_SET, scenario.testSpace.size, scenario.testSpace.offset);

    std::cout << "Profiling scenario " << scenario.name << "..." << std::endl;

    uint64_t* buffer = new uint64_t[scenario.testSpace.size[0] * scenario.testSpace.size[1]];
    auto profileResult = profiledRead((uint8_t*)buffer, dataset, dataType, memorySpace, dataSpace, scenario.repetitions);
    dataset.close();

    if (!isSilent)
    {
        printTable(profileResult.durations, scenario.testSpace.size);
        std::cout << std::endl;
        printList(profileResult.durations, std::min((int)profileResult.durations.size(), 12));
        std::cout << std::endl;
        printGraph(profileResult.durations, *std::min_element(profileResult.durations.begin(), profileResult.durations.end()), *std::max_element(profileResult.durations.begin(), profileResult.durations.end()));
        std::cout << std::endl;
        printAsDat(" ", profileResult.durations);

        hsize_t printOffset[] = {0, 0};
        hsize_t printSize[] = {8, 8};
        printBuffer(buffer, 2, scenario.testSpace.size, printOffset, printSize);
    }

    if (!std::all_of(profileResult.validationResults.begin(), profileResult.validationResults.end(), [](bool b) {return b;}))
    {
        std::cout << "FAIL" << std::endl;
    }
    else
    {
        std::cout << "PASS" << std::endl;
    }

    delete[] buffer;    
}

ProfileResult profiledRead(uint8_t buffer[], H5::DataSet dataset, H5::DataType dataType, H5::DataSpace memorySpace, H5::DataSpace dataSpace, int repetitions)
{        
    hsize_t dimensions[2];
    dataSpace.getSimpleExtentDims(nullptr, dimensions);
    hsize_t offset[2];
    hsize_t dummy[2];
    memorySpace.getSelectBounds(offset, dummy);
    hsize_t size[2];
    memorySpace.getSimpleExtentDims(size, nullptr);

    steady_clock::time_point start = steady_clock::now();
    steady_clock::time_point end = steady_clock::now();

    microseconds minimum(999999999999999);
    microseconds maximum(0);
    microseconds sum(0);

    int minimumIndex = 0;
    int maximumIndex = 0;

    ProfileResult profileResult;

    profileResult.durations.reserve(repetitions);
    profileResult.validationResults.reserve(repetitions);

    for (size_t i = 0; i < repetitions; i++)
    {
        start = steady_clock::now();
        dataset.read(buffer, dataType, memorySpace, dataSpace);
        end = steady_clock::now();

        microseconds duration = duration_cast<microseconds>(end - start);
        profileResult.durations.push_back(duration.count());           
        profileResult.validationResults.push_back(verifyBuffer((uint64_t*)buffer, 2, dimensions, offset, size));
    }

    return profileResult;
}

void printAsCSV(uint8_t buffer[], size_t size)
{
    for (size_t i = 0; i < size; i++)
    {        
        std::cout << std::to_string(buffer[i]) << std::endl;
    }    
}

void printAsDat(std::string filePath, std::vector<int> list)
{
    for (int i = 0; i < list.size(); ++i)
    {
        std::cout << i << " " << list[i] << std::endl;
    }
}

std::string fill(std::string string, char filler, int count)
{
    return string + std::string(std::min(0ul, count - string.length()), filler);
}

void printTable(std::vector<int> &durations, hsize_t sizes[])
{

    int average = std::accumulate(durations.begin(), durations.end(), 0) / durations.size();
    auto minimumIterator = std::min_element(durations.begin(), durations.end());
    int minimumIndex = std::distance(durations.begin(), minimumIterator);
    int minimum = *minimumIterator;
    auto maximumIterator = std::max_element(durations.begin(), durations.end());
    int maximumIndex = std::distance(durations.begin(), maximumIterator);
    int maximum = *maximumIterator;

    size_t bytes = sizes[0] * sizes[1] * 8;
    float throughputMinimum = bytes / (minimum / 1e+6) / 1024 / 1024;
    float throughputAverage = bytes / (average / 1e+6) / 1024 / 1024;
    float throughputMaximum = bytes / (maximum / 1e+6) / 1024 / 1024;

    std::cout << "+---------------- dataset.read(...) ----------------+" << std::endl;
    std::cout << "| repetitions:    " << fill(std::to_string(durations.size()), ' ', 6) << fill(" ", ' ', 28) << "|" << std::endl;
    std::cout << "| minimum time:   " << fill(std::to_string(minimum), ' ', 6) << " µs @ " << fill(std::to_string(minimumIndex), ' ', 7) << "   " << round(throughputMinimum) << " MiB/s  " << "|" << std::endl;
    std::cout << "| average time:   " << fill(std::to_string(average), ' ', 6) << " µs " << fill(" ", ' ', 9) << "   " << round(throughputAverage) << " MiB/s  " << "|" << std::endl;
    std::cout << "| maximum time:   " << fill(std::to_string(maximum), ' ', 6) << " µs @ " << fill(std::to_string(maximumIndex), ' ', 8) << "   " << round(throughputMaximum) << " MiB/s  " << "|" << std::endl;
    std::cout << "+---------------------------------------------------+" << std::endl;
}

void printList(std::vector<int> &durations, int size)
{
    for (int i = 0; i < size - 2; i++)
    {
        std::cout << durations[i] << " µs, ";
    }
    std::cout << durations[size - 1] << " µs" << std::endl;    
}

void printGraph(std::vector<int> &durations, int minimum, int maximum)
{
    const int verticalLength = 20;
    bool filled[durations.size()][verticalLength] = {};

    double minPercentage = (double) minimum / (double) maximum;
    double minScale = std::floor(std::log10(minPercentage));    
    double maxScale = 0.0;

    for (size_t i = 0; i < durations.size(); i++)
    {
        double percentage = (double)durations[i] / (double)maximum;
        double exponent = std::log10(percentage);
        int coordinate = (int)std::round((exponent - minScale) * verticalLength / (maxScale - minScale));
        coordinate = std::min(coordinate, verticalLength);
        coordinate = std::max(coordinate, 0);
        //int level = durations[i]*verticalLength/maximum;
        for (size_t j = 0; j < coordinate; j++)
        {
            filled[i][j] = true;
        }        
    }    

    for (size_t i = verticalLength - 1; i > 0; i--)
    {
        for (size_t j = 0; j < durations.size(); j++)
        {
            if (filled[j][i])
            {
                std::cout << '|';
            }
            else
            {
                std::cout << ' ';
            }
        }        
        
        std::cout << std::endl;
    }    
}

int toValue(CacheChunkSize value)
{
    switch (value)
    {
    case CacheChunkSize::SMALL: return 64;
    case CacheChunkSize::MEDIUM: return 1024;
    case CacheChunkSize::LARGE: return 4096;    
    default: return -1;
    }
}

uint64_t toValue(CacheLimit value, DataSpace dataSpace)
{
    uint64_t area = dataSpace.size[0] * dataSpace.size[1] * 8;

    switch (value)
    {
    case CacheLimit::TOO_LOW_FACTOR_0_1: return area / 10;
    case CacheLimit::TOO_LOW_FACTOR_0_5: return area / 2;
    case CacheLimit::TOO_LOW_FACTOR_0_9: return area * 9 / 10;
    case CacheLimit::ENOUGH_FACTOR_1: return area * 1;
    case CacheLimit::ENOUGH_FACTOR_5: return area * 5;
    default: return -1;
    }
}