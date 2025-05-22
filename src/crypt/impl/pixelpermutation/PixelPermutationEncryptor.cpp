#include "PixelPermutationEncryptor.h"
#include <random>
#include <algorithm>
#include <stdexcept>

void PixelPermutationEncryptor::encrypt(const Image& input, Image& output, const std::string& key) {
    const int width = input.width;
    const int height = input.height;
    const int channels = input.channels;
    const int totalPixels = width * height;

    std::vector<int> perm(totalPixels);
    for (int i = 0; i < totalPixels; ++i) perm[i] = i;

    const size_t seed = std::hash<std::string>{}(key);
    std::mt19937 rng(static_cast<uint32_t>(seed));
    std::ranges::shuffle(perm, rng);

    output = Image(width, height, channels);

    for (int i = 0; i < totalPixels; ++i) {
        const int srcIndex = i;
        const int dstIndex = perm[i];

        const int srcX = srcIndex % width;
        const int srcY = srcIndex / width;
        const int dstX = dstIndex % width;
        const int dstY = dstIndex / width;

        for (int c = 0; c < channels; ++c) {
            output.getPixel(dstX, dstY, c) = input.getPixel(srcX, srcY, c);
        }
    }
}

void PixelPermutationEncryptor::decrypt(const Image& input, Image& output, const std::string& key) {
    const int width = input.width;
    const int height = input.height;
    const int channels = input.channels;
    const int totalPixels = width * height;

    std::vector<int> perm(totalPixels);
    for (int i = 0; i < totalPixels; ++i) perm[i] = i;

    const size_t seed = std::hash<std::string>{}(key);
    std::mt19937 rng(static_cast<uint32_t>(seed));
    std::ranges::shuffle(perm, rng);

    std::vector<int> inversePerm(totalPixels);
    for (int i = 0; i < totalPixels; ++i) {
        inversePerm[perm[i]] = i;
    }

    output = Image(width, height, channels);

    for (int k = 0; k < totalPixels; ++k) {
        const int dstIndex = inversePerm[k];
        const int srcX = k % width;
        const int srcY = k / width;
        const int dstX = dstIndex % width;
        const int dstY = dstIndex / width;

        for (int c = 0; c < channels; ++c) {
            output.getPixel(dstX, dstY, c) = input.getPixel(srcX, srcY, c);
        }
    }
}
