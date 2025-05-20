#pragma once

#include "Image.h"
#include <filesystem>

class ImageLoader {
public:
    static Image loadImage(const std::filesystem::path &path);

    static void saveImage(const std::filesystem::path &path,
                          Image &img,
                          bool hash = false);
};
