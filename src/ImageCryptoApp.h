// ImageCryptoApp.h
#pragma once

#include <cxxopts.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include <map>
#include <vector>
#include "crypt/CryptoAlgorithm.h"
#include "img/ImageUtils.h"  // for step embedding helpers

class ImageCryptoApp {
public:
    ImageCryptoApp();
    void registerAlgorithms();
    void run(int argc, char** argv);
    void processImageWithAlgorithm();

private:
    std::shared_ptr<CryptoAlgorithm> getAlgorithm(const std::string& name);

    // CLI options and parsing
    cxxopts::Options options;
    cxxopts::ParseResult result;

    // Operation modes
    bool debug = false;
    bool decrypt = false;
    bool experimentalEmbed = false;

    // File paths
    std::filesystem::path inputPath;
    std::filesystem::path outputPath;

    // Encryption keys
    std::string masterPassword;

    // Histogram settings
    int histoBin = 64;
    int histoChartHeight = 20;

    // Images
    Image workImage;
    Image outImage;

    // Registered algorithms
    std::map<std::string, std::shared_ptr<CryptoAlgorithm>> algorithms;

    std::vector<std::string> stepsToRun;
};
