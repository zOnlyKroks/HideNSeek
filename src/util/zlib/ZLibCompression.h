#ifndef ZLIBCOMPRESSION_H
#define ZLIBCOMPRESSION_H

#include <vector>
#include <zconf.h>

class ZLIBCompression {
public:
    static std::vector<unsigned char> compressData(const std::vector<unsigned char>& data);
    static std::vector<unsigned char> decompressData(const std::vector<unsigned char>& data, uLongf originalSize);
};

#endif // ZLIBCOMPRESSION_H
