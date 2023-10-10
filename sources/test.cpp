#include <iostream>

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


hid_t useTestFile(int rank, hsize_t* dimensions, hid_t fapl)
{
    std::filesystem::path path("../assets/datasets/generated/");
    std::string fileName = createFileName(rank, dimensions);

    std::filesystem::create_directories(std::filesystem::path(path));

    if (!std::filesystem::exists(path / fileName))
    {
        generateHdf5TestFile((path / fileName).string(), rank, dimensions);
    }
        
    return H5Fopen((path / fileName).c_str(), H5F_ACC_RDONLY, fapl);
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

bool verifyBuffer(uint64_t* buffer, size_t rank, hsize_t *dimensions, hsize_t *offset, hsize_t *size)
{    
    int counter = 0;
    for (int y = 0; y < dimensions[1]; y++)
    {
        for (int x = 0; x < dimensions[0]; x++)
        {
            if (x >= offset[0] && x < (offset[0] + size[0])
            &&  y >= offset[1] && y < (offset[1] + size[1]))
            {
                volatile size_t index = x + y*size[0];
                volatile uint64_t value = ((uint64_t)counter << 32) | (x << 16) | (y << 0);

                if (buffer[index] != value)
                {
                    return false;
                }                
            }

            counter++;
        }
    }

    return true;
}