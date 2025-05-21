#include "PVDSteganography.h"
#include <zlib.h>
#include <openssl/rand.h>
#include <vector>
#include <stdexcept>
#include <iostream>

#include "../../../img/ImageUtils.h"
#include "../../../util/aes/AES256Encryptor.h"

size_t PVDSteganography::maxHiddenDataSize(const Image& carrierImage) const {
    size_t capacity = 0;
    const auto& pixels = carrierImage.getPixels();
    for (size_t i = 0; i + 1 < pixels.size(); i += 2) {
        if (const int diff = std::abs(pixels[i] - pixels[i + 1]); diff < 16) capacity += 1;
        else if (diff < 32) capacity += 2;
        else if (diff < 64) capacity += 3;
        else if (diff < 128) capacity += 4;
        else capacity += 5;
    }
    return capacity / 8;
}

bool PVDSteganography::canEmbedData(const Image& carrierImage, const Image& imageToHide, const std::string& password) const {
    const std::vector<unsigned char> dataToHide = ImageUtils::serializeImage(imageToHide);
    const std::vector data(dataToHide.begin(), dataToHide.end());

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

    return encrypted.size() <= maxHiddenDataSize(carrierImage);
}

bool PVDSteganography::hideData(const Image& carrierImage, const std::string& dataToHide, Image& resultImage, const std::string& password) {
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

    if (encrypted.size() * 8 > maxHiddenDataSize(carrierImage) * 8) return false;

    resultImage = carrierImage;
    std::vector<unsigned char> pixels = resultImage.getPixels();
    size_t dataBitIndex = 0;

    for (size_t i = 0; i + 1 < pixels.size() && dataBitIndex < encrypted.size() * 8; i += 2) {
        int p1 = pixels[i];
        int p2 = pixels[i + 1];
        const int diff = std::abs(p1 - p2);
        const int range = getBitCapacity(diff);

        unsigned char bits = 0;
        for (int b = 0; b < range; ++b) {
            if (dataBitIndex < encrypted.size() * 8) {
                bits |= ((encrypted[dataBitIndex / 8] >> (dataBitIndex % 8)) & 1) << b;
                ++dataBitIndex;
            }
        }

        int newDiff = getMinDiffForBits(range) + bits;
        if (p1 > p2) p1 = std::clamp(p2 + newDiff, 0, 255);
        else        p2 = std::clamp(p1 + newDiff, 0, 255);

        pixels[i] = static_cast<unsigned char>(p1);
        pixels[i + 1] = static_cast<unsigned char>(p2);
    }

    resultImage.setPixels(pixels);
    return true;
}

bool PVDSteganography::extractData(const Image& steganoImage, std::string& extractedData, const std::string& password) {
    const std::vector<unsigned char>& pixels = steganoImage.getPixels();
    std::vector<unsigned char> encrypted;
    unsigned char currentByte = 0;
    int bitPos = 0;

    for (size_t i = 0; i + 1 < pixels.size(); i += 2) {
        const int diff = std::abs(pixels[i] - pixels[i + 1]);
        const int range = getBitCapacity(diff);

        for (int b = 0; b < range; ++b) {
            currentByte |= ((diff >> b) & 1) << bitPos++;
            if (bitPos == 8) {
                encrypted.push_back(currentByte);
                currentByte = 0;
                bitPos = 0;
            }
        }
    }

    if (encrypted.size() < 8) return false;
    const std::vector salt(encrypted.begin(), encrypted.begin() + 8);
    encrypted.erase(encrypted.begin(), encrypted.begin() + 8);

    const AES256Encryptor aes(password, salt);
    std::vector<unsigned char> compressed;
    try {
        compressed = aes.decrypt(encrypted);
    } catch (...) {
        return false;
    }

    uLongf decompressedSizeGuess = steganoImage.getWidth() * steganoImage.getHeight() * steganoImage.channels;
    std::vector<unsigned char> decompressed(decompressedSizeGuess);
    if (uncompress(decompressed.data(), &decompressedSizeGuess, compressed.data(), compressed.size()) != Z_OK)
        return false;

    decompressed.resize(decompressedSizeGuess);
    extractedData.assign(decompressed.begin(), decompressed.end());
    return true;
}

bool PVDSteganography::hideImage(const Image& carrierImage, const Image& imageToHide, Image& resultImage, const std::string& key) {
    const auto serialized = ImageUtils::serializeImage(imageToHide);
    return hideData(carrierImage, std::string(serialized.begin(), serialized.end()), resultImage, key);
}

bool PVDSteganography::extractImage(const Image& steganoImage, Image& extractedImage, const std::string& key) {
    std::string extractedData;
    if (!extractData(steganoImage, extractedData, key)) return false;
    const std::vector<unsigned char> rawData(extractedData.begin(), extractedData.end());
    return ImageUtils::deserializeImage(rawData, extractedImage);
}

int PVDSteganography::getBitCapacity(const int diff) {
    if (diff < 16) return 1;
    if (diff < 32) return 2;
    if (diff < 64) return 3;
    if (diff < 128) return 4;
    return 5;
}

int PVDSteganography::getMinDiffForBits(const int bits) {
    switch (bits) {
        case 1: return 0;
        case 2: return 16;
        case 3: return 32;
        case 4: return 64;
        case 5: return 128;
        default: return 0;
    }
}
