#include <iostream>
#include <iomanip>

#include "test.h"

#include <string>
#include <cstdint>
#include <filesystem>

H5::H5File useTestFile(int rank, hsize_t* dimensions)
{
    std::filesystem::path path("../assets/datasets/generated/");
    std::string fileName = createFileName(rank, dimensions);

    std::filesystem::create_directories(std::filesystem::path(path));

    if (!std::filesystem::exists(path / fileName))
    {
        generateHdf5TestFile((path / fileName).string(), rank, dimensions);
    }
    
    return H5::H5File((path / fileName).string(), H5F_ACC_RDONLY);
}

std::string createFileName(int rank, hsize_t* dimenions)
{
    std::ostringstream formatted;
    formatted << rank << "d_"; 
    formatted << dimenions[0];
    formatted << "_";
    formatted << dimenions[1];
    formatted << ".hdf5";
    return formatted.str();
}

void generateHdf5TestFile(std::string path, int rank, hsize_t* dimensions)
{
    std::unique_ptr<uint64_t[]> data = generateTestData(rank, dimensions);

    H5::H5File file(path, H5F_ACC_TRUNC);
    H5::DataSpace dataspace(rank, dimensions);

    H5::IntType datatype(H5::PredType::NATIVE_UINT64);
    datatype.setOrder(H5T_ORDER_LE);

    H5::DataSet dataset = file.createDataSet("testData", datatype, dataspace);
    dataset.write(data.get(), H5::PredType::NATIVE_UINT64);
}


std::unique_ptr<uint64_t[]> generateTestData(int rank, hsize_t* dimensions)
{   
    auto data = std::make_unique<uint64_t[]>(dimensions[0] * dimensions[1]);

    int counter = 0;
    for (int y = 0; y < dimensions[1]; y++)
    {
        for (int x = 0; x < dimensions[0]; x++)
        {
            volatile size_t index = x + y*dimensions[0];
            data[index] = ((uint64_t)counter << 32) | (x << 16) | (y << 0);
            counter++;
        }
    }

    return std::move(data);
}

bool verifyBuffer(uint64_t* buffer, size_t rank, hsize_t *sourceDimensions, hsize_t *sourceOffset, hsize_t *targetDimensions, hsize_t *targetOffset, hsize_t *targetSize)
{    
    int counter = 0;
    for (hsize_t y = 0; y < sourceDimensions[1]; y++)
    {
        for (hsize_t x = 0; x < sourceDimensions[0]; x++)
        {
            if (x >= sourceOffset[0] && x < (sourceOffset[0] + targetSize[0])
            &&  y >= sourceOffset[1] && y < (sourceOffset[1] + targetSize[1]))
            {
                hsize_t targetCoordinates[] = {targetOffset[0] + x - sourceOffset[0], targetOffset[1] + y - sourceOffset[1]};
                volatile size_t index = getLinearIndex(targetCoordinates, targetDimensions, rank);
                volatile uint64_t expected = ((uint64_t)counter << 32) | (x << 16) | (y << 0);
                uint64_t found = buffer[index];

                if (found != expected)
                {
                    std::cout << "[" << x << "|" << y << "]" << std::endl;
                    return false;
                }                
            }

            counter++;
        }
    }

    return true;
}

void printBuffer(uint64_t* buffer, size_t rank, hsize_t* dimensions, hsize_t* offset, hsize_t* size)
{
    int counter = 0;
    for (int y = 0; y < dimensions[1]; y++)
    {
        for (int x = 0; x < dimensions[0]; x++)
        {
            if (x >= offset[0] && x < (offset[0] + size[0])
            &&  y >= offset[1] && y < (offset[1] + size[1]))
            {
                volatile size_t index = x + y*dimensions[0];
                volatile uint64_t expectedValue = ((uint64_t)counter << 32) | (x << 16) | (y << 0);    

                uint64_t isValue = buffer[index];
                int isX = getX(isValue);
                int isY = getY(isValue);
                int isCounter = getCounter(isValue);

                std::cout << "(" << std::setw(3) << std::setfill('0') << isX << " | " << std::setw(3) << std::setfill('0') << isY << "): " << std::setw(5) << std::setfill('0') << isCounter << "; ";
            }

            counter++;
        }        

        if (y >= offset[1] && y < (offset[1] + size[1]))
        {
            std::cout << std::endl;
        }
    }
}

int getCounter(uint64_t value)
{
    return (value >> 32 ) & 0xFFFF;
}

int getX(uint64_t value)
{
    return (value >> 16 ) & 0xFFFF;
}

int getY(uint64_t value)
{
    return (value >> 0 ) & 0xFFFF;
}

uint64_t getLinearAddress(hsize_t* coordinates, hsize_t* arrayDimensions, hsize_t rank, uint8_t typeSize)
{
    return getLinearIndex(coordinates, arrayDimensions, rank) * typeSize;
}

hsize_t getLinearIndex(hsize_t* coordinates, hsize_t* arrayDimensions, hsize_t rank)
{
    hsize_t address = 0;
    hsize_t multiplier = 1;
    for (uint i = 0; i < rank; ++i)
    {
        address += coordinates[i] * multiplier;
        multiplier *= arrayDimensions[i];
    }

    return address;
}