#include "ImageLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

#include "ImageUtils.h"

static std::string hashImage(const Image& image) {
    // Simple hash over image bytes using std::hash
    const auto size = image.pixels.size();
    const auto dataPtr = reinterpret_cast<const char*>(image.pixels.data());
    const std::size_t h = std::hash<std::string_view>{}(std::string_view(dataPtr, size));
    std::ostringstream oss;
    oss << std::hex << std::setw(sizeof(std::size_t)*2)
        << std::setfill('0') << h;
    return oss.str();
}

// Reads metadata from a separate file with the same base name as the image
void loadMetadataFromFile(const std::filesystem::path &path, Image &img) {
    // Create metadata file path
    auto metaPath = path;
    metaPath.replace_extension(path.extension().string() + ".meta");

    // Check if metadata file exists
    if (!std::filesystem::exists(metaPath)) {
        return; // No metadata file
    }

    std::ifstream metaFile(metaPath);
    if (!metaFile.is_open()) {
        std::cerr << "Warning: Could not open metadata file: " << metaPath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(metaFile, line)) {
        // Skip empty lines
        if (line.empty()) continue;

        // Parse key-value pairs
        size_t separatorPos = line.find('=');
        if (separatorPos != std::string::npos) {
            std::string key = line.substr(0, separatorPos);
            std::string value = line.substr(separatorPos + 1);
            img.metadata()[key] = value;
        }
    }

    std::cout << "Loaded " << img.metadata().size() << " metadata entries from " << metaPath << std::endl;
}

// Saves metadata to a separate file with the same base name as the image
void saveMetadataToFile(const std::filesystem::path &path, const Image &img) {
    if (img.metadata().empty()) {
        return; // No metadata to save
    }

    // Create metadata file path
    auto metaPath = path;
    metaPath.replace_extension(path.extension().string() + ".meta");

    std::ofstream metaFile(metaPath);
    if (!metaFile.is_open()) {
        std::cerr << "Warning: Could not open metadata file for writing: " << metaPath << std::endl;
        return;
    }

    for (const auto& [key, value] : img.metadata()) {
        metaFile << key << "=" << value << std::endl;
    }

    std::cout << "Saved " << img.metadata().size() << " metadata entries to " << metaPath << std::endl;
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

    // Load metadata from companion file
    loadMetadataFromFile(path, img);

    std::cout << "Successfully loaded image via stb: " << path
              << " (" << img.width << "x" << img.height
              << ", channels=" << img.channels
              << ", metadata entries=" << img.metadata().size() << ")\n";
    return img;
}

void ImageLoader::saveImage(const std::filesystem::path &path,
                            Image &img,
                            const bool hash) {
    if (img.pixels.empty()) {
        throw std::runtime_error("Error: cannot save empty image");
    }
    if (img.channels != 3) {
        img = ImageUtils::convertTo3Channels(img);
    }

    const std::string outPath = hash
        ? (path.parent_path() /
           (path.stem().string() + "_" + hashImage(img) + path.extension().string())).string()
        : path.string();

    std::cout << ">>> Saving image via stb: " << outPath
              << " (" << img.width << "x" << img.height
              << ", channels=" << img.channels
              << ", metadata entries=" << img.metadata().size() << ")" << std::endl;

    // Write PNG losslessly
    if (!stbi_write_png(outPath.c_str(),
                        img.width, img.height, img.channels,
                        img.pixels.data(),
                        img.width * img.channels))
    {
        throw std::runtime_error("Error: could not write image to " + outPath);
    }

    // Save metadata to companion file
    saveMetadataToFile(outPath, img);

    std::cout << "Successfully saved image via stb: " << outPath << std::endl;
}