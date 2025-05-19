//
// Created by Finn Rades on 19.05.25.
//

#include "ImageCryptoApp.h"
#include <cxxopts.hpp>

#include "crypt/impl/XORAlgorithm.h"
#include "img/ImageLoader.h"
#include "img/ImageUtils.h"

ImageCryptoApp::ImageCryptoApp()
    : options("MyProgram", "One line description of MyProgram") {
    options.add_options()
    ("d,debug", "Enable Debug Output")
    ("dec,decrypt", "Crypt mode")
    ("fi,inputFile", "Input File name", cxxopts::value<std::string>())
    ("fo,outputFile", "Output File name", cxxopts::value<std::string>())
    ("ecAlg,encryptAlgorithm", "Encrypt algorithm to use", cxxopts::value<std::string>())
    ("hiBin,histoBin", "Histo Bin Width", cxxopts::value<int>())
    ("hiChH,histoChartHeight", "Histo Chart Height", cxxopts::value<int>());
}

void ImageCryptoApp::run(const int argc, char **argv) {
    result = options.parse(argc, argv);

    if (result["debug"].as<bool>()) {
        std::cout << "Debug mode is enabled." << std::endl;
        debug = true;
    } else {
        std::cout << "Debug mode is disabled." << std::endl;
    }

    if (result["decrypt"].as<bool>()) {
        decrypt = true;
    }

    if (result.count("inputFile")) {
        inputPath = result["inputFile"].as<std::string>();
        std::cout << "Input file: " << inputPath << std::endl;
    } else {
        std::cerr << "Input file not specified." << std::endl;
        throw std::runtime_error("Input file not specified.");
    }

    if (result.count("outputFile")) {
        outputPath = result["outputFile"].as<std::string>();
        std::cout << "Output file: " << outputPath << std::endl;
    } else {
        outputPath = inputPath;
        std::cout << "Output file not specified, using input path as output!" << std::endl;
    }

    if (result.count("histoBin")) {
        histoBin = result["histoBin"].as<int>();
        std::cout << "Histo Bin Width: " << histoBin << std::endl;
    }

    if (result.count("histoChartHeight")) {
        histoChartHeight = result["histoChartHeight"].as<int>();
        std::cout << "Histo Chart Height: " << histoChartHeight << std::endl;
    }

    workImage = ImageLoader::loadImage(inputPath);

    if (debug) {
        ImageUtils::printImageInfo(workImage, "Debug Image Info");
        ImageUtils::printHistogram(workImage, histoBin, histoChartHeight);
    }
}

void ImageCryptoApp::processImageWithAlgorithm() {
    if (!result.count("encryptAlgorithm")) {
        std::cerr << "No encryption algorithm specified." << std::endl;
        throw std::runtime_error("No encryption algorithm specified.");
    }

    const std::string algoName = result["encryptAlgorithm"].as<std::string>();
    const std::string key = "cannotBeEmptyMyAss"; // TODO: replace with CLI key param

    auto algorithm = getAlgorithm(algoName);
    if (!algorithm) {
        std::cerr << "Unknown algorithm: " << algoName << std::endl;
        throw std::runtime_error("Unknown algorithm: " + algoName);
    }

    std::cout << "Using algorithm: " << algorithm->name() << std::endl;

    if (decrypt) {
        std::cout << "Decrypting image..." << std::endl;
        algorithm->decrypt(workImage, outImage, key);
    } else {
        std::cout << "Encrypting image..." << std::endl;
        algorithm->encrypt(workImage, outImage, key);
    }

    ImageLoader::saveImage(outputPath, outImage, false);
}

void ImageCryptoApp::registerAlgorithms() {
    const auto xorAlgo = std::make_shared<XORImageEncryptor>();
    algorithms[xorAlgo->name()] = xorAlgo;
}

std::shared_ptr<CryptoAlgorithm> ImageCryptoApp::getAlgorithm(const std::string& name) {
    const auto it = algorithms.find(name);
    return (it != algorithms.end() ? it->second : nullptr);
}
