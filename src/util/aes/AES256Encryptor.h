#pragma once
#include <vector>

class AES256Encryptor {
public:
    AES256Encryptor(const std::string& password, const std::vector<unsigned char>& salt);

    // Provide IV for encryption
    std::vector<unsigned char> encrypt(const std::vector<unsigned char>& plaintext, const std::vector<unsigned char>& iv) const;
    std::vector<unsigned char> decrypt(const std::vector<unsigned char>& ciphertext, const std::vector<unsigned char>& iv) const;

private:
    std::vector<unsigned char> key_;
};
