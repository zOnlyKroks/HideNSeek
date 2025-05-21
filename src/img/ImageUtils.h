#pragma once

#include "Image.h"
#include <string>

namespace ImageUtils {
    /**
     * Prints information about an image to the console
     *
     * @param img The image to print information about
     * @param name Optional name to include in the output
     */
    void printImageInfo(const Image& img, const std::string& name = "");

    /**
     * Converts an image to 3 channels (RGB)
     * Supports conversion from grayscale (1 channel) and RGBA (4 channels)
     *
     * @param src The source image
     * @return A new image with 3 channels
     * @throws std::runtime_error if the source has unsupported channel count
     */
    Image convertTo3Channels(const Image& src);

    /**
     * Embeds an image as metadata in another image
     * Serializes the image data and stores it in the target image's metadata
     *
     * @param targetImage The image to add metadata to
     * @param key The metadata key to use
     * @param dataImage The image to embed as metadata
     */
    void embedMetadataImage(Image& targetImage, const std::string& key, const Image& dataImage);

    /**
     * Extracts an image from metadata
     *
     * @param sourceImage The image containing the metadata
     * @param key The metadata key to extract from
     * @return A new image reconstructed from the metadata
     * @throws std::runtime_error if the metadata key is not found
     */
    Image extractMetadataImage(const Image& sourceImage, const std::string& key);

    /**
     * Converts text to a simple 1-pixel-high grayscale image
     *
     * @param text The text to convert
     * @return An image representing the text
     */
    Image textToImage(const std::string& text);

    /**
     * Extracts text from an image
     *
     * @param img The image containing encoded text
     * @return The extracted text
     */
    std::string textFromImage(const Image& img);

    std::vector<unsigned char> serializeImage(const Image& img);
    bool deserializeImage(const std::vector<unsigned char>& data, Image& img);
}