#pragma once

#include "H5Cpp.h"
#include <memory>

H5::H5File useTestFile(int rank, hsize_t* dimensions);
std::string createFileName(int rank, hsize_t* dimenions);
void generateHdf5TestFile(std::string path, int rank, hsize_t* dimensions);
std::unique_ptr<uint64_t[]> generateTestData(int rank, hsize_t* dimensions);
bool verifyBuffer(uint64_t* buffer, size_t rank, hsize_t *sourceDimensions, hsize_t *sourceOffset, hsize_t *targetDimensions, hsize_t *targetOffset, hsize_t *targetSize);
void printBuffer(uint64_t* buffer, size_t rank, hsize_t* dimensions, hsize_t* offset, hsize_t* size);
int getCounter(uint64_t value);
int getX(uint64_t value);
int getY(uint64_t value);
uint64_t getLinearAddress(hsize_t* coordinates, hsize_t* arrayDimensions, hsize_t rank, uint8_t typeSize);
hsize_t getLinearIndex(hsize_t* coordinates, hsize_t* arrayDimensions, hsize_t rank);