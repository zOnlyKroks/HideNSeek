#include "BlowfishImageEncryptor.h"
#include <openssl/rand.h>
#include <stdexcept>

#include "../../../util/blowfish/BlowfishEncryptor.h"

static void hideBytesInImage(const std::vector<unsigned char>& data, int startPixel, Image& img) {
    const int totalPixels = img.width * img.height * img.channels;
    const int bitsToHide = static_cast<int>(data.size()) * 8;

    if (startPixel + bitsToHide > totalPixels) {
        throw std::runtime_error("Image too small to hide data");
    }

    std::vector<unsigned char>& pixels = img.pixels;
    for (int bitIdx = 0; bitIdx < bitsToHide; ++bitIdx) {
        int byteIdx = bitIdx / 8;
        int bitPos = 7 - (bitIdx % 8);
        unsigned char bit = (data[byteIdx] >> bitPos) & 1;

        int pixelIdx = startPixel + bitIdx;
        pixels[pixelIdx] = (pixels[pixelIdx] & 0xFE) | bit;
    }
}

static std::vector<unsigned char> extractBytesFromImage(int startPixel, int byteCount, const Image& img) {
    const int totalPixels = img.width * img.height * img.channels;
    const int bitsToExtract = byteCount * 8;

    if (startPixel + bitsToExtract > totalPixels) {
        throw std::runtime_error("Image too small to extract data");
    }

    std::vector<unsigned char> data(byteCount, 0);
    const std::vector<unsigned char>& pixels = img.pixels;

    for (int bitIdx = 0; bitIdx < bitsToExtract; ++bitIdx) {
        int byteIdx = bitIdx / 8;
        int bitPos = 7 - (bitIdx % 8);
        if (pixels[startPixel + bitIdx] & 1) {
            data[byteIdx] |= (1 << bitPos);
        }
    }
    return data;
}

void BlowfishImageEncryptor::encrypt(const Image& input, Image& output, const std::string& key) {
    if (input.width <= 0 || input.height <= 0 || input.channels <= 0) {
        throw std::runtime_error("Invalid input image dimensions");
    }

    int pixelCount = input.width * input.height * input.channels;
    if (static_cast<int>(input.pixels.size()) != pixelCount) {
        throw std::runtime_error("Input pixel data size mismatch");
    }
    if (pixelCount < 128) {
        throw std::runtime_error("Image too small to hide salt and IV");
    }

    // Generate 8-byte salt and 8-byte IV
    std::vector<unsigned char> salt(8), iv(8);
    if (!RAND_bytes(salt.data(), 8) || !RAND_bytes(iv.data(), 8)) {
        throw std::runtime_error("Failed to generate random salt or IV");
    }

    // Use Blowfish in CFB mode (stream cipher, no padding)
    BlowfishEncryptor blowfish(key, salt, BlowfishEncryptor::Mode::CFB);
    std::vector<unsigned char> encrypted = blowfish.encrypt(input.pixels, iv);

    // encrypted.size() == input.pixels.size() in CFB mode
    output.width = input.width;
    output.height = input.height;
    output.channels = input.channels;
    output.pixels = std::move(encrypted);

    // Hide salt then IV in LSBs of the first 128 pixels
    hideBytesInImage(salt, 0, output);
    hideBytesInImage(iv, 64, output);
}

void BlowfishImageEncryptor::decrypt(const Image& input, Image& output, const std::string& key) {
    if (input.width <= 0 || input.height <= 0 || input.channels <= 0) {
        throw std::runtime_error("Invalid input image dimensions");
    }

    int pixelCount = input.width * input.height * input.channels;
    if (static_cast<int>(input.pixels.size()) != pixelCount) {
        throw std::runtime_error("Input pixel data size mismatch");
    }
    if (pixelCount < 128) {
        throw std::runtime_error("Image too small to extract salt and IV");
    }

    // Extract salt and IV
    std::vector<unsigned char> salt = extractBytesFromImage(0, 8, input);
    std::vector<unsigned char> iv   = extractBytesFromImage(64, 8, input);

    // Use Blowfish in CFB mode (stream cipher, no padding)
    BlowfishEncryptor blowfish(key, salt, BlowfishEncryptor::Mode::CFB);
    std::vector<unsigned char> decrypted = blowfish.decrypt(input.pixels, iv);

    // decrypted.size() == input.pixels.size()
    output.width = input.width;
    output.height = input.height;
    output.channels = input.channels;
    output.pixels = std::move(decrypted);
}
