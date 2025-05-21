#include "ImageUtils.h"
#include "./util/Base64.h"

#include <iostream>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace ImageUtils {

    void printImageInfo(const Image& img, const std::string& name) {
        std::cout << "Image Info";
        if (!name.empty()) std::cout << " [" << name << "]";
        std::cout << ": " << img.width << "x" << img.height
                  << ", channels=" << img.channels
                  << ", metadata=" << img.metadata().size() << "\n";

        // Print metadata keys if available
        if (!img.metadata().empty()) {
            std::cout << "  Metadata keys:";
            for (const auto &key: img.metadata() | std::views::keys) {
                std::cout << " " << key;
            }
            std::cout << "\n";
        }
    }

    Image convertTo3Channels(const Image& src) {
        if (src.channels == 3) return src;
        if (src.channels != 1 && src.channels != 4) {
            throw std::runtime_error("Unsupported channel count: " + std::to_string(src.channels));
        }

        Image result;
        result.width = src.width;
        result.height = src.height;
        result.channels = 3;
        result.pixels.resize(src.width * src.height * 3);

        if (src.channels == 1) {
            // Convert grayscale to RGB
            for (int i = 0; i < src.width * src.height; ++i) {
                unsigned char gray = src.pixels[i];
                result.pixels[i * 3 + 0] = gray;
                result.pixels[i * 3 + 1] = gray;
                result.pixels[i * 3 + 2] = gray;
            }
        } else if (src.channels == 4) {
            // Convert RGBA to RGB
            for (int i = 0; i < src.width * src.height; ++i) {
                result.pixels[i * 3 + 0] = src.pixels[i * 4 + 0]; // R
                result.pixels[i * 3 + 1] = src.pixels[i * 4 + 1]; // G
                result.pixels[i * 3 + 2] = src.pixels[i * 4 + 2]; // B
            }
        }

        // Copy metadata
        for (const auto& [key, value] : src.metadata()) {
            result.metadata()[key] = value;
        }

        return result;
    }

    void embedMetadataImage(Image& targetImage, const std::string& key, const Image& dataImage) {
        // Serialize the image data to store in metadata
        std::stringstream ss;

        // Store dimensions and format
        ss << dataImage.width << "," << dataImage.height << "," << dataImage.channels << ":";

        // Encode the pixel data in base64
        std::string pixelData(
            reinterpret_cast<const char*>(dataImage.pixels.data()),
            dataImage.pixels.size()
        );
        std::string encodedData = Base64::encodeString(pixelData);
        ss << encodedData;

        // Store metadata if available
        if (!dataImage.metadata().empty()) {
            ss << "!META!";
            for (const auto& [metaKey, metaValue] : dataImage.metadata()) {
                std::string encodedKey = Base64::encodeString(metaKey);
                std::string encodedValue = Base64::encodeString(metaValue);
                ss << encodedKey << ":" << encodedValue << ";";
            }
        }

        // Store in target image metadata
        targetImage.metadata()[key] = ss.str();
    }

    Image extractMetadataImage(const Image& sourceImage, const std::string& key) {
        auto it = sourceImage.metadata().find(key);
        if (it == sourceImage.metadata().end()) {
            throw std::runtime_error("Metadata key not found: " + key);
        }

        const std::string& data = it->second;
        std::stringstream ss(data);
        std::string header, encodedPixels, metadataSection;

        // Parse header
        std::getline(ss, header, ':');
        std::stringstream headerSS(header);
        std::string widthStr, heightStr, channelsStr;
        std::getline(headerSS, widthStr, ',');
        std::getline(headerSS, heightStr, ',');
        std::getline(headerSS, channelsStr, ',');

        int width = std::stoi(widthStr);
        int height = std::stoi(heightStr);
        int channels = std::stoi(channelsStr);

        // Get remaining content (pixels + metadata)
        std::string remaining;
        std::getline(ss, remaining);

        // Split pixel data from metadata if present
        if (size_t metaPos = remaining.find("!META!"); metaPos != std::string::npos) {
            encodedPixels = remaining.substr(0, metaPos);
            metadataSection = remaining.substr(metaPos + 6); // Skip '!META!'
        } else {
            encodedPixels = remaining;
        }

        // Decode pixel data
        std::string decodedData = Base64::decodeToString(encodedPixels);

        // Create result image
        Image result(width, height, channels);
        result.pixels.assign(
            reinterpret_cast<const unsigned char*>(decodedData.data()),
            reinterpret_cast<const unsigned char*>(decodedData.data() + decodedData.size())
        );

        // Process metadata if present
        if (!metadataSection.empty()) {
            std::stringstream metaSS(metadataSection);
            std::string metaPair;
            while (std::getline(metaSS, metaPair, ';')) {
                if (metaPair.empty()) continue;

                if (size_t colonPos = metaPair.find(':'); colonPos != std::string::npos) {
                    std::string encodedKey = metaPair.substr(0, colonPos);
                    std::string encodedValue = metaPair.substr(colonPos + 1);

                    std::string decodedKey = Base64::decodeToString(encodedKey);
                    std::string decodedValue = Base64::decodeToString(encodedValue);

                    result.metadata()[decodedKey] = decodedValue;
                }
            }
        }

        return result;
    }

    Image textToImage(const std::string& text) {
        const int width = static_cast<int>(text.length());
        constexpr int height = 1;

        Image result(width, height, 1);

        for (int i = 0; i < width; ++i) {
            result.pixels[i] = static_cast<unsigned char>(text[i]);
        }

        result.metadata()["TEXT"] = "1";

        return result;
    }

    std::string textFromImage(const Image& img) {
        if (img.height == 0 || img.width == 0) {
            return "";
        }

        std::string result;
        result.reserve(img.width * img.height);

        for (int i = 0; i < img.width * img.height; ++i) {
            if (const char c = static_cast<char>(img.pixels[i]); c >= 32 && c <= 126) {
                result.push_back(c);
            }
        }

        return result;
    }

    std::vector<unsigned char> serializeImage(const Image& img) {
        std::vector<unsigned char> data;

        auto appendUint32 = [&](const uint32_t val) {
            for (int i = 0; i < 4; ++i)
                data.push_back((val >> (i * 8)) & 0xFF);
        };

        appendUint32(img.getWidth());
        appendUint32(img.getHeight());
        appendUint32(img.channels);

        // Append pixel data
        const auto& pixels = img.getPixels();
        data.insert(data.end(), pixels.begin(), pixels.end());

        return data;
    }

    bool deserializeImage(const std::vector<unsigned char>& data, Image& img) {
        if (data.size() < 12)  // must at least contain width,height,channels
            return false;

        auto readUint32 = [&](const size_t offset) -> uint32_t {
            uint32_t val = 0;
            for (int i = 0; i < 4; ++i) {
                val |= static_cast<uint32_t>(data[offset + i]) << (i * 8);
            }
            return val;
        };

        const uint32_t width = readUint32(0);
        const uint32_t height = readUint32(4);
        const uint32_t channels = readUint32(8);

        constexpr size_t pixelDataStart = 12;
        const size_t pixelDataSize = width * height * channels;

        if (data.size() < pixelDataStart + pixelDataSize)
            return false;

        img = Image(width, height, channels);
        std::copy_n(data.begin() + pixelDataStart, pixelDataSize, img.pixels.begin());

        return true;
    }
}