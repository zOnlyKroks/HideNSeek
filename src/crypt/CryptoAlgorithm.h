#pragma once
#include <string>
#include <opencv2/opencv.hpp>

class CryptoAlgorithm {
public:
    virtual ~CryptoAlgorithm() = default;

    virtual void encrypt(const cv::Mat& inputImage, cv::Mat& outputImage, const std::string& key) = 0;
    virtual void decrypt(const cv::Mat& inputImage, cv::Mat& outputImage, const std::string& key) = 0;

    [[nodiscard]] virtual std::string name() const = 0;
};
