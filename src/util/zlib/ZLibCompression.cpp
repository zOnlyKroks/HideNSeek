#include "ZLibCompression.h"
#include <zlib.h>

std::vector<unsigned char> ZLIBCompression::compressData(const std::vector<unsigned char>& data) {
    uLongf compressedSize = compressBound(data.size());
    std::vector<unsigned char> compressedData(compressedSize);

    if (compress(compressedData.data(), &compressedSize, data.data(), data.size()) != Z_OK) {
        throw std::runtime_error("Compression failed");
    }

    compressedData.resize(compressedSize);
    return compressedData;
}

std::vector<unsigned char> ZLIBCompression::decompressData(const std::vector<unsigned char>& data, uLongf originalSize) {
    std::vector<unsigned char> decompressedData(originalSize);

    if (uncompress(decompressedData.data(), &originalSize, data.data(), data.size()) != Z_OK) {
        throw std::runtime_error("Decompression failed");
    }

    decompressedData.resize(originalSize);
    return decompressedData;
}
