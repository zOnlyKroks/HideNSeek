#pragma once

#include "Image.h"
#include <string>

namespace ImageUtils {
    /**
     * Prints basic information about an Image object.
     * @param img   Input Image struct
     * @param name  Optional name prefix
     */
    void printImageInfo(const Image& img, const std::string& name = "");
}