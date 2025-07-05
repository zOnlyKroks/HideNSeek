cmake_minimum_required(VERSION 3.20)
project(HideNSeek LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(OpenSSL REQUIRED)
find_package(cxxopts CONFIG REQUIRED)
find_package(lodepng CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

# Only find OpenGL on non-Apple platforms
if(NOT APPLE)
    find_package(OpenGL REQUIRED)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE SOURCES
        src/*.cpp
        src/*.h
        src/*.ui
)

add_executable(HideNSeek ${SOURCES})

# Base libraries for all platforms
target_link_libraries(HideNSeek
        PRIVATE
        Qt6::Widgets
        OpenSSL::SSL
        OpenSSL::Crypto
        cxxopts::cxxopts
        lodepng
        ZLIB::ZLIB
)

# Platform-specific linking
if(APPLE)
    # Use native macOS frameworks instead of OpenGL::GL
    find_library(COCOA_LIBRARY Cocoa)
    find_library(OpenGL_LIBRARY OpenGL)
    target_link_libraries(HideNSeek PRIVATE ${OpenGL_LIBRARY} ${COCOA_LIBRARY})
else()
    # Use OpenGL::GL on other platforms
    target_link_libraries(HideNSeek PRIVATE OpenGL::GL)
endif()

install(TARGETS HideNSeek RUNTIME DESTINATION bin)