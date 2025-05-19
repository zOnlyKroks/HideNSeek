#include "XORAlgorithm.h"
#include <stdexcept>
#include <random>

static uint8_t byteFromKey(const std::string& key, const int pos) {
    return static_cast<uint8_t>(key[pos % key.size()]);
}

void XORImageEncryptor::encrypt(const Image& input, Image& output, const std::string& key) {
    if (input.pixels.empty())
        throw std::runtime_error("Input image is empty.");
    if (input.channels != 3)
        throw std::runtime_error("Only 3-channel images supported.");

    const int totalBytes = input.width * input.height * input.channels;
    output.width = input.width;
    output.height = input.height;
    output.channels = input.channels;
    output.pixels.resize(totalBytes);

    // XOR each byte with key-derived stream
    for (int i = 0; i < totalBytes; ++i) {
        const uint8_t k = byteFromKey(key, i);
        output.pixels[i] = input.pixels[i] ^ k;
    }
}

void XORImageEncryptor::decrypt(const Image& input, Image& output, const std::string& key) {
    // XOR is its own inverse
    encrypt(input, output, key);
}