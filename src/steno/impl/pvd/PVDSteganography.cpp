#include "PVDSteganography.h"
#include <zlib.h>
#include <openssl/rand.h>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <iostream>

#include "../../../img/ImageUtils.h"
#include "../../../util/aes/AES256Encryptor.h"

namespace {
    void embedLSB(unsigned char& byte, const unsigned char bit) {
        byte = (byte & ~1) | (bit & 1);
    }

    unsigned char extractLSB(const unsigned char& byte) {
        return byte & 1;
    }

    void embedBitsPVD(unsigned char& p1, unsigned char& p2, const unsigned char bits, const int bitCount) {
        const int d = std::abs(p1 - p2);
        const int newDiff = ((d >> bitCount) << bitCount) + bits;
        if (p1 > p2) p1 = std::clamp<int>(p2 + newDiff, 0, 255);
        else         p2 = std::clamp<int>(p1 + newDiff, 0, 255);
    }

    unsigned char extractBitsPVD(const unsigned char p1, const unsigned char p2, const int bitCount) {
        const int d = std::abs(p1 - p2);
        return d & ((1 << bitCount) - 1);
    }
}

size_t PVDSteganography::maxHiddenDataSize(const Image& carrierImage) const {
    const int width = carrierImage.getWidth();
    const int height = carrierImage.getHeight();
    return (width * height * 3) / 8;
}

std::tuple<bool, size_t, size_t> PVDSteganography::canEmbedData(const Image& carrierImage, const Image& imageToHide, const std::string& password) const {
    const auto serialized = ImageUtils::serializeImage(imageToHide);
    const uLongf originalSize = serialized.size();
    uLongf compSize = compressBound(originalSize);
    std::vector<unsigned char> compressed(compSize);
    if (compress(compressed.data(), &compSize, serialized.data(), originalSize) != Z_OK) return std::make_tuple(false,0,0);

    compressed.resize(compSize);

    std::vector<unsigned char> salt(16), iv(16);
    if (!RAND_bytes(salt.data(), 16) || !RAND_bytes(iv.data(), 16)) return std::make_tuple(false,0,0);

    const AES256Encryptor aes(password, salt);
    const std::vector<unsigned char> encrypted = aes.encrypt(compressed, iv);
    size_t payloadSize = 4 + 16 + 16 + encrypted.size();

    return std::make_tuple(payloadSize <= maxHiddenDataSize(carrierImage), payloadSize, maxHiddenDataSize(carrierImage));
}

bool PVDSteganography::hideImage(const Image& carrierImage, const Image& imageToHide, Image& resultImage, const std::string& password) {
    auto serialized = ImageUtils::serializeImage(imageToHide);
    const std::string data(serialized.begin(), serialized.end());
    return hideData(carrierImage, data, resultImage, password);
}

bool PVDSteganography::extractImage(const Image& steganoImage, Image& extractedImage, const std::string& password) {
    std::string data;
    if (!extractData(steganoImage, data, password)) return false;
    return ImageUtils::deserializeImage(std::vector<unsigned char>(data.begin(), data.end()), extractedImage);
}

bool PVDSteganography::hideData(const Image& carrierImage, const std::string& dataToHide, Image& resultImage, const std::string& password) {
    const std::vector<unsigned char> data(dataToHide.begin(), dataToHide.end());
    const uLongf originalSize = data.size();
    uLongf compSize = compressBound(originalSize);
    std::vector<unsigned char> compressed(compSize);

    if (compress(compressed.data(), &compSize, data.data(), originalSize) != Z_OK) return false;
    compressed.resize(compSize);

    std::vector<unsigned char> salt(16), iv(16);
    if (!RAND_bytes(salt.data(), 16) || !RAND_bytes(iv.data(), 16)) return false;

    const AES256Encryptor aes(password, salt);
    std::vector<unsigned char> encrypted = aes.encrypt(compressed, iv);

    std::vector<unsigned char> payload;
    uint32_t dataSize = encrypted.size();
    payload.insert(payload.end(), reinterpret_cast<unsigned char*>(&dataSize), reinterpret_cast<unsigned char*>(&dataSize) + 4);
    payload.insert(payload.end(), salt.begin(), salt.end());
    payload.insert(payload.end(), iv.begin(), iv.end());
    payload.insert(payload.end(), encrypted.begin(), encrypted.end());

    resultImage = carrierImage;
    std::vector<unsigned char> pixels = resultImage.getPixels();
    const int width = carrierImage.getWidth();
    const int height = carrierImage.getHeight();

    std::vector<unsigned char> gray(height * width);
    for (size_t i = 0; i < gray.size(); ++i) {
        gray[i] = static_cast<unsigned char>(0.299f * pixels[i * 3 + 0] + 0.587f * pixels[i * 3 + 1] + 0.114f * pixels[i * 3 + 2]);
    }

    std::vector<unsigned char> edges(gray.size());
    applySobel(carrierImage, edges);

    size_t dataBitIndex = 0;
    for (int y = 0; y < height && dataBitIndex < payload.size() * 8; ++y) {
        for (int x = 0; x + 1 < width && dataBitIndex < payload.size() * 8; x += 2) {
            const int i1 = (y * width + x) * 3;
            const int i2 = (y * width + x + 1) * 3;
            if (isTextured(edges, x, y, width)) {
                for (int c = 0; c < 3 && dataBitIndex < payload.size() * 8; ++c) {
                    const unsigned char bit = (payload[dataBitIndex / 8] >> (dataBitIndex % 8)) & 1;
                    embedLSB(pixels[i1 + c], bit);
                    ++dataBitIndex;
                }
            } else {
                const int bits = getBitCapacity(std::abs(pixels[i1] - pixels[i2]));
                unsigned char chunk = 0;
                for (int b = 0; b < bits && dataBitIndex < payload.size() * 8; ++b) {
                    chunk |= ((payload[dataBitIndex / 8] >> (dataBitIndex % 8)) & 1) << b;
                    ++dataBitIndex;
                }
                embedBitsPVD(pixels[i1], pixels[i2], chunk, bits);
            }
        }
    }

    resultImage.setPixels(pixels);
    return true;
}

bool PVDSteganography::extractData(const Image& steganoImage, std::string& extractedData, const std::string& password) {
    const auto pixels = steganoImage.getPixels();
    const int width = steganoImage.getWidth();
    const int height = steganoImage.getHeight();

    std::vector<unsigned char> gray(width * height);
    for (size_t i = 0; i < gray.size(); ++i) {
        gray[i] = static_cast<unsigned char>(0.299f * pixels[i * 3 + 0] + 0.587f * pixels[i * 3 + 1] + 0.114f * pixels[i * 3 + 2]);
    }

    std::vector<unsigned char> edges(gray.size());
    applySobel(steganoImage, edges);

    std::vector<unsigned char> extracted;
    unsigned char currentByte = 0;
    int bitCount = 0;
    size_t extractedSize = 0;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x + 1 < width; x += 2) {
            const int i1 = (y * width + x) * 3;
            const int i2 = (y * width + x + 1) * 3;

            if (isTextured(edges, x, y, width)) {
                for (int c = 0; c < 3; ++c) {
                    currentByte |= (extractLSB(pixels[i1 + c]) << bitCount++);
                    if (bitCount == 8) {
                        extracted.push_back(currentByte);
                        if (extracted.size() == 4) {
                            extractedSize = *reinterpret_cast<uint32_t*>(extracted.data());
                        }
                        bitCount = 0;
                        currentByte = 0;
                        if (extractedSize && extracted.size() >= extractedSize + 36) goto done;
                    }
                }
            } else {
                const int bits = getBitCapacity(std::abs(pixels[i1] - pixels[i2]));
                const unsigned char val = extractBitsPVD(pixels[i1], pixels[i2], bits);
                for (int b = 0; b < bits; ++b) {
                    currentByte |= ((val >> b) & 1) << bitCount++;
                    if (bitCount == 8) {
                        extracted.push_back(currentByte);
                        if (extracted.size() == 4) {
                            extractedSize = *reinterpret_cast<uint32_t*>(extracted.data());
                        }
                        bitCount = 0;
                        currentByte = 0;
                        if (extractedSize && extracted.size() >= extractedSize + 36) goto done;
                    }
                }
            }
        }
    }

    done:
    if (extracted.size() < 36) return false;

    const std::vector salt(extracted.begin() + 4, extracted.begin() + 20);
    const std::vector iv(extracted.begin() + 20, extracted.begin() + 36);
    const std::vector encrypted(extracted.begin() + 36, extracted.end());

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

void PVDSteganography::applySobel(const Image& img, std::vector<unsigned char>& edges) {
    const int width = img.getWidth();
    const int height = img.getHeight();
    const std::vector<unsigned char>& pixels = img.getPixels();

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            int gx = 0, gy = 0;
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    const int px = x + kx;
                    const int py = y + ky;
                    const int i = (py * width + px) * 3;
                    const unsigned char gray = static_cast<unsigned char>(
                        0.299f * pixels[i] + 0.587f * pixels[i + 1] + 0.114f * pixels[i + 2]);
                    gx += kx * gray;
                    gy += ky * gray;
                }
            }
            const int mag = std::sqrt(gx * gx + gy * gy);
            edges[y * width + x] = (mag > SobelThreshold) ? 255 : 0;
        }
    }
}

bool PVDSteganography::isTextured(const std::vector<unsigned char>& edges, const int x, const int y, const int width) {
    return edges[y * width + x] > 0;
}