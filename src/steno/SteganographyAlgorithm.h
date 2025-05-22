#pragma once

#include "../img/Image.h"
#include <string>
#include <tuple>

class SteganographyAlgorithm {
public:
    virtual ~SteganographyAlgorithm() = default;

    virtual bool hideData(const Image& carrierImage, const std::string& dataToHide,
                         Image& resultImage, const std::string& key = "") = 0;

    virtual bool hideImage(const Image& carrierImage, const Image& imageToHide,
                          Image& resultImage, const std::string& key = "") = 0;

    virtual bool extractData(const Image& steganoImage, std::string& extractedData,
                            const std::string& key = "") = 0;

    virtual bool extractImage(const Image& steganoImage, Image& extractedImage,
                             const std::string& key = "") = 0;

    virtual std::string name() const = 0;

    virtual std::string description() const = 0;

    virtual size_t maxHiddenDataSize(const Image& carrierImage) const = 0;

    virtual std::tuple<bool, size_t, size_t> canEmbedData(
    const Image& carrierImage,
    const Image& imageToHide,
    const std::string& password = ""
        ) const = 0;
};