#pragma once
#include "../CryptoAlgorithm.h"

class XORImageEncryptor final : public CryptoAlgorithm {
public:
    std::string name() const override { return "xor"; }

    void encrypt(const cv::Mat& inputImage, cv::Mat& outputImage, const std::string& key) override;
    void decrypt(const cv::Mat& inputImage, cv::Mat& outputImage, const std::string& key) override;
};
