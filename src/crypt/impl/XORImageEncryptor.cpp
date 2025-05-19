#include <algorithm>

#include "XORAlgorithm.h"
#include <stdexcept>
#include <numeric>
#include <random>
#include <array>

static uint64_t seedFromKey(const std::string& key) {
    return std::hash<std::string>{}(key);
}

void XORImageEncryptor::encrypt(const Image& input, Image& output, const std::string& key) {
    if (input.pixels.empty())
        throw std::runtime_error("Input image is empty.");
    if (input.channels != 3)
        throw std::runtime_error("Only 3-channel images supported.");

    int total = input.width * input.height;
    // Extract pixels as RGB triplets
    std::vector<std::array<unsigned char,3>> pixels(total);
    for (int i = 0; i < total; ++i) {
        pixels[i] = { input.pixels[3*i], input.pixels[3*i+1], input.pixels[3*i+2] };
    }
    // Shuffle indices
    std::vector<int> order(total);
    std::iota(order.begin(), order.end(), 0);
    std::mt19937 gen(seedFromKey(key));
    std::shuffle(order.begin(), order.end(), gen);

    // Build output pixel data
    output.width = input.width;
    output.height = input.height;
    output.channels = 3;
    output.pixels.resize(total * 3);
    for (int i = 0; i < total; ++i) {
        const auto &src = pixels[order[i]];
        output.pixels[3*i]   = src[0];
        output.pixels[3*i+1] = src[1];
        output.pixels[3*i+2] = src[2];
    }
}

void XORImageEncryptor::decrypt(const Image& input, Image& output, const std::string& key) {
    if (input.pixels.empty())
        throw std::runtime_error("Input image is empty.");
    if (input.channels != 3)
        throw std::runtime_error("Only 3-channel images supported.");

    int total = input.width * input.height;
    std::vector<std::array<unsigned char,3>> pixels(total);
    for (int i = 0; i < total; ++i) {
        pixels[i] = { input.pixels[3*i], input.pixels[3*i+1], input.pixels[3*i+2] };
    }
    std::vector<int> order(total);
    std::iota(order.begin(), order.end(), 0);
    std::mt19937 gen(seedFromKey(key));
    std::ranges::shuffle(order.begin(), order.end(), gen);

    output.width = input.width;
    output.height = input.height;
    output.channels = 3;
    output.pixels.resize(total * 3);
    for (int i = 0; i < total; ++i) {
        const auto &src = pixels[i];
        int dst = order[i];
        output.pixels[3*dst]   = src[0];
        output.pixels[3*dst+1] = src[1];
        output.pixels[3*dst+2] = src[2];
    }
}