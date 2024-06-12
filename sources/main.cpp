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
#include <fstream>

#include "test.h"
#include "pseudoRandom.h"
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
    std::string comment;
    DataSpace fileSpace;
    DataSpace testSpace;
    AccessPattern accessPattern;
    int accessAmount;
    int repetitions;
    int datasetCount;
    CacheShape cacheShape;
    int chunkSize;
    uint64_t cacheLimit;
    EvictionStrategy evictionStrategy;
    bool bufferedRead;
};

struct ProfileResult 
{
    std::vector<std::vector<int>> durations;
    std::vector<std::vector<bool>> validationResults;
};

struct ProfiledReadAccess
{
    H5::DataSpace memorySpace;
    H5::DataSpace dataSpace;
    int datasetIndex;
};

void runScenario(Scenario scenario, bool isSilent);
void runScenarios(std::vector<Scenario> scenarios, bool isSilent);
std::vector<Scenario> createScenarioPermutation(DataSpace fileSpace, DataSpace testSpace, int repetitions, int accessAmount);
void scrambleScenarios(std::vector<Scenario> &scenarios);
void profiledRead(uint8_t buffer[], std::vector<H5::DataSet> &dataset, H5::DataType dataType, std::vector<ProfiledReadAccess> &spaces, ProfileResult* profileResult);
void printAsCSV(uint8_t buffer[], size_t size);
void printAsDat(std::string filePath, std::vector<int> list);
void saveToFileAsDat(std::string filePath, std::vector<std::vector<int>> list, std::string name);
std::string fill(std::string string, char filler, int count);
std::vector<ProfiledReadAccess> createAccesses(Scenario scenario, H5::DataSet &dataset);
void printTable(std::vector<int> &durations, hsize_t sizes[]);
void printList(std::vector<int> &durations, int size);
void printGraph(std::vector<int> &durations, int minimum, int maximum);
const std::string toString(EvictionStrategy strategy);
const std::string toString(AccessPattern pattern);
const std::string toString(CacheShape value);
bool isOverlappingHyperslabInVector(std::vector<ProfiledReadAccess> spaces, hsize_t offset[2], hsize_t size[2]);
bool isOverlappingHyperslab(H5::DataSpace dataSpace1, hsize_t offset2[2], hsize_t opposite2[2]);
int toValue(CacheChunkSize value);
uint64_t toValue(CacheLimit value, DataSpace dataSpace, int accessAmount);
int toValue(AccessPattern pattern);
DataSpace createDataSpace(const DataSpace &baseDataSpace, const Layout &layout, const CacheChunkSize &chunkSize);

int main(void)
{    
    Scenario s1 = {
        .name = "Standard Line",
        .comment = "",
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
        .datasetCount = 1,
        .cacheShape = CacheShape::LINE,
        .chunkSize = 1024,
        .cacheLimit = 2ULL*1024*1024*1024,
        .evictionStrategy = EvictionStrategy::FIFO,
        .bufferedRead = false,
    };
    
    Scenario s2 = {
        .name = "Standard Square",
        .comment = "",
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
        .datasetCount = 1,
        .cacheShape = CacheShape::SQUARE,
        .chunkSize = 1024,
        .cacheLimit = 2ULL*1024*1024*1024,
        .evictionStrategy = EvictionStrategy::FIFO,
        .bufferedRead = false,
    };
    
    Scenario t1 = {
        .name = "Test",
        .comment = "",
        .fileSpace = {
            .offset = {0, 0},
            .size = {8*1, 8*1},
        },
        .testSpace = {
            .offset = {0, 2},
            .size = {4*1, 4*1},
        },
        .accessPattern = AccessPattern::COHERENT_REGION,
        .accessAmount = 1,
        .repetitions = 3,
        .datasetCount = 1,
        .cacheShape = CacheShape::SQUARE,
        .chunkSize = 4,
        .cacheLimit = 2ULL*1024*1024*1024,
        .evictionStrategy = EvictionStrategy::FIFO,
        .bufferedRead = false,
    };
        
    Scenario t2 = {
        .name = "Test",
        .comment = "",
        .fileSpace = {
            .offset = {0, 0},
            .size = {16*1024, 16*1024},
        },
        .testSpace = {
            .offset = {0, 0},
            .size = {256, 256},
        },
        .accessPattern = AccessPattern::COHERENT_REGION_REPETITIVE,
        .accessAmount = 12,
        .repetitions = 1,
        .datasetCount = 1,
        .cacheShape = CacheShape::SQUARE,
        .chunkSize = toValue(CacheChunkSize::SMALL),
        .cacheLimit = 256*256*8*25/100,
        .evictionStrategy = EvictionStrategy::FIFO,
        .bufferedRead = true,
    };

    //std::vector<Scenario> scenarios = {t1};
    //std::vector<Scenario> scenarios = {t2};
    //std::vector<Scenario> scenarios = {s2, s1};
    DataSpace fileSpace = {
        .offset = {0, 0},
        .size = {16*1024, 16*1024},
    };
    DataSpace testSpace = {
        .offset = {0, 0},
        .size = {256, 256},
    };
    
    std::vector<Scenario> scenarios = createScenarioPermutation(fileSpace, testSpace, 4, 12);
    scrambleScenarios(scenarios);

    runScenarios(scenarios, true);
}

std::vector<Scenario> createScenarioPermutation(DataSpace fileSpace, DataSpace testSpace, int repetitions, int accessAmount)
{
    ReadType readType = ReadType::UNBUFFERED_READ;
    AccessPattern accessPattern = AccessPattern::ALWAYS_THE_SAME;
    CacheShape cacheShape = CacheShape::SQUARE;
    Layout layout = Layout::ALIGNED;
    CacheChunkSize chunkSize = CacheChunkSize::SMALL;
    CacheLimit cacheLimit = CacheLimit::TOO_LOW_FACTOR_0_25;
    EvictionStrategy evictionStrategy = EvictionStrategy::FIFO;

    std::vector<Scenario> scenarios;
    Scenario s;
    for (readType = ReadType::UNBUFFERED_READ; readType < ReadType::COUNT; ++readType)
    {
        for (accessPattern = AccessPattern::FULLY_RANDOM; accessPattern < AccessPattern::COHERENT_REGION; ++accessPattern)
        {
            for (cacheShape = CacheShape::SQUARE; cacheShape < CacheShape::COUNT; ++cacheShape)
            {            
                //if (cacheShape != CacheShape::LINE
                //&& 
                //(accessPattern == AccessPattern::COHERENT_REGION_UNFAVOURABLE_TRAVERSAL
                //|| accessPattern == AccessPattern::COHERENT_REGION))
                //{
                //    continue;
                //}

                for (layout = Layout::ALIGNED; layout < Layout::COUNT; ++layout)
                {
                    if ((accessPattern == AccessPattern::FULLY_RANDOM || accessPattern == AccessPattern::FULLY_RANDOM_LIMITED
                    || accessPattern == AccessPattern::RANDOM_PATTERN 
                    || accessPattern == AccessPattern::BEYOND_DATASETS) 
                    && layout != Layout::OFFSET)
                    {
                        continue;
                    }

                    for (chunkSize = CacheChunkSize::SMALL; chunkSize < CacheChunkSize::COUNT; ++chunkSize)
                    {
                        if (cacheShape == CacheShape::LINE && chunkSize != CacheChunkSize::LARGE)
                        {
                            continue;
                        }

                        for (cacheLimit = CacheLimit::TOO_LOW_FACTOR_0_25; cacheLimit < CacheLimit::COUNT; ++cacheLimit)
                        {
                            for (evictionStrategy = EvictionStrategy::FIFO; evictionStrategy < EvictionStrategy::COUNT; ++evictionStrategy)
                            {     
                                s = {
                                    .name = toString(readType) + "-" + toString(accessPattern) + "-" + toString(cacheShape) + "-" 
                                        + toString(layout) + "-" + toString(chunkSize) + "-" + toString(cacheLimit) + "-" 
                                        + toString(evictionStrategy),
                                    .comment = "",
                                    //.name = toString(cacheShape) 
                                    //    + "::" + toString(chunkSize) + "::" + toString(cacheLimit) + "::" + toString(evictionStrategy),
                                    .fileSpace = fileSpace,
                                    .testSpace = createDataSpace(testSpace, layout, chunkSize),
                                    .accessPattern = accessPattern,
                                    .accessAmount = accessAmount,
                                    .repetitions = repetitions,
                                    .datasetCount = toValue(accessPattern),
                                    .cacheShape = cacheShape,
                                    .chunkSize = toValue(chunkSize),
                                    .cacheLimit = toValue(cacheLimit, testSpace, accessAmount),
                                    .evictionStrategy = evictionStrategy,
                                    .bufferedRead = readType == ReadType::BUFFERED_READ,
                                };

                                scenarios.push_back(s);

                                if (accessPattern == AccessPattern::FULLY_RANDOM || accessPattern == AccessPattern::FULLY_RANDOM_LIMITED || accessPattern == AccessPattern::RANDOM_PATTERN || accessPattern == AccessPattern::BEYOND_DATASETS)
                                {
                                    s.accessAmount = s.accessAmount * 3;
                                    s.comment = "-more";
                                    scenarios.push_back(s);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return scenarios;
}

void scrambleScenarios(std::vector<Scenario> &scenarios)
{
    std::random_shuffle(scenarios.begin(), scenarios.end());
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
    std::cout << "Profiling scenario " << scenario.name << scenario.comment << "..." << std::endl;
    auto start = steady_clock::now();

    setenv("STAGING_CACHE_SHAPE", toString(scenario.cacheShape).c_str(), 1);
    setenv("STAGING_CHUNK_SIZE", std::to_string(scenario.chunkSize).c_str(), 1);
    setenv("STAGING_CACHE_LIMIT", std::to_string(scenario.cacheLimit).c_str(), 1);
    setenv("STAGING_EVICTION_STRATEGY", toString(scenario.evictionStrategy).c_str(), 1);
    setenv("STAGING_BUFFERED_READ", std::to_string(scenario.bufferedRead).c_str(), 1);

    H5::H5File file = useTestFile(2, scenario.fileSpace.size, scenario.datasetCount);
    
    auto middle0 = steady_clock::now();
    std::cout << "File created in " << duration_cast<milliseconds>(middle0 - start).count() << " ms" << std::endl;
        
    H5::DataSet dataset = file.openDataSet("testData_0");
    H5::DataType dataType = dataset.getDataType();

    std::vector<ProfiledReadAccess> spaces = createAccesses(scenario, dataset);
    dataset.close();

    ProfileResult profileResult;
    for (int repetition = 0; repetition < scenario.repetitions; repetition++)
    {
        hsize_t memorySpaceSize[] = {scenario.testSpace.size[0] + scenario.testSpace.offset[0], scenario.testSpace.size[1] + scenario.testSpace.offset[1]};
        size_t size = (memorySpaceSize[0] * memorySpaceSize[1]);
        uint64_t* buffer = new uint64_t[size];
        std::memset(buffer, 0, size * sizeof(uint64_t));

        std::vector<H5::DataSet> datasets;
        for (size_t i = 0; i < scenario.datasetCount; i++)
        {
            datasets.push_back(file.openDataSet("testData_" + std::to_string(i)));
        }       

        profiledRead((uint8_t*)buffer, datasets, dataType, spaces, &profileResult);
        
        for (auto & dataset : datasets)
        {
            dataset.close();
        }

        if (!isSilent)
        {
            printTable(profileResult.durations.back(), scenario.testSpace.size);
            std::cout << std::endl;
            printList(profileResult.durations.back(), std::min((int)profileResult.durations.back().size(), 12));
            std::cout << std::endl;
            printGraph(profileResult.durations.back(), *std::min_element(profileResult.durations.back().begin(), profileResult.durations.back().end()), *std::max_element(profileResult.durations.back().begin(), profileResult.durations.back().end()));
            std::cout << std::endl;
            printAsDat(" ", profileResult.durations.back());

            hsize_t printOffset[] = {0, 0};
            hsize_t printSize[] = {8, 8};
            //printBuffer(buffer, 2, scenario.testSpace.size, printOffset, printSize);
        }

        std::cout << repetition << " ";
        
        if (!std::all_of(profileResult.validationResults.back().begin(), profileResult.validationResults.back().end(), [](bool b) { return b; }))
        {
            std::cout << "FAIL -------------";
        }
        else
        {
            std::cout << "PASS";
        }
        
        int totalDuration = std::accumulate(profileResult.durations.back().begin(), profileResult.durations.back().end(), 0);
        std::cout << " (in " << totalDuration/1000 << " ms)" << std::endl;

        delete[] buffer;   
    }

    std::string environment = ENVIRONMENT;

    char* basePathString = std::getenv("HOME");
    std::filesystem::path basePath(basePathString);
    std::filesystem::path path = basePath / "results";
    saveToFileAsDat(path.string(), profileResult.durations, scenario.name + scenario.comment);

    auto end = steady_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);

    std::cout << "Total test time: " << duration.count() << " ms" << std::endl;
}

void profiledRead(uint8_t buffer[], std::vector<H5::DataSet>& datasets, H5::DataType dataType, std::vector<ProfiledReadAccess> &spaces, ProfileResult* profileResult)
{        
    steady_clock::time_point start = steady_clock::now();
    steady_clock::time_point end = steady_clock::now();

    microseconds minimum(999999999999999);
    microseconds maximum(0);
    microseconds sum(0);

    int minimumIndex = 0;
    int maximumIndex = 0;

    profileResult->durations.emplace_back(std::vector<int>());
    profileResult->validationResults.emplace_back(std::vector<bool>());

    profileResult->durations.back().reserve(spaces.size());
    profileResult->validationResults.back().reserve(spaces.size());

    for (ProfiledReadAccess access : spaces)
    {        
        hsize_t sourceDimensions[2];
        access.dataSpace.getSimpleExtentDims(nullptr, sourceDimensions);
        hsize_t sourceOffset[2];
        hsize_t sourceOpposite[2];
        access.dataSpace.getSelectBounds(sourceOffset, sourceOpposite);

        hsize_t targetDimensions[2];
        access.memorySpace.getSimpleExtentDims(nullptr, targetDimensions);
        hsize_t targetOffset[2];
        hsize_t targetOpposite[2];
        access.memorySpace.getSelectBounds(targetOffset, targetOpposite);
        hsize_t targetSize[2];
        targetSize[0] = targetOpposite[0] - targetOffset[0] + 1;
        targetSize[1] = targetOpposite[1] - targetOffset[1] + 1;

        start = steady_clock::now();
        datasets[access.datasetIndex].read(buffer, dataType, access.memorySpace, access.dataSpace);
        end = steady_clock::now();

        microseconds duration = duration_cast<microseconds>(end - start);
        profileResult->durations.back().push_back(duration.count());           
        profileResult->validationResults.back().push_back(verifyBuffer((uint64_t*)buffer, access.datasetIndex, 2, sourceDimensions, sourceOffset, targetDimensions, targetOffset, targetSize));
    }
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

void saveToFileAsDat(std::string filePath, std::vector<std::vector<int>> matrix, std::string name)
{
    std::filesystem::path directoryPath = std::filesystem::path(filePath) / name / ENVIRONMENT;
    std::filesystem::create_directories(directoryPath);
    std::string fullPathString = (directoryPath / HDF5_VARIANT).string();

    int width = matrix.size();
    int height = matrix[0].size();

    std::ofstream file;
    file.open(fullPathString + ".dat");

    for (int i = 0; i < height; ++i)
    {
        file << i << " ";
        for (int  j = 0; j < width; j++)
        {
            /* code */
            file << matrix.at(j).at(i) << " ";
        }        
        file << std::endl;
    }
    file.close();
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

std::vector<ProfiledReadAccess> createAccesses(Scenario scenario, H5::DataSet &dataset)
{
    hsize_t memorySpaceSize[] = {scenario.testSpace.size[0] + scenario.testSpace.offset[0], scenario.testSpace.size[1] + scenario.testSpace.offset[1]};

    std::vector<ProfiledReadAccess> spaces;
    spaces.reserve(scenario.accessAmount);
    if (scenario.accessPattern == AccessPattern::ALWAYS_THE_SAME)
    {
        H5::DataSpace dataSpace = dataset.getSpace();
        dataSpace.selectHyperslab(H5S_SELECT_SET, scenario.testSpace.size, scenario.testSpace.offset);

        H5::DataSpace memorySpace(2, memorySpaceSize);
        memorySpace.selectHyperslab(H5S_SELECT_SET, scenario.testSpace.size, scenario.testSpace.offset);
        
        ProfiledReadAccess access = {
            .memorySpace = memorySpace,
            .dataSpace = dataSpace,
            .datasetIndex = 0,
        };

        for (int i = 0; i < scenario.accessAmount; i++) {
            spaces.push_back(access);
        }
    }
    else if (scenario.accessPattern == AccessPattern::FULLY_RANDOM)
    {
        resetPseudodRandomGenerator();
        for (int i = 0; i < scenario.accessAmount; i++)
        {
            hsize_t randomOffset[2] = {getPseudoRandomNumber() % (scenario.fileSpace.size[0] - scenario.testSpace.size[0]), getPseudoRandomNumber() % (scenario.fileSpace.size[1] - scenario.testSpace.size[1])};
            hsize_t noOffset[2] = {0, 0};

            H5::DataSpace dataSpace = dataset.getSpace();
            dataSpace.selectHyperslab(H5S_SELECT_SET, scenario.testSpace.size, randomOffset);

            H5::DataSpace memorySpace(2, memorySpaceSize);
            memorySpace.selectHyperslab(H5S_SELECT_SET, scenario.testSpace.size, noOffset);
            
            ProfiledReadAccess access = {
                .memorySpace = memorySpace,
                .dataSpace = dataSpace,
                .datasetIndex = 0,
            };

            spaces.push_back(access);
        }
    }
    else if (scenario.accessPattern == AccessPattern::FULLY_RANDOM_LIMITED)
    {
        resetPseudodRandomGenerator();
        for (int i = 0; i < scenario.accessAmount; i++)
        {
            hsize_t yRange = (scenario.fileSpace.size[0] - scenario.testSpace.size[0]) / 10;
            hsize_t xRange = (scenario.fileSpace.size[1] - scenario.testSpace.size[1]) / 10;
            hsize_t randomOffset[2] = {getPseudoRandomNumber() % yRange, getPseudoRandomNumber() % xRange};
            hsize_t noOffset[2] = {0, 0};

            H5::DataSpace dataSpace = dataset.getSpace();
            dataSpace.selectHyperslab(H5S_SELECT_SET, scenario.testSpace.size, randomOffset);

            H5::DataSpace memorySpace(2, memorySpaceSize);
            memorySpace.selectHyperslab(H5S_SELECT_SET, scenario.testSpace.size, noOffset);
            
            ProfiledReadAccess access = {
                .memorySpace = memorySpace,
                .dataSpace = dataSpace,
                .datasetIndex = 0,
            };

            spaces.push_back(access);
        }
    }
    else if (scenario.accessPattern == AccessPattern::RANDOM_PATTERN)
    {
        resetPseudodRandomGenerator();
        for (int i = 0; i < scenario.accessAmount; i++)
        {
            hsize_t randomOffset[2] = {getPseudoRandomNumber() % (scenario.fileSpace.size[0] - scenario.testSpace.size[0]), getPseudoRandomNumber() % (scenario.fileSpace.size[1] - scenario.testSpace.size[1])};
            hsize_t noOffset[2] = {0, 0};

            H5::DataSpace dataSpace = dataset.getSpace();
            dataSpace.selectHyperslab(H5S_SELECT_SET, scenario.testSpace.size, randomOffset);

            H5::DataSpace memorySpace(2, memorySpaceSize);
            memorySpace.selectHyperslab(H5S_SELECT_SET, scenario.testSpace.size, noOffset);
            
            ProfiledReadAccess access = {
                .memorySpace = memorySpace,
                .dataSpace = dataSpace,
                .datasetIndex = 0,
            };

            spaces.push_back(access);
        }
    }
    else if (scenario.accessPattern == AccessPattern::BEYOND_DATASETS)
    {
        resetPseudodRandomGenerator();
        int accessCounter = 0;
        int currentDatasetIndex = 0;
        int totalAccessesEachDataset = scenario.accessAmount / scenario.datasetCount;
        for (int i = 0; i < scenario.accessAmount; i++)
        {
            hsize_t yRange = (scenario.fileSpace.size[0] - scenario.testSpace.size[0]) / 10;
            hsize_t xRange = (scenario.fileSpace.size[1] - scenario.testSpace.size[1]) / 10;
            hsize_t randomOffset[2] = {getPseudoRandomNumber() % yRange, getPseudoRandomNumber() % xRange};
            hsize_t noOffset[2] = {0, 0};

            H5::DataSpace dataSpace = dataset.getSpace();
            dataSpace.selectHyperslab(H5S_SELECT_SET, scenario.testSpace.size, randomOffset);

            H5::DataSpace memorySpace(2, memorySpaceSize);
            memorySpace.selectHyperslab(H5S_SELECT_SET, scenario.testSpace.size, noOffset);

            if (accessCounter == totalAccessesEachDataset)
            {
                currentDatasetIndex = (currentDatasetIndex + 1) % scenario.datasetCount;
                accessCounter = 0;
            }
            accessCounter++;
            
            ProfiledReadAccess access = {
                .memorySpace = memorySpace,
                .dataSpace = dataSpace,
                .datasetIndex = currentDatasetIndex,
            };

            spaces.push_back(access);
        }
    }
    else if (scenario.accessPattern == AccessPattern::COHERENT_REGION)
    {
        hsize_t offset[2]  = {scenario.testSpace.offset[0], scenario.testSpace.offset[1]};
        
        for (int i = 0; i < scenario.accessAmount; i++) 
        {
            if (i > 0)
            {
                offset[1] += scenario.testSpace.size[1];
                if (offset[1] + scenario.testSpace.size[1] > scenario.fileSpace.size[1])
                {
                    offset[1] = scenario.testSpace.offset[1];
                    offset[0] += scenario.testSpace.size[0];
                }
            }

            H5::DataSpace dataSpace = dataset.getSpace();
            dataSpace.selectHyperslab(H5S_SELECT_SET, scenario.testSpace.size, offset);

            H5::DataSpace memorySpace(2, memorySpaceSize);
            memorySpace.selectHyperslab(H5S_SELECT_SET, scenario.testSpace.size, scenario.testSpace.offset);

            ProfiledReadAccess access = {
                .memorySpace = memorySpace,
                .dataSpace = dataSpace,
                .datasetIndex = 0,
            };
        
            spaces.push_back(access);
        }
    }
    else if (scenario.accessPattern == AccessPattern::COHERENT_REGION_REPETITIVE)
    {
        hsize_t offset[2]  = {scenario.testSpace.offset[0], scenario.testSpace.offset[1]};
        int divisionSize = 4;
        
        for (int i = 0; i < std::min(scenario.accessAmount, divisionSize); i++) 
        {
            if (i > 0)
            {
                offset[1] += scenario.testSpace.size[1];
                if (offset[1] + scenario.testSpace.size[1] > scenario.fileSpace.size[1])
                {
                    offset[1] = scenario.testSpace.offset[1];
                    offset[0] += scenario.testSpace.size[0];
                }
            }

            H5::DataSpace dataSpace = dataset.getSpace();
            dataSpace.selectHyperslab(H5S_SELECT_SET, scenario.testSpace.size, offset);

            H5::DataSpace memorySpace(2, memorySpaceSize);
            memorySpace.selectHyperslab(H5S_SELECT_SET, scenario.testSpace.size, scenario.testSpace.offset);

            ProfiledReadAccess access = {
                .memorySpace = memorySpace,
                .dataSpace = dataSpace,
                .datasetIndex = 0,
            };
        
            spaces.push_back(access);
        }

        for (int i = divisionSize; i < scenario.accessAmount; i++) 
        {
            spaces.push_back(spaces[i % divisionSize]);
        }        
    }
    else if (scenario.accessPattern == AccessPattern::COHERENT_REGION_UNFAVOURABLE_TRAVERSAL)
    {
        hsize_t offset[2]  = {scenario.testSpace.offset[0], scenario.testSpace.offset[1]};

        for (int i = 0; i < scenario.accessAmount; i++) 
        {
            if (i > 0)
            {
                offset[0] += scenario.testSpace.size[0];
                if (offset[0] + scenario.testSpace.size[0] > scenario.fileSpace.size[0])
                {
                    offset[0] = scenario.testSpace.offset[0];
                    offset[1] += scenario.testSpace.size[1];
                }
            }

            H5::DataSpace dataSpace = dataset.getSpace();
            dataSpace.selectHyperslab(H5S_SELECT_SET, scenario.testSpace.size, offset);

            H5::DataSpace memorySpace(2, memorySpaceSize);
            memorySpace.selectHyperslab(H5S_SELECT_SET, scenario.testSpace.size, scenario.testSpace.offset);

            ProfiledReadAccess access = {
                .memorySpace = memorySpace,
                .dataSpace = dataSpace,
                .datasetIndex = 0,
            };
        
            spaces.push_back(access);
        }
    }

    return spaces;
}

bool isOverlappingHyperslabInVector(std::vector<ProfiledReadAccess> spaces, hsize_t offset[2], hsize_t size[2])
{
    for (ProfiledReadAccess space : spaces)
    {
        hsize_t opposite[2] = {offset[0] + size[0], offset[1] + size[1]};
        if (isOverlappingHyperslab(space.memorySpace, offset, opposite))
        {
            return true;
        }
    }
    return false;
}

bool isOverlappingHyperslab(H5::DataSpace dataSpace1, hsize_t offset2[2], hsize_t opposite2[2])
{
    hsize_t offset1[2];
    hsize_t opposite1[2];
    dataSpace1.getSelectBounds(offset1, opposite1);

    if (offset1[0] > opposite2[0] || offset2[0] > opposite1[0])
    {
        return false;
    }
    if (offset1[1] > opposite2[1] || offset2[1] > opposite1[1])
    {
        return false;
    }
    return true;
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

uint64_t toValue(CacheLimit value, DataSpace dataSpace, int accessAmount)
{
    uint64_t area = dataSpace.size[0] * dataSpace.size[1] * 8;
    uint64_t baseValue = area * accessAmount;

    switch (value)
    {
    case CacheLimit::TOO_LOW_FACTOR_0_25: return baseValue * 25 / 100;
    case CacheLimit::TOO_LOW_FACTOR_0_34: return baseValue * 34 / 100;
    case CacheLimit::ENOUGH_FACTOR_10: return baseValue * 10;
    default: return -1;
    }
}

int toValue(AccessPattern pattern)
{
    switch (pattern)
    {
        case AccessPattern::ALWAYS_THE_SAME: 
        case AccessPattern::FULLY_RANDOM: 
        case AccessPattern::RANDOM_PATTERN: 
            return 1;
        case AccessPattern::BEYOND_DATASETS: 
            return 3;
        case AccessPattern::COHERENT_REGION:
        case AccessPattern::COHERENT_REGION_UNFAVOURABLE_TRAVERSAL:
            return 1;
        default: return 1;
    }
}

DataSpace createDataSpace(const DataSpace &baseDataSpace, const Layout &layout, const CacheChunkSize &chunkSize)
{
    DataSpace newSpace;
    int offset = 0.1 * toValue(chunkSize);
    offset = (offset < 2) ? 2 : offset;
    
    newSpace.size[0] = baseDataSpace.size[0];
    newSpace.size[1] = baseDataSpace.size[1];

    if (layout == Layout::ALIGNED)
    {
        newSpace.offset[0] = baseDataSpace.offset[0];
        newSpace.offset[1] = baseDataSpace.offset[1];
    }
    else if (layout == Layout::VERTICAL_OFFSET)
    {
        newSpace.offset[0] = baseDataSpace.offset[0] + offset;
        newSpace.offset[1] = baseDataSpace.offset[1];
    }
    else if (layout == Layout::HORIZONTAL_OFFSET)
    {
        newSpace.offset[0] = baseDataSpace.offset[0];
        newSpace.offset[1] = baseDataSpace.offset[1] + offset;
    }
    else if (layout == Layout::OFFSET)
    {
        newSpace.offset[0] = baseDataSpace.offset[0] + offset;
        newSpace.offset[1] = baseDataSpace.offset[1] + offset;
    }
    
    return newSpace;
}