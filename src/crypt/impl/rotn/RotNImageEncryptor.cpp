#include "RotNImageEncryptor.h"
#include <stdexcept>

namespace {
    uint8_t rotateLeft(uint8_t byte, unsigned int n) {
        return (byte << n) | (byte >> (8 - n));
    }

    uint8_t rotateRight(uint8_t byte, unsigned int n) {
        return (byte >> n) | (byte << (8 - n));
    }

    unsigned int parseRotationAmount(const std::string& key) {
        size_t hashed = std::hash<std::string>{}(key);
        std::string hashStr = std::to_string(hashed);

        for (char c : hashStr) {
            if (c >= '1' && c <= '7') {
                return static_cast<unsigned int>(c - '0');
            }
        }
        throw std::invalid_argument("Invalid ROTN key. Use an integer between 1 and 7.");
    }
}

void RotNImageEncryptor::encrypt(const Image& in, Image& out, const std::string& key) {
    unsigned int n = parseRotationAmount(key);
    out = Image(in.width, in.height, in.channels);
    for (int y = 0; y < in.height; ++y) {
        for (int x = 0; x < in.width; ++x) {
            for (int c = 0; c < in.channels; ++c) {
                out.getPixel(x, y, c) = rotateLeft(in.getPixel(x, y, c), n);
            }
        }
    }
}

void RotNImageEncryptor::decrypt(const Image& in, Image& out, const std::string& key) {
    unsigned int n = parseRotationAmount(key);
    out = Image(in.width, in.height, in.channels);
    for (int y = 0; y < in.height; ++y) {
        for (int x = 0; x < in.width; ++x) {
            for (int c = 0; c < in.channels; ++c) {
                out.getPixel(x, y, c) = rotateRight(in.getPixel(x, y, c), n);
            }
        }
    }
}
