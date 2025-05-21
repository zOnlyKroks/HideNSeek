cmake_minimum_required(VERSION 3.20)
project(HideNSeek LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------
# Find dependencies
# -----------------------------
find_package(lodepng CONFIG REQUIRED)   # provides target: lodepng or lodepng::lodepng
find_package(OpenSSL REQUIRED)          # provides OpenSSL::SSL, OpenSSL::Crypto
find_package(cxxopts CONFIG REQUIRED)   # provides cxxopts::cxxopts
find_package(ZLIB REQUIRED)

# -----------------------------
# Gather sources
# -----------------------------
file(GLOB_RECURSE SOURCES
        src/*.cpp
        src/*.h
)

# -----------------------------
# Define executable
# -----------------------------
add_executable(HideNSeek ${SOURCES})

# -----------------------------
# Link libraries
# -----------------------------
target_link_libraries(HideNSeek
        PRIVATE
        OpenSSL::SSL        # SSL support
        OpenSSL::Crypto     # Crypto functions
        cxxopts::cxxopts    # CLI options parsing
        lodepng             # PNG read/write + metadata
        ZLIB::ZLIB
)

# If your package provides the target under the namespace form, you can also try:
#    lodepng::lodepng
# instead of just 'lodepng' above.

# -----------------------------
# Installation (optional)
# -----------------------------
install(TARGETS HideNSeek
        RUNTIME DESTINATION bin)
