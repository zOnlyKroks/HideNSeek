// Base64.h
#pragma once
#include <string>
#include <vector>

namespace Base64 {

    // Encode binary data to base64 string
    std::string encode(const std::vector<unsigned char>& data);

    // Decode base64 string to binary data
    std::vector<unsigned char> decode(const std::string& encoded_string);

    // Convenience overloads for text strings
    std::string encodeString(const std::string& text);
    std::string decodeToString(const std::string& encoded_string);

} // namespace Base64
