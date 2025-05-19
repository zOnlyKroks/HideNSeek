cmake_minimum_required(VERSION 3.20)
project(HideNSeek)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include third-party libraries
include_directories(/opt/homebrew/opt/cxxopts/include)
include_directories(/opt/homebrew/opt/opencv/include/opencv4)

# Find OpenSSL
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

# Recursively find all .cpp and .h files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        src/*.cpp
        src/*.h
)

# Add executable
add_executable(HideNSeek ${SOURCES})

# Link OpenCV
find_package(OpenCV REQUIRED)
target_include_directories(HideNSeek PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(HideNSeek PRIVATE ${OpenCV_LIBS})

# Link OpenSSL
target_link_libraries(HideNSeek PRIVATE OpenSSL::SSL OpenSSL::Crypto)
