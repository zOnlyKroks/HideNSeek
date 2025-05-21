#include "LSBSteganography.h"
#include <zlib.h>
#include <openssl/rand.h>
#include <algorithm>
#include <stdexcept>
#include <iostream>

#include "../../img/ImageUtils.h"
#include "../../util/aes/AES256Encryptor.h"

LSBSteganography::LSBSteganography(int bits)
    : bitsPerChannel(std::min(4, std::max(1, bits))) {}

size_t LSBSteganography::maxHiddenDataSize(const Image& carrierImage) const {
    const int channels = carrierImage.channels;
    const size_t totalPixels = carrierImage.getWidth() * carrierImage.getHeight();
    const size_t totalBits = totalPixels * channels * bitsPerChannel;
    return (totalBits / 8) - 16;  // account for header
}

void LSBSteganography::embedByte(std::vector<unsigned char>& pixels, size_t& pixelIndex, const unsigned char byte) const {
    const int channelsNeeded = 8 / bitsPerChannel + (8 % bitsPerChannel != 0);
    const unsigned char clearMask = ~((1 << bitsPerChannel) - 1);

    for (int i = 0; i < channelsNeeded && pixelIndex < pixels.size(); ++i) {
        const int shift = i * bitsPerChannel;
        const unsigned char bits = (byte >> shift) & ((1 << bitsPerChannel) - 1);
        pixels[pixelIndex] = (pixels[pixelIndex] & clearMask) | bits;
        ++pixelIndex;
    }
}

unsigned char LSBSteganography::extractByte(const std::vector<unsigned char>& pixels, size_t& pixelIndex) const {
    unsigned char byte = 0;
    const int channelsNeeded = 8 / bitsPerChannel + (8 % bitsPerChannel != 0);
    const unsigned char extractMask = (1 << bitsPerChannel) - 1;

    for (int i = 0; i < channelsNeeded && pixelIndex < pixels.size(); ++i) {
        byte |= (pixels[pixelIndex] & extractMask) << (i * bitsPerChannel);
        ++pixelIndex;
    }
    return byte;
}

void LSBSteganography::embedHeader(std::vector<unsigned char>& pixels, size_t& index, const uint32_t dataSize) const {
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
    std::vector<unsigned char> data(dataToHide.begin(), dataToHide.end());

    uLongf originalSize = data.size();
    uLongf compSize = compressBound(originalSize);
    std::vector<unsigned char> compressedData(compSize);

    if (compress(compressedData.data(), &compSize, data.data(), originalSize) != Z_OK)
        return false;
    compressedData.resize(compSize);

    std::vector<unsigned char> salt(8);
    if (!RAND_bytes(salt.data(), salt.size()))
        return false;

    AES256Encryptor aes(password, salt);
    std::vector<unsigned char> encrypted = aes.encrypt(compressedData);

    // Prepend salt
    encrypted.insert(encrypted.begin(), salt.begin(), salt.end());

    if (encrypted.size() > maxHiddenDataSize(carrierImage))
        return false;

    resultImage = carrierImage;
    std::vector<unsigned char> pixels = resultImage.getPixels();
    size_t index = 0;

    embedHeader(pixels, index, static_cast<uint32_t>(encrypted.size()));
    for (unsigned char byte : encrypted) {
        embedByte(pixels, index, byte);
    }

    resultImage.setPixels(pixels);
    return true;
}

bool LSBSteganography::extractData(const Image& steganoImage, std::string& extractedData, const std::string& password) {
    const std::vector<unsigned char> pixels = steganoImage.getPixels();
    size_t index = 0;

    uint32_t dataSize = 0;
    extractHeader(pixels, index, dataSize);
    std::cerr << "[DEBUG] Extracted header data size: " << dataSize << std::endl;

    if (dataSize == 0 || dataSize > pixels.size()) {
        std::cerr << "[ERROR] Invalid dataSize extracted.\n";
        return false;
    }

    std::vector<unsigned char> encrypted;
    encrypted.reserve(dataSize);

    for (uint32_t i = 0; i < dataSize && index < pixels.size(); ++i) {
        encrypted.push_back(extractByte(pixels, index));
    }

    if (encrypted.size() < 8) {
        std::cerr << "[ERROR] Extracted data too short for salt.\n";
        return false;
    }

    const std::vector salt(encrypted.begin(), encrypted.begin() + 8);
    encrypted.erase(encrypted.begin(), encrypted.begin() + 8);

    if (encrypted.size() % 16 != 0) {
        std::cerr << "[ERROR] Encrypted data size not aligned to AES block size: " << encrypted.size() << "\n";
        return false;
    }

    const AES256Encryptor aes(password, salt);
    std::vector<unsigned char> compressed;

    try {
        compressed = aes.decrypt(encrypted);
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] AES decryption failed: " << e.what() << "\n";
        return false;
    }

    // Use a very large initial output size, then resize after decompression
    uLongf decompressedSizeGuess = steganoImage.getWidth() * steganoImage.getHeight() * steganoImage.channels;
    std::vector<unsigned char> decompressed(decompressedSizeGuess);

    if (uncompress(decompressed.data(), &decompressedSizeGuess, compressed.data(), compressed.size()) != Z_OK) {
        std::cerr << "[ERROR] ZLib decompression failed.\n";
        return false;
    }

    decompressed.resize(decompressedSizeGuess);
    extractedData.assign(decompressed.begin(), decompressed.end());
    return true;
}


bool LSBSteganography::hideImage(const Image& carrierImage, const Image& imageToHide, Image& resultImage, const std::string& key) {
    const std::vector<unsigned char> serialized = ImageUtils::serializeImage(imageToHide);
    const std::string dataToHide(reinterpret_cast<const char*>(serialized.data()), serialized.size());
    return hideData(carrierImage, dataToHide, resultImage, key);
}

bool LSBSteganography::extractImage(const Image& steganoImage, Image& extractedImage, const std::string& key) {
    std::string extractedData;
    if (!extractData(steganoImage, extractedData, key))
        return false;

    const std::vector<unsigned char> rawData(extractedData.begin(), extractedData.end());
    return ImageUtils::deserializeImage(rawData, extractedImage);
}
