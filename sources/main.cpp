#include "H5Cpp.h"
#include "hdf5.h"
#include "H5VLcache_ext.h"
#include "H5FDmpio.h"

#include <iostream>
#include <cstring>
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>
#include <math.h>
#include <filesystem>

#include "test.h"

using namespace std::chrono;

enum class Hdf5Api {
    CppApi,
    CApi,
};

struct DataSpace
{
    hsize_t offset[2];
    hsize_t size[2];
};

struct Scenario
{
    std::string name;
    Hdf5Api hdf5Api;
    DataSpace fileSpace;
    DataSpace testSpace;
    int repetitions;
};

std::vector<int> profiledRead(uint8_t buffer[], H5::DataSet dataset, H5::DataType dataType, H5::DataSpace memorySpace, H5::DataSpace dataSpace);
std::vector<int> profiledRead(uint8_t buffer[], hid_t dataset, hid_t dataType, hid_t memorySpace, hid_t dataSpace);
void printAsCSV(uint8_t buffer[], size_t size);
void printAsDat(std::string filePath, std::vector<int> list);
std::string fill(std::string string, char filler, int count);
void printTable(std::vector<int> &durations, hsize_t sizes[]);
void printList(std::vector<int> &durations, int size);
void printGraph(std::vector<int> &durations, int minimum, int maximum);
void runScenario(Scenario scenario);
void runScenarioC(Scenario scenario);
void runScenarios(std::vector<Scenario> scenarios);

int main(void)
{    
    Scenario s1 = {
        .name = "standard",
        .hdf5Api = Hdf5Api::CApi,
        .fileSpace = {
            .offset = {0, 0},
            .size = {16*1024, 16*1024},
        },
        .testSpace = {
            .offset = {0, 0},
            .size = {4*1024, 4*1024},
        },
        .repetitions = 12,
    };

    std::vector<Scenario> scenarios = {s1};

    runScenarios(scenarios);
}

void runScenarios(std::vector<Scenario> scenarios)
{
    for (Scenario s : scenarios)
    {
        if (s.hdf5Api == Hdf5Api::CppApi)
        {
            runScenario(s);
        }
        else if (s.hdf5Api == Hdf5Api::CApi)
        {
            runScenarioC(s);
        }
    }    
}

void runScenario(Scenario scenario)
{
    H5::H5File file = useTestFile(2, scenario.fileSpace.size);
        
    H5::DataSet dataset = file.openDataSet("testData");
    H5::DataType dataType = dataset.getDataType();

    H5::DataSpace dataSpace = dataset.getSpace();
    dataSpace.selectHyperslab(H5S_SELECT_SET, scenario.testSpace.size, scenario.testSpace.offset);

    H5::DataSpace memorySpace(2, scenario.testSpace.size);
    memorySpace.selectHyperslab(H5S_SELECT_SET, scenario.testSpace.size, scenario.testSpace.offset);

    std::cout << "Profiling scenario " << scenario.name << "..." << std::endl;

    uint64_t* buffer = new uint64_t[scenario.testSpace.size[0] * scenario.testSpace.size[1]];
    auto durations = profiledRead((uint8_t*)buffer, dataset, dataType, memorySpace, dataSpace);

    printTable(durations, scenario.testSpace.size);
    std::cout << std::endl;
    printList(durations, std::min((int)durations.size(), 12));
    std::cout << std::endl;
    printGraph(durations, *std::min_element(durations.begin(), durations.end()), *std::max_element(durations.begin(), durations.end()));
    std::cout << std::endl;
    printAsDat(" ", durations);
}

void runScenarioC(Scenario scenario)
{    
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Info info = MPI_INFO_NULL;
    int rank, nproc, provided;
    MPI_Init_thread(NULL, NULL, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_size(comm, &nproc);
    MPI_Comm_rank(comm, &rank);

    hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fapl_mpio(fapl, comm, info);
    bool b = true;
    H5Pset_fapl_cache(fapl, "HDF5_CACHE_RD", &b);

    hid_t file = useTestFile(2, scenario.fileSpace.size, fapl);
        
    hid_t dataset = H5Dopen(file, "testData", fapl);
    hid_t dataType = H5Dget_type(dataset);

    hid_t dataSpace = H5Dget_space(dataset);
    H5Sselect_hyperslab(dataSpace, H5S_SELECT_SET, scenario.testSpace.offset, NULL, scenario.testSpace.size, NULL);

    hid_t memorySpace = H5Screate_simple(2, scenario.testSpace.size, NULL);
    H5Sselect_hyperslab(memorySpace, H5S_SELECT_SET, scenario.testSpace.offset, NULL, scenario.testSpace.size, NULL);    

    std::cout << "Profiling scenario " << scenario.name << "..." << std::endl;

    uint64_t* buffer = new uint64_t[scenario.testSpace.size[0] * scenario.testSpace.size[1]];
    auto durations = profiledRead((uint8_t*)buffer, dataset, dataType, memorySpace, dataSpace);

    printTable(durations, scenario.testSpace.size);
    std::cout << std::endl;
    printList(durations, std::min((int)durations.size(), 12));
    std::cout << std::endl;
    printGraph(durations, *std::min_element(durations.begin(), durations.end()), *std::max_element(durations.begin(), durations.end()));
    std::cout << std::endl;
    printAsDat(" ", durations);
}

std::vector<int> profiledRead(uint8_t buffer[], H5::DataSet dataset, H5::DataType dataType, H5::DataSpace memorySpace, H5::DataSpace dataSpace)
{    
    const int count = 12;
    
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

    std::vector<int> durations;
    durations.reserve(count);

    for (size_t i = 0; i < count; i++)
    {
        start = steady_clock::now();
        dataset.read(buffer, dataType, memorySpace, dataSpace);
        end = steady_clock::now();

        microseconds duration = duration_cast<microseconds>(end - start);
        durations.push_back(duration.count());   
        
        bool isOkay = verifyBuffer((uint64_t*)buffer, 2, dimensions, offset, size);
        if (!isOkay)
        {
            std::cout << "Buffers do NOT match" << std::endl;
        }
    }

    return durations;
}

std::vector<int> profiledRead(uint8_t buffer[], hid_t dataset, hid_t dataType, hid_t memorySpace, hid_t dataSpace)
{    
    const int count = 12;

    hsize_t dimensions[2];
    H5Sget_simple_extent_dims(dataSpace, dimensions, NULL);

    hsize_t offset[2];
    hsize_t dummy[2];
    H5Sget_select_bounds(memorySpace, offset, dummy);

    hsize_t size[2];
    H5Sget_simple_extent_dims(memorySpace, size, NULL);

    steady_clock::time_point start = steady_clock::now();
    steady_clock::time_point end = steady_clock::now();

    microseconds minimum(999999999999999);
    microseconds maximum(0);
    microseconds sum(0);

    int minimumIndex = 0;
    int maximumIndex = 0;

    std::vector<int> durations;
    durations.reserve(count);

    for (size_t i = 0; i < count; i++)
    {
        start = steady_clock::now();
        H5Dread(dataset, dataType, memorySpace, dataSpace, H5P_DEFAULT, buffer);
        end = steady_clock::now();

        microseconds duration = duration_cast<microseconds>(end - start);
        durations.push_back(duration.count());   
        
        bool isOkay = verifyBuffer((uint64_t*)buffer, 2, dimensions, offset, size);
        if (!isOkay)
        {
            std::cout << "Buffers do NOT match" << std::endl;
        }
    }

    return durations;
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
    return string + std::string(count - string.length(), filler);
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
    for (size_t i = 0; i < size - 2; i++)
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
