#pragma once
#include <cxxopts.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include <map>
#include "crypt/CryptoAlgorithm.h"

class ImageCryptoApp {
public:
    ImageCryptoApp();
    void registerAlgorithms();
    void run(int argc, char** argv);
    void processImageWithAlgorithm();
    cxxopts::Options options;
    cxxopts::ParseResult result;

    bool debug = false;
    bool decrypt = false;

    std::filesystem::path inputPath;
    std::filesystem::path outputPath;

    int histoBin = 64;
    int histoChartHeight = 20;

    Image workImage;
    Image outImage;

private:
    std::shared_ptr<CryptoAlgorithm> getAlgorithm(const std::string& name);

    std::map<std::string, std::shared_ptr<CryptoAlgorithm>> algorithms;
};
