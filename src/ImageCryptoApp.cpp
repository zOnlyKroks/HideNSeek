#include "ImageCryptoApp.h"
#include <cxxopts.hpp>
#include <iostream>

#include "crypt/impl/addbit/AddBitImageEncryptor.h"
#include "crypt/impl/bitnot/BitwiseNotImageEncryptor.h"
#include "crypt/impl/channelswap/SwapChannelsImageEncryptor.h"
#include "crypt/impl/pixelpermutation/PixelPermutationEncryptor.h"
#include "crypt/impl/xor/XORAlgorithm.h"
#include "crypt/impl/rotn/RotNImageEncryptor.h"
#include "img/ImageLoader.h"
#include "img/ImageUtils.h"

ImageCryptoApp::ImageCryptoApp()
    : options("MyProgram", "One line description of MyProgram"), workImage(), outImage() {
    options.add_options()
            ("d,debug", "Enable Debug Output")
            ("dec,decrypt", "Crypt mode")
            ("fi,inputFile", "Input File name", cxxopts::value<std::string>())
            ("fo,outputFile", "Output File name", cxxopts::value<std::string>())
            ("step,steps", "User-defined steps (e.g. xor:3)", cxxopts::value<std::vector<std::string>>())
            ("mpw,masterPassword", "Master password for embedding steps", cxxopts::value<std::string>()->default_value(""))
            ("hiBin,histoBin", "Histo Bin Width", cxxopts::value<int>())
            ("hiChH,histoChartHeight", "Histo Chart Height", cxxopts::value<int>());
}

void ImageCryptoApp::run(const int argc, char **argv) {
    result = options.parse(argc, argv);

    debug = result["debug"].as<bool>();
    std::cout << "Debug mode is " << (debug ? "enabled." : "disabled.") << std::endl;

    decrypt = result["decrypt"].as<bool>();

    if (result.count("steps")) {
        stepsToRun = result["steps"].as<std::vector<std::string>>();
        std::cout << "User-defined steps:";
        for (const auto &s : stepsToRun) std::cout << " " << s;
        std::cout << std::endl;
    } else if (!decrypt) {
        throw std::runtime_error("Error: steps must be provided.");
    }

    masterPassword = result["masterPassword"].as<std::string>();
    if (masterPassword.empty()) {
        throw std::runtime_error("Error: masterPassword required");
    }

    if (result.count("inputFile")) inputPath = result["inputFile"].as<std::string>();
    else throw std::runtime_error("Input file not specified.");

    if (result.count("outputFile")) outputPath = result["outputFile"].as<std::string>();
    else outputPath = inputPath;

    if (result.count("histoBin")) histoBin = result["histoBin"].as<int>();
    if (result.count("histoChartHeight")) histoChartHeight = result["histoChartHeight"].as<int>();

    workImage = ImageLoader::loadImage(inputPath);
    if (debug) ImageUtils::printImageInfo(workImage, "Debug Image Info");
}

void ImageCryptoApp::processImageWithAlgorithm() {
    Image current = workImage;
    Image next;

    if (decrypt && stepsToRun.empty()) {
        try {
            Image encImg = ImageUtils::extractMetadataImage(current, "enc_steps_img");
            Image decImg;

            // Use a known default algorithm to decrypt the metadata (e.g., XOR)
            auto fallbackAlgorithm = getAlgorithm("xor");
            if (!fallbackAlgorithm) throw std::runtime_error("No algorithm available to decrypt metadata.");

            fallbackAlgorithm->decrypt(encImg, decImg, masterPassword);
            std::string recovered = ImageUtils::textFromImage(decImg);
            std::cout << "Recovered steps: " << recovered << std::endl;

            // Tokenize recovered steps and populate userSteps
            std::istringstream iss(recovered);
            std::string step;
            while (iss >> step) {
                stepsToRun.push_back(step);
            }

        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Failed to recover steps from metadata: ") + e.what());
        }
    }

    if (stepsToRun.empty()) {
        throw std::runtime_error("No encryption steps provided.");
    }

    // Create a copy of steps for processing - we'll reverse them for decryption
    std::vector<std::string> processSteps = stepsToRun;

    // If we're decrypting, reverse the order of steps
    if (decrypt) {
        std::reverse(processSteps.begin(), processSteps.end());
    }

    // Process each step in the appropriate order
    for (const auto& step : processSteps) {
        std::istringstream iss(step);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(iss, token, ':')) {
            tokens.push_back(token);
        }

        if (tokens.empty()) continue;

        std::string stepAlgo = tokens[0];
        int count = 1;
        std::string customParam;

        // Parse count and customParam robustly
        if (tokens.size() == 2) {
            try {
                count = std::stoi(tokens[1]);
            } catch (const std::invalid_argument&) {
                // tokens[1] is not a number, treat as customParam
                customParam = tokens[1];
            }
        } else if (tokens.size() >= 3) {
            count = std::stoi(tokens[1]);
            customParam = tokens[2];
        }

        auto stepAlgorithm = getAlgorithm(stepAlgo);
        if (!stepAlgorithm) {
            throw std::runtime_error("Unknown step algorithm: " + stepAlgo);
        }

        for (int i = 0; i < count; ++i) {
            next = Image();

            std::string effectiveKey = masterPassword;

            // Use custom param as key override or configuration
            if (!customParam.empty()) {
                if (stepAlgo == "rotn") {
                    effectiveKey = customParam;
                }
                // For other algorithms (e.g. aes256), you can handle customParam here as needed
            }

            if (decrypt) stepAlgorithm->decrypt(current, next, effectiveKey);
            else         stepAlgorithm->encrypt(current, next, effectiveKey);

            current = next;
            std::cout << (decrypt ? "Decrypt" : "Encrypt")
                      << " step " << stepAlgo << " pass " << (i + 1) << std::endl;
        }
    }

    outImage = current;

    // Embed steps if encrypting
    if (!decrypt) {
        std::string blob;
        for (const auto& s : stepsToRun) blob += s + " ";
        Image textImg = ImageUtils::textToImage(blob);

        std::string firstAlgo = stepsToRun.front().substr(0, stepsToRun.front().find(':'));
        auto algorithm = getAlgorithm(firstAlgo);
        if (!algorithm) throw std::runtime_error("Cannot find algorithm for embedding: " + firstAlgo);

        Image encImg;
        algorithm->encrypt(textImg, encImg, masterPassword);
        ImageUtils::embedMetadataImage(outImage, "enc_steps_img", encImg);
        std::cout << "✓ Embedded encrypted steps-image into metadata." << std::endl;
    }

    ImageLoader::saveImage(outputPath, outImage, false);
}

void ImageCryptoApp::registerAlgorithms() {
    const auto xorAlgo = std::make_shared<XORImageEncryptor>();
    algorithms[xorAlgo->name()] = xorAlgo;

    const auto rotnAlgo = std::make_shared<RotNImageEncryptor>();
    algorithms[rotnAlgo->name()] = rotnAlgo;

    const auto bitNot = std::make_shared<BitwiseNotImageEncryptor>();
    algorithms[bitNot->name()] = bitNot;

    const auto addBit = std::make_shared<AddBitImageEncryptor>();
    algorithms[addBit->name()] = addBit;

    const auto channelSwap = std::make_shared<SwapChannelsImageEncryptor>();
    algorithms[channelSwap->name()] = channelSwap;

    const auto pixelPermutation = std::make_shared<PixelPermutationEncryptor>();
    algorithms[pixelPermutation->name()] = pixelPermutation;
}

std::shared_ptr<CryptoAlgorithm> ImageCryptoApp::getAlgorithm(const std::string& name) {
    const auto it = algorithms.find(name);
    return (it != algorithms.end() ? it->second : nullptr);
}
