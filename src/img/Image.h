#pragma once

#include <vector>
#include <string>
#include <map>

struct Image {
    int width;
    int height;
    int channels;
    std::vector<unsigned char> pixels;

    std::map<std::string, std::string>& metadata() {
        return meta;
    }
    [[nodiscard]] const std::map<std::string, std::string>& metadata() const {
        return meta;
    }

    [[nodiscard]] int getWidth() const { return width; }
    [[nodiscard]] int getHeight() const { return height; }

    [[nodiscard]] unsigned char& getPixel(const int x, const int y, const int channel = 0) {
        return pixels[((y * width) + x) * channels + channel];
    }

    [[nodiscard]] unsigned char getPixel(const int x, const int y, const int channel = 0) const {
        return pixels[((y * width) + x) * channels + channel];
    }

    void setPixel(const int x, const int y, const int c, const int px) {
        pixels[((y * width) + x) * channels + c] = static_cast<unsigned char>(px);
    }

    [[nodiscard]] std::vector<unsigned char> getPixels() const {
        return pixels;
    }

    void setPixels(const std::vector<unsigned char>& toSet) {
        pixels = toSet;
    }

    // Adds a metadata entry
    void addMetadata(const std::string& key, const std::string& value) {
        meta[key] = value;
    }

    // Gets a metadata entry if it exists, empty string otherwise
    [[nodiscard]] std::string getMetadataValue(const std::string& key) const {
        const auto it = meta.find(key);
        return (it != meta.end()) ? it->second : "";
    }

    // Check if metadata key exists
    [[nodiscard]] bool hasMetadata(const std::string& key) const {
        return meta.contains(key);
    }

    // Remove a metadata entry
    void removeMetadata(const std::string& key) {
        meta.erase(key);
    }

    // Clear all metadata
    void clearMetadata() {
        meta.clear();
    }

    explicit Image(const int w = 0, const int h = 0, const int c = 1)
      : width(w), height(h), channels(c),
        pixels(static_cast<size_t>(w) * h * c, 0) {}

private:
    std::map<std::string, std::string> meta;
};