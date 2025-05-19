#include "XORAlgorithm.h"
#include <stdexcept>
#include <numeric>
#include <random>
#include <opencv2/opencv.hpp>

static uint64_t seedFromKey(const std::string &key) {
    return std::hash<std::string>{}(key);
}

void XORImageEncryptor::encrypt(const cv::Mat& inputImage,
                                cv::Mat& outputImage,
                                const std::string& key) {
    if (inputImage.depth() != CV_8U)
        throw std::runtime_error("Only 8‑bit images supported");

    if (inputImage.channels() != 3)
        throw std::runtime_error("Only 3-channel (color) images supported");

    outputImage.create(inputImage.size(), CV_8UC3);

    int rows = inputImage.rows;
    int cols = inputImage.cols;
    int totalPixels = rows * cols;

    std::vector<int> indices(totalPixels);
    std::iota(indices.begin(), indices.end(), 0);

    std::mt19937 rng(seedFromKey(key));
    std::shuffle(indices.begin(), indices.end(), rng);

    for (int i = 0; i < totalPixels; ++i) {
        int srcIdx = indices[i];
        int srcRow = srcIdx / cols;
        int srcCol = srcIdx % cols;
        int dstRow = i / cols;
        int dstCol = i % cols;
        outputImage.at<cv::Vec3b>(dstRow, dstCol) = inputImage.at<cv::Vec3b>(srcRow, srcCol);
    }
}

void XORImageEncryptor::decrypt(const cv::Mat& inputImage,
                                cv::Mat& outputImage,
                                const std::string& key) {
    if (inputImage.depth() != CV_8U)
        throw std::runtime_error("Only 8‑bit images supported");

    if (inputImage.channels() != 3)
        throw std::runtime_error("Only 3-channel (color) images supported");

    outputImage.create(inputImage.size(), CV_8UC3);

    int rows = inputImage.rows;
    int cols = inputImage.cols;
    int totalPixels = rows * cols;

    std::vector<int> indices(totalPixels);
    std::iota(indices.begin(), indices.end(), 0);

    std::mt19937 rng(seedFromKey(key));
    std::shuffle(indices.begin(), indices.end(), rng);

    std::vector<cv::Vec3b> restored(totalPixels);

    for (int i = 0; i < totalPixels; ++i) {
        int srcRow = i / cols;
        int srcCol = i % cols;
        int dstIdx = indices[i];
        restored[dstIdx] = inputImage.at<cv::Vec3b>(srcRow, srcCol);
    }

    for (int i = 0; i < totalPixels; ++i) {
        int dstRow = i / cols;
        int dstCol = i % cols;
        outputImage.at<cv::Vec3b>(dstRow, dstCol) = restored[i];
    }
}