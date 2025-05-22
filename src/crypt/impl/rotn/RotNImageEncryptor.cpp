#include "RotNImageEncryptor.h"
#include <stdexcept>

namespace {
    uint8_t rotateLeft(const uint8_t byte, const unsigned int n) {
        return (byte << n) | (byte >> (8 - n));
    }

    uint8_t rotateRight(const uint8_t byte, const unsigned int n) {
        return (byte >> n) | (byte << (8 - n));
    }

    unsigned int parseRotationAmount(const std::string& key) {
        const size_t hashed = std::hash<std::string>{}(key);

        for (const std::string hashStr = std::to_string(hashed); const char c : hashStr) {
            if (c >= '1' && c <= '7') {
                return static_cast<unsigned int>(c - '0');
            }
        }
        throw std::invalid_argument("Invalid ROTN key. Use an integer between 1 and 7.");
    }
}

void RotNImageEncryptor::encrypt(const Image& input, Image& output, const std::string& key) {
    const unsigned int n = parseRotationAmount(key);
    output = Image(input.width, input.height, input.channels);
    for (int y = 0; y < input.height; ++y) {
        for (int x = 0; x < input.width; ++x) {
            for (int c = 0; c < input.channels; ++c) {
                output.getPixel(x, y, c) = rotateLeft(input.getPixel(x, y, c), n);
            }
        }
    }
}

void RotNImageEncryptor::decrypt(const Image& input, Image& output, const std::string& key) {
    const unsigned int n = parseRotationAmount(key);
    output = Image(input.width, input.height, input.channels);
    for (int y = 0; y < input.height; ++y) {
        for (int x = 0; x < input.width; ++x) {
            for (int c = 0; c < input.channels; ++c) {
                output.getPixel(x, y, c) = rotateRight(input.getPixel(x, y, c), n);
            }
        }
    }
}
