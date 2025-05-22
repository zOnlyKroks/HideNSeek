#pragma once

#include <string>
#include <vector>
#include "../../../img/Image.h"
#include "../../SteganographyAlgorithm.h"

class PVDSteganography final : public SteganographyAlgorithm {
public:
    std::string name() const override { return "pvd"; }

    std::string description() const override {
        return "Pixel Value Differencing (PVD) with Hybrid LSB in textured regions";
    }

    bool hideData(const Image& carrierImage, const std::string& dataToHide,
                  Image& resultImage, const std::string& password = "") override;

    bool extractData(const Image& steganoImage, std::string& extractedData,
                     const std::string& password = "") override;

    bool hideImage(const Image& carrierImage, const Image& imageToHide,
                   Image& resultImage, const std::string& password = "") override;

    bool extractImage(const Image& steganoImage, Image& extractedImage,
                      const std::string& password = "") override;

    std::tuple<bool, size_t, size_t> canEmbedData(
    const Image& carrierImage,
    const Image& imageToHide,
    const std::string& password = ""
        ) const override;

    size_t maxHiddenDataSize(const Image& carrierImage) const override;

private:
    static int getBitCapacity(int diff);

    static int getMinDiffForBits(int bits);

    static void applySobel(const Image& img, std::vector<unsigned char>& edges);

    static bool isTextured(const std::vector<unsigned char>& edges, int x, int y, int width);

    static constexpr int SobelThreshold = 100;
};
