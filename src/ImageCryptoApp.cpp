#include "ImageCryptoApp.h"
#include <cxxopts.hpp>
#include <iostream>
#include <sstream>
#include <fstream>

#include "crypt/impl/addbit/AddBitImageEncryptor.h"
#include "crypt/impl/bitnot/BitwiseNotImageEncryptor.h"
#include "crypt/impl/channelswap/SwapChannelsImageEncryptor.h"
#include "crypt/impl/pixelpermutation/PixelPermutationEncryptor.h"
#include "crypt/impl/rotn/RotNImageEncryptor.h"
#include "crypt/impl/xor/XORAlgorithm.h"
#include "img/ImageLoader.h"
#include "img/ImageUtils.h"
#include "steno/impl/LSBSteganography.h"
#include "steno/impl/pvd/PVDSteganography.h"

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
        ("hiChH,histoChartHeight", "Histo Chart Height", cxxopts::value<int>())
        ("steg", "Steganography mode (hide or extract)", cxxopts::value<std::string>())
        ("algo", "Steganography algorithm to use (lsb)", cxxopts::value<std::string>())
        ("data", "Data to hide or output file", cxxopts::value<std::string>())
        ("pass", "Password for steganography", cxxopts::value<std::string>()->default_value(""))
        ("image", "Treat data as image for steganography", cxxopts::value<bool>()->default_value("false"));
}

void ImageCryptoApp::run(const int argc, char **argv) {
    result = options.parse(argc, argv);

    debug = result["debug"].as<bool>();
    std::cout << "Debug mode is " << (debug ? "enabled." : "disabled.") << std::endl;

    if (result.count("steg")) {
        stegMode = result["steg"].as<std::string>();
        stegAlgo = result["algo"].as<std::string>();
        stegPassword = result["pass"].as<std::string>();
        hideAsImage = result["image"].as<bool>();

        if (result.count("inputFile")) inputPath = result["inputFile"].as<std::string>();
        else throw std::runtime_error("Input file not specified.");
        if (result.count("outputFile")) outputPath = result["outputFile"].as<std::string>();
        else outputPath = inputPath + ".out";

        if (result.count("data")) hiddenData = result["data"].as<std::string>();
        else if (stegMode == "hide") throw std::runtime_error("Data to hide must be specified.");

        workImage = ImageLoader::loadImage(inputPath);
        if (debug) ImageUtils::printImageInfo(workImage, "Debug Image Info");
        return;
    }

    // Encryption/decryption path
    decrypt = result["decrypt"].as<bool>();

    if (result.count("steps")) {
        stepsToRun = result["steps"].as<std::vector<std::string>>();
    }

    masterPassword = result["masterPassword"].as<std::string>();
    if (masterPassword.empty()) throw std::runtime_error("Error: masterPassword required");

    if (result.count("inputFile")) inputPath = result["inputFile"].as<std::string>();
    else throw std::runtime_error("Input file not specified.");

    if (result.count("outputFile")) outputPath = result["outputFile"].as<std::string>();
    else outputPath = inputPath;

    if (result.count("histoBin")) histoBin = result["histoBin"].as<int>();
    if (result.count("histoChartHeight")) histoChartHeight = result["histoChartHeight"].as<int>();

    workImage = ImageLoader::loadImage(inputPath);
    if (debug) ImageUtils::printImageInfo(workImage, "Debug Image Info");
}

void ImageCryptoApp::processSteganography() {
    if (stegMode == "hide") hide();
    else if (stegMode == "extract") extract();
    else throw std::runtime_error("Invalid steg mode. Use 'hide' or 'extract'.");
}


void ImageCryptoApp::registerAlgorithms() {
    const auto add = std::make_shared<AddBitImageEncryptor>();
    const auto xorAlgo = std::make_shared<XORImageEncryptor>();
    const auto rotn = std::make_shared<RotNImageEncryptor>();
    const auto bitnot = std::make_shared<BitwiseNotImageEncryptor>();
    const auto swap = std::make_shared<SwapChannelsImageEncryptor>();
    const auto perm = std::make_shared<PixelPermutationEncryptor>();

    const auto lsb = std::make_shared<LSBSteganography>(3);
    const auto pvd = std::make_shared<PVDSteganography>();

    algorithms[add->name()] = add;
    algorithms[xorAlgo->name()] = xorAlgo;
    algorithms[rotn->name()] = rotn;
    algorithms[bitnot->name()] = bitnot;
    algorithms[swap->name()] = swap;
    algorithms[perm->name()] = perm;

    stegAlgorithms[lsb->name()] = lsb;
    stegAlgorithms[pvd->name()] = pvd;
}

void ImageCryptoApp::processImageWithAlgorithm() {
    Image current = workImage;
    Image next;

    if (decrypt && stepsToRun.empty()) {
        try {
            Image encImg = ImageUtils::extractMetadataImage(current, "enc_steps_img");
            Image decImg;
            auto fallback = getAlgorithm("xor");
            if (!fallback) throw std::runtime_error("Fallback algorithm missing");
            fallback->decrypt(encImg, decImg, masterPassword);
            std::string recovered = ImageUtils::textFromImage(decImg);
            std::cout << "Recovered steps: " << recovered << std::endl;

            std::istringstream iss(recovered);
            std::string step;
            while (iss >> step) stepsToRun.push_back(step);
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Failed to recover steps: ") + e.what());
        }
    }

    if (stepsToRun.empty()) throw std::runtime_error("No encryption steps provided.");

    std::vector<std::string> steps = stepsToRun;
    if (decrypt) std::ranges::reverse(steps);

    for (const auto& step : steps) {
        std::istringstream iss(step);
        std::string token;
        std::vector<std::string> tokens;

        while (std::getline(iss, token, ':')) tokens.push_back(token);

        if (tokens.empty()) continue;

        std::string algo = tokens[0];
        int count = 1;
        std::string param;

        if (tokens.size() == 2) {
            try {
                count = std::stoi(tokens[1]);
            } catch (...) {
                param = tokens[1];
            }
        } else if (tokens.size() >= 3) {
            count = std::stoi(tokens[1]);
            param = tokens[2];
        }

        auto algorithm = getAlgorithm(algo);
        if (!algorithm) throw std::runtime_error("Unknown step: " + algo);

        for (int i = 0; i < count; ++i) {
            next = Image();
            std::string key = param.empty() ? masterPassword : param;
            if (decrypt) algorithm->decrypt(current, next, key);
            else algorithm->encrypt(current, next, key);
            current = next;
        }
    }

    outImage = current;

    if (!decrypt) {
        std::string blob;
        for (const auto& s : stepsToRun) blob += s + " ";
        Image textImg = ImageUtils::textToImage(blob);
        Image encImg;
        auto algo = getAlgorithm(stepsToRun.front().substr(0, stepsToRun.front().find(':')));
        if (!algo) throw std::runtime_error("Missing algorithm for metadata embed.");
        algo->encrypt(textImg, encImg, masterPassword);
        ImageUtils::embedMetadataImage(outImage, "enc_steps_img", encImg);
    }

    ImageLoader::saveImage(outputPath, outImage, false);
}

void ImageCryptoApp::hide() {
    auto steg = getSteganographyAlgorithm(stegAlgo);
    if (!steg) {
        throw std::runtime_error("Steganography algorithm not found: " + stegAlgo);
    }

    std::cout << "Hiding data using " << stegAlgo << " algorithm..." << std::endl;

    if (hideAsImage) {
        hideImage = ImageLoader::loadImage(hiddenData);
        if (debug) ImageUtils::printImageInfo(hideImage, "Debug Hide Image Info");

        if (!steg->canEmbedData(workImage, hideImage, stegPassword)) {
            throw std::runtime_error("Steganography failed: Image is too small to hide data.");
        }

        std::cout << "Image is large enough to hide data." << std::endl;

        if (bool success = steg->hideImage(workImage, hideImage, outImage, stegPassword); !success) {
            throw std::runtime_error("Steganography failed: Could not hide image.");
        }
    } else {
        std::ifstream file(hiddenData);
        std::string dataToHide;

        if (file.good()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            dataToHide = buffer.str();
            std::cout << "Loaded data from file: " << hiddenData << " (" << dataToHide.size() << " bytes)" << std::endl;
        } else {
            dataToHide = hiddenData;
            std::cout << "Using provided string data (" << dataToHide.size() << " bytes)" << std::endl;
        }

        bool success = steg->hideData(workImage, dataToHide, outImage, stegPassword);
        if (!success) {
            throw std::runtime_error("Steganography failed: Could not hide data.");
        }
    }

    ImageLoader::saveImage(outputPath, outImage, false);
    std::cout << "Steganographic image saved to: " << outputPath << std::endl;
}

void ImageCryptoApp::extract() {
    const auto steg = getSteganographyAlgorithm(stegAlgo);
    if (!steg) {
        throw std::runtime_error("Steganography algorithm not found: " + stegAlgo);
    }

    std::cout << "Extracting data using " << stegAlgo << " algorithm..." << std::endl;

    if (hideAsImage) {
        bool success = steg->extractImage(workImage, outImage, stegPassword);
        if (!success || outImage.pixels.empty()) {
            throw std::runtime_error("Steganography extraction failed: No image could be recovered. "
                                     "Make sure the image actually contains hidden data and the password is correct.");
        }

        ImageLoader::saveImage(outputPath, outImage, false);
        std::cout << "Extracted image saved to: " << outputPath << std::endl;
    } else {
        std::string extracted;
        bool success = steg->extractData(workImage, extracted, stegPassword);
        if (!success) {
            throw std::runtime_error("Steganography extraction failed: No data could be recovered.");
        }

        if (outputPath == inputPath + ".out") {
            std::cout << "Extracted data:" << std::endl;
            std::cout << extracted << std::endl;
        } else {
            std::ofstream outFile(outputPath);
            if (!outFile.good()) {
                throw std::runtime_error("Cannot write to output file: " + outputPath);
            }
            outFile << extracted;
            outFile.close();
            std::cout << "Extracted data saved to: " << outputPath << std::endl;
        }
    }
}

std::shared_ptr<CryptoAlgorithm> ImageCryptoApp::getAlgorithm(const std::string& name) {
    const auto it = algorithms.find(name);
    return it != algorithms.end() ? it->second : nullptr;
}

std::shared_ptr<SteganographyAlgorithm> ImageCryptoApp::getSteganographyAlgorithm(const std::string& name) {
    const auto it = stegAlgorithms.find(name);
    return it != stegAlgorithms.end() ? it->second : nullptr;
}
