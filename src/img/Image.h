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

    [[nodiscard]] unsigned char& getPixel(int x, int y, int channel = 0) {
        return pixels[((y * width) + x) * channels + channel];
    }

    [[nodiscard]] unsigned char getPixel(int x, int y, int channel = 0) const {
        return pixels[((y * width) + x) * channels + channel];
    }

    // Adds a metadata entry
    void addMetadata(const std::string& key, const std::string& value) {
        meta[key] = value;
    }

    // Gets a metadata entry if it exists, empty string otherwise
    [[nodiscard]] std::string getMetadataValue(const std::string& key) const {
        auto it = meta.find(key);
        return (it != meta.end()) ? it->second : "";
    }

    // Check if metadata key exists
    [[nodiscard]] bool hasMetadata(const std::string& key) const {
        return meta.find(key) != meta.end();
    }

    // Remove a metadata entry
    void removeMetadata(const std::string& key) {
        meta.erase(key);
    }

    // Clear all metadata
    void clearMetadata() {
        meta.clear();
    }

    explicit Image(int w = 0, int h = 0, int c = 1)
      : width(w), height(h), channels(c),
        pixels(static_cast<size_t>(w) * h * c, 0) {}

private:
    std::map<std::string, std::string> meta;
};