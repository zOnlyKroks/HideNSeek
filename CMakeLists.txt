cmake_minimum_required(VERSION 3.20)
project(HideNSeek LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)             # cryptographic functions
find_package(cxxopts CONFIG REQUIRED)      # CLI parsing

# Collect sources
file(GLOB_RECURSE SOURCES
        src/*.cpp
        src/*.h
)

# Build executable
add_executable(HideNSeek ${SOURCES}
        src/img/Image.h)

# Link libraries
target_link_libraries(HideNSeek PRIVATE
        OpenSSL::SSL     # SSL support
        OpenSSL::Crypto  # Crypto functions
        cxxopts::cxxopts # CLI options parsing
)