#pragma once
#include "../../CryptoAlgorithm.h"

class AES256ImageEncryptor final : public CryptoAlgorithm {
public:
    [[nodiscard]] std::string name() const override { return "aes256"; }

    void encrypt(const Image& input, Image& output, const std::string& key) override;
    void decrypt(const Image& input, Image& output, const std::string& key) override;

    [[nodiscard]] std::vector<std::string> getEncryptionSteps(const Image&) const override {
        return { "aes256:1" };
    }
};
