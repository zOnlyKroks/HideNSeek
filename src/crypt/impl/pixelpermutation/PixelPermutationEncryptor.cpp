#include "PixelPermutationEncryptor.h"
#include <random>
#include <algorithm>
#include <stdexcept>

void PixelPermutationEncryptor::encrypt(const Image& input, Image& output, const std::string& key) {
    int width = input.width;
    int height = input.height;
    int channels = input.channels;
    int totalPixels = width * height;

    // Generate permutation
    std::vector<int> perm(totalPixels);
    for (int i = 0; i < totalPixels; ++i) perm[i] = i;

    size_t seed = std::hash<std::string>{}(key);
    std::mt19937 rng(static_cast<uint32_t>(seed));
    std::shuffle(perm.begin(), perm.end(), rng);

    output = Image(width, height, channels);

    // Write pixels permuted
    for (int i = 0; i < totalPixels; ++i) {
        int srcIndex = i;
        int dstIndex = perm[i];

        int srcX = srcIndex % width;
        int srcY = srcIndex / width;
        int dstX = dstIndex % width;
        int dstY = dstIndex / width;

        for (int c = 0; c < channels; ++c) {
            output.getPixel(dstX, dstY, c) = input.getPixel(srcX, srcY, c);
        }
    }
}

void PixelPermutationEncryptor::decrypt(const Image& input, Image& output, const std::string& key) {
    int width = input.width;
    int height = input.height;
    int channels = input.channels;
    int totalPixels = width * height;

    // Generate permutation
    std::vector<int> perm(totalPixels);
    for (int i = 0; i < totalPixels; ++i) perm[i] = i;

    size_t seed = std::hash<std::string>{}(key);
    std::mt19937 rng(static_cast<uint32_t>(seed));
    std::shuffle(perm.begin(), perm.end(), rng);

    // Invert permutation
    std::vector<int> inversePerm(totalPixels);
    for (int i = 0; i < totalPixels; ++i) {
        inversePerm[perm[i]] = i;
    }

    output = Image(width, height, channels);

    // Corrected loop: Iterate over each pixel in the encrypted image
    for (int k = 0; k < totalPixels; ++k) {
        int dstIndex = inversePerm[k];  // Original position of the pixel at k
        int srcX = k % width;
        int srcY = k / width;
        int dstX = dstIndex % width;
        int dstY = dstIndex / width;

        for (int c = 0; c < channels; ++c) {
            output.getPixel(dstX, dstY, c) = input.getPixel(srcX, srcY, c);
        }
    }
}
