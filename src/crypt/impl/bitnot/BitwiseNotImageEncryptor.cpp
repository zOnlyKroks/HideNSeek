#include "BitwiseNotImageEncryptor.h"

void BitwiseNotImageEncryptor::encrypt(const Image& input, Image& output, const std::string& key) {
    output = Image(input.width, input.height, input.channels);
    for (int y = 0; y < input.height; ++y) {
        for (int x = 0; x < input.width; ++x) {
            for (int c = 0; c < input.channels; ++c) {
                output.getPixel(x, y, c) = ~input.getPixel(x, y, c);
            }
        }
    }
}

void BitwiseNotImageEncryptor::decrypt(const Image& input, Image& output, const std::string& key) {
    // Bitwise NOT is its own inverse
    encrypt(input, output, key);
}
