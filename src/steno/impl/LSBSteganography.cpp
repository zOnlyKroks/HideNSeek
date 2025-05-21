#include "LSBSteganography.h"
#include <zlib.h>
#include <openssl/rand.h>
#include <algorithm>
#include <stdexcept>
#include <iostream>

#include "../../img/ImageUtils.h"
#include "../../util/aes/AES256Encryptor.h"

LSBSteganography::LSBSteganography(int bitsPerChannel) : bitsPerChannel(std::clamp(bitsPerChannel, 1, 4)) {}

size_t LSBSteganography::maxHiddenDataSize(const Image& carrierImage) const {
    const int channels = carrierImage.channels;
    size_t totalPixels = carrierImage.getWidth() * carrierImage.getHeight();
    size_t totalBits = totalPixels * channels * bitsPerChannel;
    return (totalBits / 8) - 16; // subtract header
}

bool LSBSteganography::canEmbedData(const Image& carrierImage, const Image& imageToHide, const std::string& password) const {
    auto serialized = ImageUtils::serializeImage(imageToHide);
    const std::string dataToHide(serialized.begin(), serialized.end());

    const std::vector<unsigned char> data(dataToHide.begin(), dataToHide.end());
    uLongf originalSize = data.size();
    uLongf compSize = compressBound(originalSize);
    std::vector<unsigned char> compressed(compSize);

    if (compress(compressed.data(), &compSize, data.data(), originalSize) != Z_OK) return false;
    compressed.resize(compSize);

    std::vector<unsigned char> salt(8);
    if (!RAND_bytes(salt.data(), salt.size())) return false;

    AES256Encryptor aes(password, salt);
    std::vector<unsigned char> encrypted = aes.encrypt(compressed);
    encrypted.insert(encrypted.begin(), salt.begin(), salt.end());

    return encrypted.size() <= maxHiddenDataSize(carrierImage);
}

void LSBSteganography::embedByte(std::vector<unsigned char>& pixels, size_t& index, unsigned char byte) const {
    constexpr int bitsInByte = 8;
    const int chunks = (bitsInByte + bitsPerChannel - 1) / bitsPerChannel;
    const unsigned char mask = (1 << bitsPerChannel) - 1;
    const unsigned char clearMask = ~mask;

    for (int i = 0; i < chunks; ++i) {
        const unsigned char bits = (byte >> (i * bitsPerChannel)) & mask;
        pixels[index] = (pixels[index] & clearMask) | bits;
        ++index;
    }
}

unsigned char LSBSteganography::extractByte(const std::vector<unsigned char>& pixels, size_t& index) const {
    unsigned char byte = 0;
    constexpr int bitsInByte = 8;
    const int chunks = (bitsInByte + bitsPerChannel - 1) / bitsPerChannel;
    const unsigned char mask = (1 << bitsPerChannel) - 1;

    for (int i = 0; i < chunks; ++i) {
        byte |= (pixels[index] & mask) << (i * bitsPerChannel);
        ++index;
    }
    return byte;
}

void LSBSteganography::embedHeader(std::vector<unsigned char>& pixels, size_t& index, uint32_t dataSize) const {
    for (int i = 0; i < 4; ++i) {
        embedByte(pixels, index, (dataSize >> (i * 8)) & 0xFF);
    }
}

void LSBSteganography::extractHeader(const std::vector<unsigned char>& pixels, size_t& index, uint32_t& dataSize) const {
    dataSize = 0;
    for (int i = 0; i < 4; ++i) {
        dataSize |= static_cast<uint32_t>(extractByte(pixels, index)) << (i * 8);
    }
}

bool LSBSteganography::hideData(const Image& carrierImage, const std::string& dataToHide, Image& resultImage, const std::string& password) {
    const std::vector<unsigned char> data(dataToHide.begin(), dataToHide.end());
    const uLongf originalSize = data.size();
    uLongf compSize = compressBound(originalSize);
    std::vector<unsigned char> compressed(compSize);
    if (compress(compressed.data(), &compSize, data.data(), originalSize) != Z_OK) return false;
    compressed.resize(compSize);

    std::vector<unsigned char> salt(8);
    if (!RAND_bytes(salt.data(), salt.size())) return false;

    const AES256Encryptor aes(password, salt);
    std::vector<unsigned char> encrypted = aes.encrypt(compressed);
    encrypted.insert(encrypted.begin(), salt.begin(), salt.end());

    resultImage = carrierImage;
    std::vector<unsigned char> pixels = resultImage.getPixels();
    size_t index = 0;

    embedHeader(pixels, index, static_cast<uint32_t>(encrypted.size()));
    for (unsigned char byte : encrypted) embedByte(pixels, index, byte);

    resultImage.setPixels(pixels);
    return true;
}

bool LSBSteganography::extractData(const Image& steganoImage, std::string& extractedData, const std::string& password) {
    const auto pixels = steganoImage.getPixels();
    size_t index = 0;

    uint32_t dataSize = 0;
    extractHeader(pixels, index, dataSize);
    if (dataSize == 0 || dataSize > pixels.size()) return false;

    std::vector<unsigned char> encrypted;
    encrypted.reserve(dataSize);
    for (uint32_t i = 0; i < dataSize && index < pixels.size(); ++i) {
        encrypted.push_back(extractByte(pixels, index));
    }

    if (encrypted.size() < 8) return false;
    const std::vector salt(encrypted.begin(), encrypted.begin() + 8);
    encrypted.erase(encrypted.begin(), encrypted.begin() + 8);

    if (encrypted.size() % 16 != 0) return false;

    const AES256Encryptor aes(password, salt);
    std::vector<unsigned char> compressed;
    try {
        compressed = aes.decrypt(encrypted);
    } catch (...) {
        return false;
    }

    uLongf decompressedSize = steganoImage.getWidth() * steganoImage.getHeight() * steganoImage.channels;
    std::vector<unsigned char> decompressed(decompressedSize);
    if (uncompress(decompressed.data(), &decompressedSize, compressed.data(), compressed.size()) != Z_OK)
        return false;

    decompressed.resize(decompressedSize);
    extractedData.assign(decompressed.begin(), decompressed.end());
    return true;
}

bool LSBSteganography::hideImage(const Image& carrierImage, const Image& imageToHide, Image& resultImage, const std::string& key) {
    auto serialized = ImageUtils::serializeImage(imageToHide);
    const std::string dataToHide(serialized.begin(), serialized.end());
    return hideData(carrierImage, dataToHide, resultImage, key);
}

bool LSBSteganography::extractImage(const Image& steganoImage, Image& extractedImage, const std::string& key) {
    std::string extractedData;
    if (!extractData(steganoImage, extractedData, key)) return false;
    const std::vector<unsigned char> rawData(extractedData.begin(), extractedData.end());
    return ImageUtils::deserializeImage(rawData, extractedImage);
}