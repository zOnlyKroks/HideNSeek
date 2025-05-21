#ifndef AES256ENCRYPTOR_H
#define AES256ENCRYPTOR_H

#include <vector>
#include <string>

class AES256Encryptor {
public:
    AES256Encryptor(const std::string& password, const std::vector<unsigned char>& salt);
    std::vector<unsigned char> encrypt(const std::vector<unsigned char>& plaintext) const;
    std::vector<unsigned char> decrypt(const std::vector<unsigned char>& ciphertext) const;

private:
    std::vector<unsigned char> key_;
    std::vector<unsigned char> iv_;
};

#endif // AES256ENCRYPTOR_H
