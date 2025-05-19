#include "ImageUtils.h"
#include <iostream>
#include <iomanip>

namespace ImageUtils {

    void printImageInfo(const Image& img, const std::string& name) {
        const std::string prefix = name.empty() ? "" : name + ": ";
        const int channels = img.channels;
        const long long totalPixels = static_cast<long long>(img.width) * img.height;

        std::cout << prefix
                  << "size=" << img.width << "x" << img.height
                  << ", channels=" << channels
                  << ", total pixels=" << totalPixels
                  << std::endl;
    }

} // namespace ImageUtils
