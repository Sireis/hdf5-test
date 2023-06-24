cmake_minimum_required(VERSION 3.12)
project(hdf5-test LANGUAGES CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../binary)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Find the HDF5 package
# set(CMAKE_PREFIX_PATH "/usr/local/HDF_Group/HDF5/1.15.0")
set(HDF5_ROOT "/usr/local/HDF_Group/HDF5/1.15.0")
# set(HDF5_ROOT "/usr/local/")
find_package(HDF5 COMPONENTS C CXX NO_MODULE REQUIRED shared)
# find_package(HDF5 1.15.0 COMPONENTS CXX)
# find_package(HDF5 1.15.0 COMPONENTS CXX HL NO_MODULE REQUIRED shared)

# Add the include directories for HDF5
if (HDF5_FOUND)
    if (HDF5_INCLUDE_DIR)
        message(STATUS " hdf5 include directory path is: ${HDF5_INCLUDE_DIR}")
        include_directories(${HDF5_INCLUDE_DIR})
    else ()
        message(WARNING " hdf5 include directory path is empty")
        include_directories(${HDF5_ROOT}/include())
    endif ()
else ()
    message ("HDF5 library not found")
endif ()

# Add your project's source files
set(SOURCES
    sources/main.cpp
)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})

if (HDF5_CXX_SHARED_LIBRARY)
    message(STATUS " hdf5 lib is: ${HDF5_CXX_SHARED_LIBRARY}")
    target_link_libraries(${PROJECT_NAME} PRIVATE ${HDF5_CXX_SHARED_LIBRARY})
else ()
    message(WARNING " hdf5 lib directory path is empty")
    target_link_libraries(${PROJECT_NAME} PRIVATE ${HDF5_ROOT}/lib)
endif ()
# Link against the HDF5 libraries