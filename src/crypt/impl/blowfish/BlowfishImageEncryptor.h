#pragma once
#include "../../CryptoAlgorithm.h"

class BlowfishImageEncryptor final : public CryptoAlgorithm {
public:
    void encrypt(const Image& input, Image& output, const std::string& key) override;
    void decrypt(const Image& input, Image& output, const std::string& key) override;
    std::string name() const override { return "blowfish";}
    std::vector<std::string> getEncryptionSteps(const Image &in) const override { return {"blowfish:1"}; }
};
