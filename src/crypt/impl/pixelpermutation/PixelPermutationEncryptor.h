#pragma once

#include "../../CryptoAlgorithm.h"
#include "../../../img/Image.h"
#include <string>
#include <vector>

class PixelPermutationEncryptor final : public CryptoAlgorithm {
public:
    [[nodiscard]] std::string name() const override { return "pixel_permutation"; }
    void encrypt(const Image& input, Image& output, const std::string& key) override;
    void decrypt(const Image& input, Image& output, const std::string& key) override;
    [[nodiscard]] std::vector<std::string> getEncryptionSteps(const Image& in) const override { return { "pixel_permutation" }; }
};
