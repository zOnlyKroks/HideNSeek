#pragma once
#include "../../CryptoAlgorithm.h"

class XORImageEncryptor final : public CryptoAlgorithm {
public:
    [[nodiscard]] std::string name() const override { return "xor"; }
    void encrypt(const Image& input, Image& output, const std::string& key) override;
    void decrypt(const Image& input, Image& output, const std::string& key) override;
    [[nodiscard]] std::vector<std::string> getEncryptionSteps(const Image& in) const override { return { "xor:1" };}
};