#include "H5Cpp.h"
#include <iostream>
#include <cstring>
#include <chrono>
#include <vector>
#include <math.h>
#include <filesystem>

#include "test.h"

using namespace std::chrono;

uint8_t expected[] = {0, 0, 237, 172, 1, 0, 104, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 255, 255, 0, 0, 3, 0, 0, 0, 99, 2, 0, 0, 112, 3, 156, 126, 211, 69, 96, 0, 8, 0, 7, 1, 64, 0, 0, 0, 160, 36, 4, 85, 216, 64, 0, 0, 21, 0, 0, 0, 247, 255, 255, 255, 78, 111, 118, 32, 50, 57, 32, 50, 48, 49, 56, 0, 49, 53, 58, 49, 54, 58, 52, 55, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 237, 172, 15, 0, 60, 0, 145, 231, 0, 0, 0, 0, 0, 0, 29, 20, 33, 1, 0, 0, 0, 0, 0, 0, 0, 202, 0, 80, 0, 0, 104, 218, 1, 0, 28, 0, 0, 0, 80, 0, 58, 1, 254, 255, 255, 0, 0, 0, 254, 255, 255, 0, 0, 1, 254, 255, 255, 0, 0, 1, 32, 35, 0, 0, 0, 0, 2, 0, 237, 172, 15, 0, 60, 0, 189, 32, 1, 0, 0, 0, 0, 0, 117, 20, 189, 0, 0, 0, 0, 0, 0, 0, 0, 201, 248, 8, 0, 249, 104, 183, 0, 0, 28, 0, 0, 0, 8, 66, 0, 0, 51, 51, 0, 1, 0, 2, 254, 255, 255, 0, 0, 1, 254, 255, 255, 0, 0, 2, 64, 35, 0, 0, 0, 0, 3, 0, 237, 172, 15, 0, 60, 0, 134, 189, 1, 0, 0, 0, 0, 0, 13, 20, 54, 1, 0, 0, 0, 0, 0, 0, 0, 182, 50, 128, 0, 66, 111, 42, 1, 0, 28, 0, 0, 0, 128, 0, 0, 0, 255, 255, 255, 255, 255, 255, 254, 255, 255, 0, 0, 3, 254, 255, 255, 0, 0, 3, 240, 71, 0, 0, 0, 0, 4, 0, 237, 172, 15, 0, 60, 0, 9, 200, 1, 0, 0, 0, 0, 0, 157, 20, 233, 0, 0, 0, 0, 0, 0, 0, 0, 192, 219, 128, 0, 158, 108, 191, 0, 0, 28, 0, 0, 0, 128, 0, 0, 0, 255, 255, 255, 255, 255, 255, 254, 255, 255, 0, 0, 4, 254, 255, 255, 0, 0, 4, 192, 255, 0, 0, 0, 0, 5, 0, 237, 172, 15, 0, 60, 0, 255, 212, 1, 0, 0, 0, 0, 0, 133, 20, 63, 1, 0, 0, 0, 0, 0, 0, 0, 174, 253, 128, 0, 109, 115, 122, 1, 0, 28, 0, 0, 0, 128, 0, 0, 0, 255, 255, 255, 255, 255, 255, 254, 255, 255, 0, 0, 5, 254, 255, 255, 0, 0, 5, 16, 41, 0, 0, 0, 0, 6, 0, 237, 172, 15, 0, 60, 0, 243, 231, 1, 0, 0, 0, 0, 0, 45, 20, 33, 1, 0, 0, 0, 0, 0, 0, 0, 202, 255, 80, 0, 150, 104, 243, 1, 0, 28, 0, 0, 0, 80, 0, 58, 1, 254, 255, 255, 0, 0, 0, 254, 255, 255, 0, 0, 1, 254, 255, 255, 0, 0, 1, 80, 35, 0, 0, 0, 0, 7, 0, 237, 172, 15, 0, 60, 0, 10, 243, 1, 0, 0, 0, 0, 0, 93, 20, 33, 1, 0, 0, 0, 0, 0, 0, 0, 202, 237, 80, 0, 233, 104, 249, 1, 0, 28, 0, 0, 0, 80, 8, 58, 1, 254, 255, 255, 0, 0, 0, 254, 255, 255, 0, 0, 1, 254, 255, 255, 0, 0, 1, 80, 35, 0, 0, 0, 0, 8, 0, 237, 172, 15, 0, 60, 0, 164, 166, 2, 0, 0, 0, 0, 0, 149, 20, 39, 1, 0, 0, 0, 0, 0, 0, 0, 201, 252, 128, 0, 59, 104, 79, 0, 0, 28, 0, 0, 0, 128, 0, 0, 0, 255, 255, 255, 255, 255, 255, 254, 255, 255, 0, 0, 1, 254, 255, 255, 0, 0, 6, 68, 35, 0, 0, 0, 0, 9, 0, 237, 172, 15, 0, 60, 0, 132, 149, 0, 0, 0, 0, 0, 0, 37, 20, 233, 0, 0, 0, 0, 0, 0, 0, 0, 200, 220, 128, 1, 57, 103, 63, 1, 0, 28, 0, 0, 0, 128, 0, 0, 0, 255, 255, 255, 255, 255, 255, 254, 255, 255, 0, 0, 4, 254, 255, 255, 0, 0, 4, 48, 16, 0, 0, 0, 0, 10, 0, 237, 172, 15, 0, 60, 0, 158, 244, 0, 0, 0, 0, 0, 0, 125, 20, 59, 1, 0, 0, 0, 0, 0, 0, 0, 215, 249, 128, 1, 255, 73, 255, 1, 0, 28, 0, 0, 0, 128, 0, 0, 0, 255, 255, 255, 255, 255, 255, 254, 255, 255, 0, 0, 7, 254, 255, 255, 0, 0, 7, 80, 23, 0, 0, 0, 0, 11, 0, 237, 172, 15, 0, 60, 0, 175, 124, 1, 0, 0, 0, 0, 0, 37, 20, 39, 1, 0, 0, 0, 0, 0, 0, 0, 207, 87, 128, 1, 100, 101, 241, 1, 0, 28, 0, 0, 0, 128, 0, 0, 0, 255, 255, 255, 255, 255, 255, 254, 255, 255, 0, 0, 1, 254, 255, 255, 0, 0, 1, 224, 35, 0, 0, 0, 0, 12, 0, 237, 172, 25, 0, 64, 0, 9, 144, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 16, 15, 244, 1, 147, 0, 0, 0, 15, 0, 128, 48, 13, 20, 1, 0, 57, 101, 24, 0, 0, 0, 128, 0, 0, 0, 255, 255, 255, 255, 255, 255, 64, 216, 85, 4, 36, 160, 64, 216, 85, 4, 36, 160, 0, 0, 13, 0, 237, 172, 15, 0, 60, 0, 133, 37, 2, 0, 0, 0, 0, 0, 45, 20, 233, 0, 0, 0, 0, 0, 0, 0, 0, 200, 125, 128, 1, 51, 102, 163, 1, 0, 28, 0, 0, 0, 128, 0, 0, 0, 255, 255, 255, 255, 255, 255, 254, 255, 255, 0, 0, 4, 254, 255, 255, 0, 0, 4, 64, 16, 0, 0, 0, 0, 14, 0, 237, 172, 15, 0, 60, 0, 176, 12, 3, 0, 0, 0, 0, 0, 29, 20, 39, 1, 0, 0, 0, 0};

void profiledRead(uint8_t buffer[], H5::DataSet dataset, H5::DataType dataType, H5::DataSpace memorySpace, H5::DataSpace dataSpace);
void printAsCSV(uint8_t buffer[], size_t size);
std::string fill(std::string string, char filler, int count);
void printList(std::vector<int> &durations, int size);
void printGraph(std::vector<int> &durations, int minimum, int maximum);

int main(void)
{    
    //hsize_t dimensions[] = {4096, 4096};
    //generateHdf5TestFile("2d_4096_4096.hdf5", 2, dimensions);
    std::filesystem::path currentDir = std::filesystem::current_path();
    std::cout << "Current working directory: " << currentDir << std::endl;

    H5::H5File file("../assets/datasets/2d_4096_4096.hdf5", H5F_ACC_RDONLY);    
    H5::DataSet dataset = file.openDataSet("testData");
    H5::DataType dataType = dataset.getDataType();

    hsize_t offset[2] = {0, 0};
    hsize_t count[2] = {64, 64};
    H5::DataSpace dataSpace = dataset.getSpace();
    dataSpace.selectHyperslab(H5S_SELECT_SET, count, offset);

    H5::DataSpace memorySpace(2, count);
    memorySpace.selectHyperslab(H5S_SELECT_SET, count, offset);

    std::cout << "Profiling..." << std::endl;
    std::cout << " " << std::endl;

    uint64_t buffer[count[0]*count[1]];
    profiledRead((uint8_t*)buffer, dataset, dataType, memorySpace, dataSpace);
    
    std::cout << " " << std::endl;
    std::cout << "Exiting..." << std::endl;
}

void profiledRead(uint8_t buffer[], H5::DataSet dataset, H5::DataType dataType, H5::DataSpace memorySpace, H5::DataSpace dataSpace)
{    
    const int count = 300;

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
        
        hsize_t dimensions[2];
        dataSpace.getSimpleExtentDims(nullptr, dimensions);
        hsize_t offset[2];
        hsize_t dummy[2];
        memorySpace.getSelectBounds(offset, dummy);
        hsize_t size[2];
        memorySpace.getSimpleExtentDims(size, nullptr);
        int result = verifyBuffer((uint64_t*)buffer, 2, dimensions, offset, size);
        if (result != 0)
        {
            std::cout << "Buffers do NOT match" << std::endl;
        }
    }

    microseconds average = sum / count;

    std::cout << "+---------------- dataset.read(...) ----------------+" << std::endl;
    std::cout << "| repetitions: \t\t" << fill(std::to_string(count), ' ', 6) << fill(" ", ' ', 22) << "|" << std::endl;
    std::cout << "| minimum time: \t" << fill(std::to_string(minimum.count()), ' ', 6) << " µs @ " << fill(std::to_string(minimumIndex), ' ', 16) << "|" << std::endl;
    std::cout << "| average time: \t" << fill(std::to_string(average.count()), ' ', 6) << " µs " << fill(" ", ' ', 18) << "|" << std::endl;
    std::cout << "| maximum time: \t" << fill(std::to_string(maximum.count()), ' ', 6) << " µs @ " << fill(std::to_string(maximumIndex), ' ', 16) << "|" << std::endl;
    std::cout << "+---------------------------------------------------+" << std::endl;

    printList(durations, 11);
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