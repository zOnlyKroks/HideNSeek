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

    // Main processing methods
    void processEncryptionMode();               // New encryption processing
    void processImageEncryption();              // Core encryption logic

    // Steganography methods
    void processSteganography();
    void processSteganographyMode();            // New steganography mode setup
    void hideSteganographyData();               // New method name
    void extractSteganographyData();            // New method name

    bool isInSteganographyMode() const { return !stegMode.empty(); }

private:
    // Algorithm getters
    std::shared_ptr<CryptoAlgorithm> getAlgorithm(const std::string& name);
    std::shared_ptr<SteganographyAlgorithm> getSteganographyAlgorithm(const std::string& name);

    // New encryption helper methods
    Image applyEncryptionStep(const Image& input, const std::string& stepStr, bool isDecrypt);
    void recoverEncryptionSteps(const Image& image);
    void embedEncryptionMetadata();

    // Command line parsing
    cxxopts::Options options;
    cxxopts::ParseResult result;

    // Image data
    Image workImage;
    Image outImage;
    Image hideImage;

    // File paths
    std::string inputPath;
    std::string outputPath;
    std::string hideDataPath;

    // General flags
    bool debug = false;
    bool decrypt = false;

    // Encryption settings
    std::vector<std::string> stepsToRun;
    std::string masterPassword;

    // Steganography settings
    std::string stegMode;
    std::string stegAlgo;
    std::string stegPassword;
    std::string hiddenData;
    bool hideAsImage = false;

    // Histogram settings
    int histoBin = 10;
    int histoChartHeight = 10;

    // Algorithm registries
    std::map<std::string, std::shared_ptr<CryptoAlgorithm>> algorithms;
    std::map<std::string, std::shared_ptr<SteganographyAlgorithm>> stegAlgorithms;
};