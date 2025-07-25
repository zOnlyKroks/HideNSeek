#include "SwapChannelsImageEncryptor.h"
#include <algorithm>
#include <numeric>
#include <functional>
#include <random>
#include <stdexcept>

std::vector<int> SwapChannelsImageEncryptor::getChannelOrder(const std::string& key, int channels) {
    std::vector<int> order(channels);
    std::iota(order.begin(), order.end(), 0);

    const size_t seed = std::hash<std::string>{}(key);
    std::mt19937 rng(static_cast<uint32_t>(seed));
    std::ranges::shuffle(order, rng);

    return order;
}

void SwapChannelsImageEncryptor::encrypt(const Image& input, Image& output, const std::string& key) {
    output = Image(input.width, input.height, input.channels);
    const auto order = getChannelOrder(key, input.channels);

    for (int y = 0; y < input.height; ++y) {
        for (int x = 0; x < input.width; ++x) {
            for (int c = 0; c < input.channels; ++c) {
                output.getPixel(x, y, c) = input.getPixel(x, y, order[c]);
            }
        }
    }
}

void SwapChannelsImageEncryptor::decrypt(const Image& input, Image& output, const std::string& key) {
    output = Image(input.width, input.height, input.channels);
    const auto order = getChannelOrder(key, input.channels);

    std::vector<int> inverse(input.channels);
    for (int i = 0; i < input.channels; ++i) {
        inverse[order[i]] = i;
    }

    for (int y = 0; y < input.height; ++y) {
        for (int x = 0; x < input.width; ++x) {
            for (int c = 0; c < input.channels; ++c) {
                output.getPixel(x, y, c) = input.getPixel(x, y, inverse[c]);
            }
        }
    }
}
