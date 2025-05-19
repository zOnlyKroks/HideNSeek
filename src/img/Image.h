#pragma once

#include <vector>

// A simple POD to hold raw RGB pixels in row‐major order.
struct Image {
    int width;
    int height;
    int channels;            // should always be 3
    std::vector<unsigned char> pixels;  // size == width*height*channels
};