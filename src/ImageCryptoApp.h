#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "crypt/CryptoAlgorithm.h"
#include "img/Image.h"
#include <cxxopts.hpp>

#include "steno/SteganographyAlgorithm.h"

class ImageCryptoApp {
public:
    ImageCryptoApp();

    void run(int argc, char** argv);
    void registerAlgorithms();
    void processImageWithAlgorithm();

    void processSteganography();
    void hide();
    void extract();

    bool isInSteganographyMode() const { return !stegMode.empty(); }

private:
    std::shared_ptr<CryptoAlgorithm> getAlgorithm(const std::string& name);
    std::shared_ptr<SteganographyAlgorithm> getSteganographyAlgorithm(const std::string& name);

    cxxopts::Options options;
    cxxopts::ParseResult result;

    Image workImage;
    Image outImage;
    Image hideImage;

    std::string inputPath;
    std::string outputPath;
    std::string hideDataPath;
    bool debug = false;
    bool decrypt = false;

    std::vector<std::string> stepsToRun;
    std::string masterPassword;

    std::string stegMode;
    std::string stegAlgo;
    std::string stegPassword;
    std::string hiddenData;
    bool hideAsImage = false;

    int histoBin = 10;
    int histoChartHeight = 10;

    std::map<std::string, std::shared_ptr<CryptoAlgorithm>> algorithms;
    std::map<std::string, std::shared_ptr<SteganographyAlgorithm>> stegAlgorithms;
};
