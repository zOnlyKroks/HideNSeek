#pragma once

#include <string>

#include "../../SteganographyAlgorithm.h"

class PVDSteganography final : public SteganographyAlgorithm {
public:
    std::string name() const override { return "pvd"; }
    std::string description() const override { return "Pixel Value Differencing (PVD) Steganography"; }

    size_t maxHiddenDataSize(const Image& carrierImage) const override;
    bool hideData(const Image& carrierImage, const std::string& dataToHide, Image& resultImage, const std::string& password = "") override;
    bool extractData(const Image& steganoImage, std::string& extractedData, const std::string& password = "") override;

    bool hideImage(const Image& carrierImage, const Image& imageToHide, Image& resultImage, const std::string& key) override;
    bool extractImage(const Image& steganoImage, Image& extractedImage, const std::string& key) override;

    bool canEmbedData(const Image& carrierImage, const Image& imageToHide, const std::string& password = "") const override;
private:
    static int getBitCapacity(int diff);
    static int getMinDiffForBits(int bits);
};
