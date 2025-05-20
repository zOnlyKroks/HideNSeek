#pragma once

#include "../../CryptoAlgorithm.h"
#include "../../../img/Image.h"

class RotNImageEncryptor final : public CryptoAlgorithm {
public:
    [[nodiscard]] std::string name() const override { return "rotn"; }
    void encrypt(const Image& input, Image& output, const std::string& key) override;
    void decrypt(const Image& input, Image& output, const std::string& key) override;
    [[nodiscard]] std::vector<std::string> getEncryptionSteps(const Image& in) const override { return { "rotn:1" };}
};
