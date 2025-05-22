#include "AES256ImageEncryptor.h"
#include <openssl/rand.h>
#include <stdexcept>
#include <iostream>
#include "../../../util/aes/AES256Encryptor.h"

static void hideBytesInImage(const std::vector<unsigned char>& data, const int startPixel, Image& img) {
    const int totalPixels = img.width * img.height * img.channels;
    const int bitsToHide = data.size() * 8;

    if (startPixel + bitsToHide > totalPixels) {
        throw std::runtime_error("Image too small to hide data");
    }

    std::vector<unsigned char>& pixels = img.pixels;

    for (int bitIdx = 0; bitIdx < bitsToHide; ++bitIdx) {
        const int byteIdx = bitIdx / 8;
        const int bitPos = 7 - (bitIdx % 8);
        const unsigned char bit = (data[byteIdx] >> bitPos) & 1;

        const int pixelIdx = startPixel + bitIdx;
        pixels[pixelIdx] = (pixels[pixelIdx] & 0xFE) | bit;
    }
}

static std::vector<unsigned char> extractBytesFromImage(const int startPixel, int byteCount, const Image& img) {
    const int totalPixels = img.width * img.height * img.channels;
    const int bitsToExtract = byteCount * 8;

    if (startPixel + bitsToExtract > totalPixels) {
        throw std::runtime_error("Image too small to extract data");
    }

    std::vector<unsigned char> data(byteCount, 0);
    const std::vector<unsigned char>& pixels = img.pixels;

    for (int bitIdx = 0; bitIdx < bitsToExtract; ++bitIdx) {
        const int byteIdx = bitIdx / 8;
        const int bitPos = 7 - (bitIdx % 8);

        if (pixels[startPixel + bitIdx] & 1) {
            data[byteIdx] |= (1 << bitPos);
        }
    }
    return data;
}

void AES256ImageEncryptor::encrypt(const Image& input, Image& output, const std::string& key) {
    if (input.width <= 0 || input.height <= 0 || input.channels <= 0) {
        throw std::runtime_error("Invalid input image dimensions");
    }

    const int pixelCount = input.width * input.height * input.channels;

    if (static_cast<int>(input.pixels.size()) != pixelCount) {
        throw std::runtime_error("Input pixel data size mismatch");
    }

    if (pixelCount < 256) {
        throw std::runtime_error("Image too small to hide salt and IV");
    }

    std::vector<unsigned char> salt(16);
    std::vector<unsigned char> iv(16);
    if (!RAND_bytes(salt.data(), 16) || !RAND_bytes(iv.data(), 16)) {
        throw std::runtime_error("Failed to generate random salt or IV");
    }

    const AES256Encryptor aes(key, salt);
    std::vector<unsigned char> encrypted = aes.encrypt(input.pixels, iv);

    if (encrypted.size() != input.pixels.size()) {
        throw std::runtime_error("Encryption changed data size");
    }

    output.width = input.width;
    output.height = input.height;
    output.channels = input.channels;
    output.pixels = std::move(encrypted);

    hideBytesInImage(salt, 0, output);

    hideBytesInImage(iv, 128, output);
}

void AES256ImageEncryptor::decrypt(const Image& input, Image& output, const std::string& key) {
    if (input.width <= 0 || input.height <= 0 || input.channels <= 0) {
        throw std::runtime_error("Invalid input image dimensions");
    }

    const int pixelCount = input.width * input.height * input.channels;

    if (static_cast<int>(input.pixels.size()) != pixelCount) {
        throw std::runtime_error("Input pixel data size mismatch");
    }

    if (pixelCount < 256) {
        throw std::runtime_error("Image too small to extract salt and IV");
    }

    const std::vector<unsigned char> salt = extractBytesFromImage(0, 16, input);
    const std::vector<unsigned char> iv = extractBytesFromImage(128, 16, input);

    const AES256Encryptor aes(key, salt);
    std::vector<unsigned char> decrypted = aes.decrypt(input.pixels, iv);

    if (decrypted.size() != input.pixels.size()) {
        throw std::runtime_error("Decryption changed data size");
    }

    output.width = input.width;
    output.height = input.height;
    output.channels = input.channels;
    output.pixels = std::move(decrypted);
}
