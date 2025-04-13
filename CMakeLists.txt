cmake_minimum_required(VERSION 3.24)
project(edgebreaker LANGUAGES CXX)

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set source and include directories
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

# Collect all .cpp files in the src directory
file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")

# Add the executable (use SOURCES to include all .cpp files)
add_executable(${PROJECT_NAME} ${SOURCES})

# Include the headers
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})