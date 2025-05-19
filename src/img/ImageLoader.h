#pragma once
#include <filesystem>
#include <string>
#include <opencv2/opencv.hpp>

class ImageLoader {
public:
    static cv::Mat loadImage(const std::filesystem::path &path);
    static void saveImage(const std::filesystem::path &path, const cv::Mat &image, bool hash);
};
