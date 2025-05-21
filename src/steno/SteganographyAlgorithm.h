#pragma once

#include "../img/Image.h"
#include <string>
#include <memory>

/**
 * Base class for steganography algorithms.
 * Steganography algorithms hide data within images.
 */
class SteganographyAlgorithm {
public:
    virtual ~SteganographyAlgorithm() = default;

    /**
     * Hide data within a carrier image.
     * @param carrierImage The image to hide data in
     * @param dataToHide The data to hide (could be text or another image)
     * @param resultImage The output image with hidden data
     * @param key Optional security key
     * @return true if operation was successful
     */
    virtual bool hideData(const Image& carrierImage, const std::string& dataToHide,
                         Image& resultImage, const std::string& key = "") = 0;

    /**
     * Hide one image within another.
     * @param carrierImage The image to hide data in
     * @param imageToHide The image to hide
     * @param resultImage The output image with hidden data
     * @param key Optional security key
     * @return true if operation was successful
     */
    virtual bool hideImage(const Image& carrierImage, const Image& imageToHide,
                          Image& resultImage, const std::string& key = "") = 0;

    /**
     * Extract hidden data from an image.
     * @param steganoImage The image containing hidden data
     * @param extractedData The extracted data as string
     * @param key Optional security key used during hiding
     * @return true if operation was successful
     */
    virtual bool extractData(const Image& steganoImage, std::string& extractedData,
                            const std::string& key = "") = 0;

    /**
     * Extract hidden image from an image.
     * @param steganoImage The image containing hidden image
     * @param extractedImage The extracted image
     * @param key Optional security key used during hiding
     * @return true if operation was successful
     */
    virtual bool extractImage(const Image& steganoImage, Image& extractedImage,
                             const std::string& key = "") = 0;

    /**
     * @return The name of the algorithm.
     */
    virtual std::string name() const = 0;

    /**
     * @return Description of the algorithm.
     */
    virtual std::string description() const = 0;

    /**
     * Calculate maximum data size that can be hidden in the given carrier image.
     * @param carrierImage The carrier image
     * @return Maximum number of bytes that can be hidden
     */
    virtual size_t maxHiddenDataSize(const Image& carrierImage) const = 0;
};