#pragma once

#include "Image.h"
#include <string>

namespace ImageUtils {
    void printImageInfo(const Image& img, const std::string& name = "");

    Image convertTo3Channels(const Image& src);

    void embedMetadataImage(Image& targetImage, const std::string& key, const Image& dataImage);

    Image extractMetadataImage(const Image& sourceImage, const std::string& key);

    Image textToImage(const std::string& text);

    std::string textFromImage(const Image& img);

    std::vector<unsigned char> serializeImage(const Image& img);
    bool deserializeImage(const std::vector<unsigned char>& data, Image& img);
}