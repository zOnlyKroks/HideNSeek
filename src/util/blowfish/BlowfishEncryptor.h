#pragma once
#include <vector>
#include <string>

class BlowfishEncryptor {
public:
    enum class Mode { CBC, CFB };

private:
    static constexpr int ROUNDS = 16;
    static constexpr int BLOCK_SIZE = 8; // 64-bit blocks

    struct BlowfishContext {
        uint32_t P[ROUNDS + 2];
        uint32_t S[4][256];
    } ctx{};

    Mode mode;

    // Initial P-array (first 32 bits of fractional parts of pi)
    static const uint32_t INITIAL_P[ROUNDS + 2];
    // Initial S-boxes (first 32 bits of fractional parts of pi)
    static const uint32_t INITIAL_S[4][256];

    void initializeContext(const std::string& key, const std::vector<unsigned char>& salt);
    [[nodiscard]] uint32_t F(uint32_t x) const;
    void encryptBlock(uint32_t& left, uint32_t& right) const;
    void decryptBlock(uint32_t& left, uint32_t& right) const;

    [[nodiscard]] std::vector<unsigned char> processBlocks(const std::vector<unsigned char>& data,
                                                             const std::vector<unsigned char>& iv,
                                                             bool encrypt) const;

public:
    BlowfishEncryptor(const std::string& key,
                      const std::vector<unsigned char>& salt,
                      const Mode mode = Mode::CBC)
        : mode(mode) {
        initializeContext(key, salt);
    }

    [[nodiscard]] std::vector<unsigned char> encrypt(const std::vector<unsigned char>& data,
                                                      const std::vector<unsigned char>& iv) const {
        return processBlocks(data, iv, true);
    }

    [[nodiscard]] std::vector<unsigned char> decrypt(const std::vector<unsigned char>& data,
                                       const std::vector<unsigned char>& iv) const {
        return processBlocks(data, iv, false);
    }
};