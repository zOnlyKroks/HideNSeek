#pragma once
#include <string>

#include "../img/Image.h"

class CryptoAlgorithm {
public:
    virtual ~CryptoAlgorithm() = default;
    virtual void encrypt(const Image& input, Image& output, const std::string& key) = 0;
    virtual void decrypt(const Image& input, Image& output, const std::string& key) = 0;
    [[nodiscard]] virtual std::string name() const = 0;
};
