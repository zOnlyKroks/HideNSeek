//
// Created by Finn Rades on 19.05.25.
//

#include "ImageLoader.h"

#include <filesystem>

static std::string hashImage(const cv::Mat &image) {
    // Simple hash over image bytes using std::hash
    const auto size = image.total() * image.elemSize();
    const char* dataPtr = reinterpret_cast<const char*>(image.data);
    // Compute hash
    std::size_t h = std::hash<std::string_view>{}(std::string_view(dataPtr, size));
    // Convert to hex string
    std::ostringstream oss;
    oss << std::hex << std::setw(sizeof(std::size_t)*2) << std::setfill('0') << h;
    return oss.str();
}

cv::Mat ImageLoader::loadImage(const std::filesystem::path &path) {
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("Error: Path is empty");
    }

    cv::Mat img = cv::imread(path, cv::IMREAD_COLOR);
    if (img.empty()) {
        std::cerr << "Error: could not load image: " << path << "\n";
        throw std::runtime_error("Error: could not load image.");
    }

    if (img.channels() != 3) {
        cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
    }

    std::cout << "Successfully loaded image: " << path << "\n";

    return img;
}

void ImageLoader::saveImage(const std::filesystem::path &path, const cv::Mat &image, const bool hash) {
    const auto hashStr = hashImage(image);
    const auto dir = path.parent_path();
    const auto stem = path.stem().string();
    const auto ext  = path.extension().string();
    const std::filesystem::path newPath = dir / (stem + "_" + hashStr + ext);

    std::vector<int> params;
    params.push_back(cv::IMWRITE_PNG_COMPRESSION);
    params.push_back(0);

    // Write image
    if (!cv::imwrite(hash ? newPath : path, image, params)) {
        std::cerr << "Error: could not write image to: " << (hash ? newPath : path) << std::endl;
        throw std::runtime_error("Error: could not write image.");
    }

    std::cout << "Successfully processed and saved to " << (hash ? newPath : path) << std::endl;
}
