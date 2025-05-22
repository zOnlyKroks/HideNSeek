#include "LSBSteganography.h"
#include <zlib.h>
#include <openssl/rand.h>
#include <algorithm>
#include <stdexcept>
#include <iostream>

#include "../../../img/ImageUtils.h"
#include "../../../util/aes/AES256Encryptor.h"

LSBSteganography::LSBSteganography(const int bitsPerChannel) : bitsPerChannel(std::clamp(bitsPerChannel, 1, 4)) {}

size_t LSBSteganography::maxHiddenDataSize(const Image& carrierImage) const {
    const int channels = carrierImage.channels;
    const size_t totalPixels = carrierImage.getWidth() * carrierImage.getHeight();
    const size_t totalBits = totalPixels * channels * bitsPerChannel;
    return (totalBits / 8) - 20;
}

std::tuple<bool, size_t, size_t> LSBSteganography::canEmbedData(const Image& carrierImage, const Image& imageToHide, const std::string& password) const {
    auto serialized = ImageUtils::serializeImage(imageToHide);
    const std::string dataToHide(serialized.begin(), serialized.end());

    const std::vector<unsigned char> data(dataToHide.begin(), dataToHide.end());
    const uLongf originalSize = data.size();
    uLongf compSize = compressBound(originalSize);
    std::vector<unsigned char> compressed(compSize);

    if (compress(compressed.data(), &compSize, data.data(), originalSize) != Z_OK) {
        return std::make_tuple(false, 0, 0);
    }

    compressed.resize(compSize);

    std::vector<unsigned char> salt(16);
    std::vector<unsigned char> iv(16);
    if (!RAND_bytes(salt.data(), 16) || !RAND_bytes(iv.data(), 16)) {
        throw std::runtime_error("Failed to generate random salt or IV");
    }

    const AES256Encryptor aes(password, salt);

    const std::vector<unsigned char> encrypted = aes.encrypt(compressed, iv);

    size_t totalSize = 16 + 16 + encrypted.size();

    return std::make_tuple(totalSize <= maxHiddenDataSize(carrierImage),
                           totalSize,
                           maxHiddenDataSize(carrierImage));
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

    std::vector<unsigned char> salt(16);
    std::vector<unsigned char> iv(16);
    if (!RAND_bytes(salt.data(), 16) || !RAND_bytes(iv.data(), 16)) {
        throw std::runtime_error("Failed to generate random salt or IV");
    }

    const AES256Encryptor aes(password, salt);

    std::vector<unsigned char> encrypted = aes.encrypt(compressed, iv);

    std::vector<unsigned char> fullData;
    fullData.reserve(16 + 16 + encrypted.size());
    fullData.insert(fullData.end(), salt.begin(), salt.end());
    fullData.insert(fullData.end(), iv.begin(), iv.end());
    fullData.insert(fullData.end(), encrypted.begin(), encrypted.end());

    resultImage = carrierImage;
    std::vector<unsigned char> pixels = resultImage.getPixels();
    size_t index = 0;

    embedHeader(pixels, index, static_cast<uint32_t>(fullData.size()));
    for (unsigned char byte : fullData) embedByte(pixels, index, byte);

    resultImage.setPixels(pixels);
    return true;
}

bool LSBSteganography::extractData(const Image& steganoImage, std::string& extractedData, const std::string& password) {
    const auto pixels = steganoImage.getPixels();
    size_t index = 0;

    uint32_t dataSize = 0;
    extractHeader(pixels, index, dataSize);
    if (dataSize == 0 || dataSize > pixels.size()) return false;

    std::vector<unsigned char> fullData;
    fullData.reserve(dataSize);
    for (uint32_t i = 0; i < dataSize && index < pixels.size(); ++i) {
        fullData.push_back(extractByte(pixels, index));
    }

    if (fullData.size() < 32) return false;

    const std::vector salt(fullData.begin(), fullData.begin() + 16);
    const std::vector iv(fullData.begin() + 16, fullData.begin() + 32);
    const std::vector encrypted(fullData.begin() + 32, fullData.end());

    if (encrypted.empty()) return false;

    const AES256Encryptor aes(password, salt);
    std::vector<unsigned char> compressed;

    try {
        compressed = aes.decrypt(encrypted, iv);
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
