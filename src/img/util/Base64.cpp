// Base64.cpp
#include "Base64.h"
#include <iostream>

namespace Base64 {

const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

inline bool is_base64(unsigned char c) {
    return std::isalnum(c) || (c == '+') || (c == '/');
}

std::string encode(const std::vector<unsigned char>& data) {
    std::string ret;
    int i = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    size_t len = data.size();
    size_t pos = 0;

    while (len--) {
        char_array_3[i++] = data[pos++];
        if (i == 3) {
            char_array_4[0] =  (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] =   char_array_3[2] & 0x3f;

            for (i = 0; i < 4; ++i)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (int j = i; j < 3; ++j)
            char_array_3[j] = 0;

        char_array_4[0] =  (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] =   char_array_3[2] & 0x3f;

        for (int j = 0; j < i + 1; ++j)
            ret += base64_chars[char_array_4[j]];

        // Add padding '=' characters
        while ((i++ < 3))
            ret += '=';
    }

    return ret;
}

std::vector<unsigned char> decode(const std::string& encoded_string) {
    size_t in_len = encoded_string.size();
    int i = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::vector<unsigned char> ret;

    while (in_ < in_len && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_++];

        if (i == 4) {
            // Convert from base64 encoding to original representation
            for (i = 0; i < 4; ++i)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            // Decode 4 characters into 3 bytes
            char_array_3[0] =  (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0x0f) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x03) << 6) + char_array_4[3];

            for (i = 0; i < 3; ++i)
                ret.push_back(char_array_3[i]);
            i = 0;
        }
    }

    // Handle the case where we have only part of a 4-character block
    if (i) {
        // Fill remaining characters with 0
        for (int j = i; j < 4; ++j)
            char_array_4[j] = 0;

        // Convert from base64 encoding to indices
        for (int j = 0; j < i; ++j)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        // Decode the characters we have
        char_array_3[0] =  (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0x0f) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x03) << 6) + char_array_4[3];

        // Store only valid decoded bytes
        for (int j = 0; j < i - 1; ++j)
            ret.push_back(char_array_3[j]);
    }

    return ret;
}

// Helper to encode from text string (UTF-8 assumed)
std::string encodeString(const std::string& text) {
    std::vector<unsigned char> data(text.begin(), text.end());
    return encode(data);
}

// Helper to decode to text string (may contain binary data, so use carefully)
std::string decodeToString(const std::string& encoded_string) {
    std::vector<unsigned char> decoded = decode(encoded_string);
    return std::string(decoded.begin(), decoded.end());
}

} // namespace Base64