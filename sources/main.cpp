#include "H5Cpp.h"
#include <iostream>
#include <cstring>
#include <chrono>
#include <vector>
#include <math.h>
#include <filesystem>

#include "test.h"

using namespace std::chrono;

void profiledRead(uint8_t buffer[], H5::DataSet dataset, H5::DataType dataType, H5::DataSpace memorySpace, H5::DataSpace dataSpace);
void printAsCSV(uint8_t buffer[], size_t size);
std::string fill(std::string string, char filler, int count);
void printList(std::vector<int> &durations, int size);
void printGraph(std::vector<int> &durations, int minimum, int maximum);

int main(void)
{    
    //hsize_t dimensions[] = {16384, 16384};
    //generateHdf5TestFile("2d_16384_16384.hdf5", 2, dimensions);
    std::filesystem::path currentDir = std::filesystem::current_path();
    std::cout << "Current working directory: " << currentDir << std::endl;

    H5::H5File file("../assets/datasets/2d_16384_16384.hdf5", H5F_ACC_RDONLY);    
    H5::DataSet dataset = file.openDataSet("testData");
    H5::DataType dataType = dataset.getDataType();

    hsize_t offset[2] = {0, 0};
    hsize_t count[2] = {4096, 4096};
    H5::DataSpace dataSpace = dataset.getSpace();
    dataSpace.selectHyperslab(H5S_SELECT_SET, count, offset);

    H5::DataSpace memorySpace(2, count);
    memorySpace.selectHyperslab(H5S_SELECT_SET, count, offset);

    std::cout << "Profiling..." << std::endl;
    std::cout << " " << std::endl;

    //uint64_t buffer[count[0]*count[1]];
    uint64_t* buffer = new uint64_t[count[0]*count[1]];
    profiledRead((uint8_t*)buffer, dataset, dataType, memorySpace, dataSpace);
    
    std::cout << " " << std::endl;
    std::cout << "Exiting..." << std::endl;
}

void profiledRead(uint8_t buffer[], H5::DataSet dataset, H5::DataType dataType, H5::DataSpace memorySpace, H5::DataSpace dataSpace)
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
        sum += duration;

        if (duration < minimum)
        {
            minimum = duration;
            minimumIndex = i;
        }
        
        if (duration > maximum)
        {
            maximum = duration;
            maximumIndex = i;
        }        
        
        bool isOkay = verifyBuffer((uint64_t*)buffer, 2, dimensions, offset, size);
        if (!isOkay)
        {
            std::cout << "Buffers do NOT match" << std::endl;
        }
    }

    microseconds average = sum / count;

    size_t bytes = size[0] * size[1] * 8;
    float throughputMinimum = bytes / (minimum.count() / 1e+6) / 1024 / 1024;
    float throughputAverage = bytes / (average.count() / 1e+6) / 1024 / 1024;
    float throughputMaximum = bytes / (maximum.count() / 1e+6) / 1024 / 1024;

    std::cout << "+---------------- dataset.read(...) ----------------+" << std::endl;
    std::cout << "| repetitions:    " << fill(std::to_string(count), ' ', 6) << fill(" ", ' ', 28) << "|" << std::endl;
    std::cout << "| minimum time:   " << fill(std::to_string(minimum.count()), ' ', 6) << " µs @ " << fill(std::to_string(minimumIndex), ' ', 7) << "   " << round(throughputMinimum) << " MiB/s  " << "|" << std::endl;
    std::cout << "| average time:   " << fill(std::to_string(average.count()), ' ', 6) << " µs " << fill(" ", ' ', 9) << "   " << round(throughputAverage) << " MiB/s  " << "|" << std::endl;
    std::cout << "| maximum time:   " << fill(std::to_string(maximum.count()), ' ', 6) << " µs @ " << fill(std::to_string(maximumIndex), ' ', 8) << "   " << round(throughputMaximum) << " MiB/s  " << "|" << std::endl;
    std::cout << "+---------------------------------------------------+" << std::endl;

    std::cout << std::endl;
    printList(durations, 11);
    std::cout << std::endl;
    printGraph(durations, minimum.count(), maximum.count());
}

void printAsCSV(uint8_t buffer[], size_t size)
{
    for (size_t i = 0; i < size; i++)
    {        
        std::cout << std::to_string(buffer[i]) << std::endl;
    }    
}

std::string fill(std::string string, char filler, int count)
{
    return string + std::string(count - string.length(), filler);
}

void printList(std::vector<int> &durations, int size)
{
    for (size_t i = 0; i < size - 1; i++)
    {
        std::cout << durations[i] << " µs, ";
    }
    std::cout << durations[size] << " µs" << std::endl;    
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