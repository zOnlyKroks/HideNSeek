#pragma once
#include <string>
#include <vector>

#include "../SteganographyAlgorithm.h"
#include "../../img/Image.h"

class LSBSteganography final : public SteganographyAlgorithm {
public:
    explicit LSBSteganography(int bitsPerChannel);

    bool hideData(const Image& carrierImage, const std::string& dataToHide,
                  Image& resultImage, const std::string& password = "") override;

    bool extractData(const Image& steganoImage, std::string& extractedData,
                     const std::string& password = "") override;

    bool hideImage(const Image &carrierImage, const Image &imageToHide, Image &resultImage, const std::string &key) override;

    bool extractImage(const Image &steganoImage, Image &extractedImage, const std::string &key) override;

    size_t maxHiddenDataSize(const Image& carrierImage) const override;

    std::string name() const override { return "lsb"; }

    std::string description() const override {
        return "Least Significant Bit (LSB) Steganography";
    }

private:
    void embedByte(std::vector<unsigned char>& pixels, size_t& pixelIndex,
                   unsigned char byte) const;
    unsigned char extractByte(const std::vector<unsigned char>& pixels, size_t& pixelIndex) const;

    void embedHeader(std::vector<unsigned char>& pixels, size_t& index,
                     uint32_t dataSize) const;
    void extractHeader(const std::vector<unsigned char>& pixels, size_t& index,
                       uint32_t& dataSize) const;

    int bitsPerChannel;
};
