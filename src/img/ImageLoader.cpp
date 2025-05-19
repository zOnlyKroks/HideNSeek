#include "ImageLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <filesystem>
#include <iostream>
#include <iomanip>

static std::string hashImage(const Image& image) {
    // Simple hash over image bytes using std::hash
    const auto size = image.pixels.size();
    const auto dataPtr = reinterpret_cast<const char*>(image.pixels.data());
    std::size_t h = std::hash<std::string_view>{}(std::string_view(dataPtr, size));
    std::ostringstream oss;
    oss << std::hex << std::setw(sizeof(std::size_t)*2)
        << std::setfill('0') << h;
    return oss.str();
}

Image ImageLoader::loadImage(const std::filesystem::path &path) {
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("Error: Path does not exist: " + path.string());
    }

    Image img;
    int w, h, c;
    unsigned char* data = stbi_load(path.string().c_str(), &w, &h, &c, 3);
    if (!data) {
        throw std::runtime_error("Error: could not load image: " + path.string());
    }
    img.width = w;
    img.height = h;
    img.channels = 3;
    img.pixels.assign(data, data + (w * h * 3));
    stbi_image_free(data);

    std::cout << "Successfully loaded image via stb: " << path
              << " (" << img.width << "x" << img.height
              << ", channels=" << img.channels << ")\n";
    return img;
}

void ImageLoader::saveImage(const std::filesystem::path &path,
                            const Image &img,
                            const bool hash) {
    if (img.pixels.empty()) {
        throw std::runtime_error("Error: cannot save empty image");
    }
    if (img.channels != 3) {
        throw std::runtime_error("Error: only 3-channel images supported by saveImage");
    }

    std::string outPath = hash
        ? (path.parent_path() /
           (path.stem().string() + "_" + hashImage(img) + path.extension().string())).string()
        : path.string();

    std::cout << ">>> Saving image via stb: " << outPath
              << " (" << img.width << "x" << img.height
              << ", channels=" << img.channels << ")" << std::endl;

    // Write PNG losslessly
    if (!stbi_write_png(outPath.c_str(),
                        img.width, img.height, img.channels,
                        img.pixels.data(),
                        img.width * img.channels))
    {
        throw std::runtime_error("Error: could not write image to " + outPath);
    }

    std::cout << "Successfully saved image via stb: " << outPath << std::endl;
}
