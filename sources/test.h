#pragma once

#include "H5Cpp.h"
#include <memory>

H5::H5File useTestFile(int rank, hsize_t* dimensions);
hid_t useTestFile(int rank, hsize_t* dimensions, hid_t fapl);
std::string createFileName(int rank, hsize_t* dimenions);
void generateHdf5TestFile(std::string path, int rank, hsize_t* dimensions);
std::unique_ptr<uint64_t[]> generateTestData(int rank, hsize_t* dimensions);
bool verifyBuffer(uint64_t* buffer, size_t rank, hsize_t *dimensions, hsize_t *offset, hsize_t *size);