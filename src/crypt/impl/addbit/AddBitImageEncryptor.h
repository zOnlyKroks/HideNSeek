#pragma once
#include "../../CryptoAlgorithm.h"

class AddBitImageEncryptor final : public CryptoAlgorithm {
public:
    [[nodiscard]] std::string name() const override { return "addbit"; }
    void encrypt(const Image& input, Image& output, const std::string& key) override;
    void decrypt(const Image& input, Image& output, const std::string& key) override;
    [[nodiscard]] std::vector<std::string> getEncryptionSteps(const Image& in) const override { return { "addbit:1" }; }
};

