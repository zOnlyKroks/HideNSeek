#pragma once

#include "../../CryptoAlgorithm.h"
#include "../../../img/Image.h"
#include <string>
#include <vector>

class SwapChannelsImageEncryptor final : public CryptoAlgorithm {
public:
    [[nodiscard]] std::string name() const override { return "swap_channels"; }
    void encrypt(const Image& input, Image& output, const std::string& key) override;
    void decrypt(const Image& input, Image& output, const std::string& key) override;
    [[nodiscard]] std::vector<std::string> getEncryptionSteps(const Image& in) const override { return { "swap_channels:1" }; }
private:
    static std::vector<int> getChannelOrder(const std::string& key, int channels);
};
