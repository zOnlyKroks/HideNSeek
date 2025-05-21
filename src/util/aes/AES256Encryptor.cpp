#include "AES256Encryptor.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdexcept>
#include <cstring>

AES256Encryptor::AES256Encryptor(const std::string& password, const std::vector<unsigned char>& salt) {
    key_.resize(32); // 256 bits
    iv_.resize(16);  // 128 bits

    if (!PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                           salt.data(), salt.size(),
                           10000, EVP_sha256(), key_.size(), key_.data())) {
        throw std::runtime_error("Key derivation failed");
    }

    // Generate a random IV
    if (!RAND_bytes(iv_.data(), iv_.size())) {
        throw std::runtime_error("IV generation failed");
    }
}

std::vector<unsigned char> AES256Encryptor::encrypt(const std::vector<unsigned char>& plaintext) const {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key_.data(), iv_.data())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptInit_ex failed");
    }

    std::vector<unsigned char> ciphertext(plaintext.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int len;

    if (1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptUpdate failed");
    }
    int ciphertext_len = len;

    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptFinal_ex failed");
    }
    ciphertext_len += len;
    EVP_CIPHER_CTX_free(ctx);

    ciphertext.resize(ciphertext_len);

    // Prepend IV
    std::vector<unsigned char> output;
    output.reserve(iv_.size() + ciphertext_len);
    output.insert(output.end(), iv_.begin(), iv_.end());
    output.insert(output.end(), ciphertext.begin(), ciphertext.end());

    return output;
}

std::vector<unsigned char> AES256Encryptor::decrypt(const std::vector<unsigned char>& ciphertext) const {
    if (ciphertext.size() < 16)
        throw std::runtime_error("Ciphertext too short to contain IV");

    const std::vector iv(ciphertext.begin(), ciphertext.begin() + 16);
    const std::vector actualCipher(ciphertext.begin() + 16, ciphertext.end());

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key_.data(), iv.data())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptInit_ex failed");
    }

    std::vector<unsigned char> plaintext(actualCipher.size());
    int len;

    if (1 != EVP_DecryptUpdate(ctx, plaintext.data(), &len, actualCipher.data(), actualCipher.size())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptUpdate failed");
    }
    int plaintext_len = len;

    if (1 != EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptFinal_ex failed");
    }
    plaintext_len += len;
    EVP_CIPHER_CTX_free(ctx);

    plaintext.resize(plaintext_len);
    return plaintext;
}
