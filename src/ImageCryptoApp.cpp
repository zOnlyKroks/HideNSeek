#include "ImageCryptoApp.h"
#include <cxxopts.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <QDebug>

#include "crypt/impl/addbit/AddBitImageEncryptor.h"
#include "crypt/impl/aes256/AES256ImageEncryptor.h"
#include "crypt/impl/bitnot/BitwiseNotImageEncryptor.h"
#include "crypt/impl/channelswap/SwapChannelsImageEncryptor.h"
#include "crypt/impl/pixelpermutation/PixelPermutationEncryptor.h"
#include "crypt/impl/rotn/RotNImageEncryptor.h"
#include "crypt/impl/xor/XORAlgorithm.h"
#include "img/ImageLoader.h"
#include "img/ImageUtils.h"
#include "steno/impl/lsb/LSBSteganography.h"

void ImageCryptoApp::log(const std::string& message) const {
    if (logFunction) {
        logFunction(message);
    } else {
        qDebug() << message.c_str();
    }
}

ImageCryptoApp::ImageCryptoApp()
    : options("HideNSeek", "Image encryption and steganography tool") {

    options.add_options()
        ("d,debug", "Enable debug output")
        ("dec,decrypt", "Decrypt mode (default: encrypt)")
        ("fi,inputFile", "Input image file", cxxopts::value<std::string>())
        ("fo,outputFile", "Output image file", cxxopts::value<std::string>())
        ("step,steps", "Encryption steps (e.g. aes256:1)", cxxopts::value<std::vector<std::string>>())
        ("mpw,masterPassword", "Master password", cxxopts::value<std::string>()->default_value(""))
        // Steganography options
        ("steg", "Steganography mode (hide|extract)", cxxopts::value<std::string>())
        ("algo", "Steganography algorithm (lsb|pvd)", cxxopts::value<std::string>())
        ("data", "Data to hide or extract to", cxxopts::value<std::string>())
        ("pass", "Steganography password", cxxopts::value<std::string>()->default_value(""))
        ("image", "Treat data as image", cxxopts::value<bool>());
}

void ImageCryptoApp::run(const int argc, char** argv) {
    try {
        result = options.parse(argc, argv);

        debug = result["debug"].as<bool>();
        if (debug) {
            log("ImageCryptoApp starting.");
            log("Debug mode is enabled.");
        }

        registerAlgorithms();

        if (result.count("steg")) {
            processSteganographyMode();
            return;
        }

        processEncryptionMode();

    } catch (const std::exception& e) {
        log("Error: " + std::string(e.what()));
        throw;
    }
}

void ImageCryptoApp::registerAlgorithms() {
    algorithms["addbit"] = std::make_shared<AddBitImageEncryptor>();
    algorithms["xor"] = std::make_shared<XORImageEncryptor>();
    algorithms["rotn"] = std::make_shared<RotNImageEncryptor>();
    algorithms["bitnot"] = std::make_shared<BitwiseNotImageEncryptor>();
    algorithms["channelswap"] = std::make_shared<SwapChannelsImageEncryptor>();
    algorithms["pixelperm"] = std::make_shared<PixelPermutationEncryptor>();
    algorithms["aes256"] = std::make_shared<AES256ImageEncryptor>();

    stegAlgorithms["lsb"] = std::make_shared<LSBSteganography>(3);
    stegAlgorithms["pvd"] = std::make_shared<LSBSteganography>(4);
}

void ImageCryptoApp::processSteganographyMode() {
    stegMode = result["steg"].as<std::string>();
    stegAlgo = result.count("algo") ? result["algo"].as<std::string>() : "lsb";
    stegPassword = result["pass"].as<std::string>();
    hideAsImage = result["image"].as<bool>();

    if (!result.count("inputFile")) {
        throw std::runtime_error("Input file required for steganography");
    }
    inputPath = result["inputFile"].as<std::string>();
    outputPath = result.count("outputFile") ? result["outputFile"].as<std::string>() : inputPath + ".out";

    if (stegMode == "hide" && !result.count("data")) {
        throw std::runtime_error("Data to hide must be specified");
    }
    if (result.count("data")) {
        hiddenData = result["data"].as<std::string>();
    }

    workImage = ImageLoader::loadImage(inputPath);
    if (debug) ImageUtils::printImageInfo(workImage, "Input Image");

    processSteganography();
}

void ImageCryptoApp::processEncryptionMode() {
    decrypt = result["decrypt"].as<bool>();

    masterPassword = result["masterPassword"].as<std::string>();
    if (masterPassword.empty()) {
        throw std::runtime_error("Master password is required");
    }

    if (!result.count("inputFile")) {
        throw std::runtime_error("Input file is required");
    }
    inputPath = result["inputFile"].as<std::string>();
    outputPath = result.count("outputFile") ? result["outputFile"].as<std::string>() : inputPath + ".processed";

    if (result.count("steps")) {
        stepsToRun = result["steps"].as<std::vector<std::string>>();
    }

    workImage = ImageLoader::loadImage(inputPath);
    if (debug) ImageUtils::printImageInfo(workImage, "Debug Image Info");

    processImageEncryption();
}

void ImageCryptoApp::processImageEncryption() {
    Image currentImage = workImage;

    if (decrypt && stepsToRun.empty()) {
        recoverEncryptionSteps(currentImage);
    }

    if (stepsToRun.empty()) {
        throw std::runtime_error("No encryption steps specified");
    }

    std::vector<std::string> steps = stepsToRun;
    if (decrypt) {
        std::ranges::reverse(steps);
    }

    for (const auto& stepStr : steps) {
        currentImage = applyEncryptionStep(currentImage, stepStr, decrypt);
    }

    outImage = currentImage;

    if (!decrypt) {
        embedEncryptionMetadata();
    }

    ImageLoader::saveImage(outputPath, outImage, false);

    if (debug) {
        log("Process completed. Output saved to: " + outputPath);
    }
}

Image ImageCryptoApp::applyEncryptionStep(const Image& input, const std::string& stepStr, bool isDecrypt) {
    std::vector<std::string> tokens;
    std::istringstream iss(stepStr);
    std::string token;

    while (std::getline(iss, token, ':')) {
        tokens.push_back(token);
    }

    if (tokens.empty()) {
        throw std::runtime_error("Invalid step format: " + stepStr);
    }

    std::string algoName = tokens[0];
    int count = 1;
    std::string param;

    if (tokens.size() >= 2) {
        try {
            count = std::stoi(tokens[1]);
        } catch (...) {
            count = 1;
            param = tokens[1];
        }
    }

    if (tokens.size() >= 3) {
        param = tokens[2];
    }

    auto algorithm = getAlgorithm(algoName);
    if (!algorithm) {
        throw std::runtime_error("Unknown algorithm: " + algoName);
    }

    Image current = input;

    for (int i = 0; i < count; ++i) {
        Image next;
        std::string key = param.empty() ? masterPassword : param;

        if (debug) {
            log("Step: " + algoName + " (" + std::to_string(i + 1) + "/" + std::to_string(count) + ")");
        }

        if (isDecrypt) {
            algorithm->decrypt(current, next, key);
        } else {
            algorithm->encrypt(current, next, key);
        }

        if (next.width != current.width || next.height != current.height || next.channels != current.channels) {
            throw std::runtime_error("Algorithm " + algoName + " corrupted image dimensions");
        }

        current = std::move(next);
    }

    return current;
}

void ImageCryptoApp::recoverEncryptionSteps(const Image& image) {
    try {
        if (debug) {
            log("Attempting to recover encryption steps from metadata...");
        }

        Image encImg = ImageUtils::extractMetadataImage(image, "enc_steps_img");

        auto xorAlgo = getAlgorithm("xor");
        if (!xorAlgo) {
            throw std::runtime_error("XOR algorithm not available for metadata decryption");
        }

        Image decImg;
        xorAlgo->decrypt(encImg, decImg, masterPassword);

        std::string recovered = ImageUtils::textFromImage(decImg);

        if (debug) {
            log("Recovered steps: " + recovered);
        }

        // Parse steps
        std::istringstream iss(recovered);
        std::string step;
        stepsToRun.clear();

        while (iss >> step) {
            stepsToRun.push_back(step);
        }

        if (stepsToRun.empty()) {
            throw std::runtime_error("No valid steps recovered from metadata");
        }

    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to recover encryption steps: " + std::string(e.what()));
    }
}

void ImageCryptoApp::embedEncryptionMetadata() {
    try {
        std::string stepsStr;
        for (const auto& step : stepsToRun) {
            stepsStr += step + " ";
        }

        if (stepsStr.empty()) return;

        if (debug) {
            log("Embedding metadata: " + stepsStr);
        }

        const Image textImg = ImageUtils::textToImage(stepsStr);
        Image encImg;

        const auto xorAlgo = getAlgorithm("xor");
        if (!xorAlgo) {
            throw std::runtime_error("XOR algorithm not available for metadata encryption");
        }

        xorAlgo->encrypt(textImg, encImg, masterPassword);

        ImageUtils::embedMetadataImage(outImage, "enc_steps_img", encImg);
    } catch (const std::exception& e) {
        log("Warning: Failed to embed metadata: " + std::string(e.what()));
    }
}

void ImageCryptoApp::processSteganography() {
    if (stegMode == "hide") {
        hideSteganographyData();
    } else if (stegMode == "extract") {
        extractSteganographyData();
    } else {
        throw std::runtime_error("Invalid steganography mode. Use 'hide' or 'extract'");
    }
}

void ImageCryptoApp::hideSteganographyData() {
    auto steg = getSteganographyAlgorithm(stegAlgo);
    if (!steg) {
        throw std::runtime_error("Steganography algorithm not found: " + stegAlgo);
    }

    if (debug) {
        log("Hiding data using " + stegAlgo + " algorithm...");
    }

    bool success = false;

    if (hideAsImage) {
        hideImage = ImageLoader::loadImage(hiddenData);
        if (debug) ImageUtils::printImageInfo(hideImage, "Data Image to Hide");

        const auto res = steg->canEmbedData(workImage, hideImage, stegPassword);

        log("Can hide image: " + std::to_string(std::get<0>(res)));
        log("Data size: " + std::to_string(std::get<1>(res)));
        log("Carrier Image capacity: " + std::to_string(std::get<2>(res)));

        if (!std::get<0>(res)) {
            throw std::runtime_error("Image too small to hide the specified data");
        }

        success = steg->hideImage(workImage, hideImage, outImage, stegPassword);
    } else {
        std::string dataToHide;

        if (std::ifstream file(hiddenData); file.good()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            dataToHide = buffer.str();
            if (debug) {
                log("Loaded data from file: " + hiddenData + " (" + std::to_string(dataToHide.size()) + " bytes)");
            }
        } else {
            dataToHide = hiddenData;
            if (debug) {
                log("Using literal string data (" + std::to_string(dataToHide.size()) + " bytes)");
            }
        }

        success = steg->hideData(workImage, dataToHide, outImage, stegPassword);
    }

    if (!success) {
        throw std::runtime_error("Steganography failed: Could not hide data");
    }

    ImageLoader::saveImage(outputPath, outImage, false);
    if (debug) {
        log("Steganographic image saved to: " + outputPath);
    }
}

void ImageCryptoApp::extractSteganographyData() {
    const auto steg = getSteganographyAlgorithm(stegAlgo);
    if (!steg) {
        throw std::runtime_error("Steganography algorithm not found: " + stegAlgo);
    }

    if (debug) {
        log("Extracting data using " + stegAlgo + " algorithm...");
    }

    bool success = false;

    if (hideAsImage) {
        success = steg->extractImage(workImage, outImage, stegPassword);
        if (!success || outImage.pixels.empty()) {
            throw std::runtime_error("No hidden image could be extracted");
        }

        ImageLoader::saveImage(outputPath, outImage, false);
        if (debug) {
            log("Extracted image saved to: " + outputPath);
        }
    } else {
        std::string extracted;
        success = steg->extractData(workImage, extracted, stegPassword);
        if (!success) {
            throw std::runtime_error("No hidden data could be extracted");
        }

        if (outputPath == inputPath + ".out") {
            log("Extracted data:");
            log(extracted);
        } else {
            std::ofstream outFile(outputPath);
            if (!outFile.good()) {
                throw std::runtime_error("Cannot write to output file: " + outputPath);
            }
            outFile << extracted;
            if (debug) {
                log("Extracted data saved to: " + outputPath);
            }
        }
    }
}

std::shared_ptr<CryptoAlgorithm> ImageCryptoApp::getAlgorithm(const std::string& name) {
    const auto it = algorithms.find(name);
    return (it != algorithms.end()) ? it->second : nullptr;
}

std::shared_ptr<SteganographyAlgorithm> ImageCryptoApp::getSteganographyAlgorithm(const std::string& name) {
    const auto it = stegAlgorithms.find(name);
    return (it != stegAlgorithms.end()) ? it->second : nullptr;
}