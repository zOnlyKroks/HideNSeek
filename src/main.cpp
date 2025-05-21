#include <iostream>

#include "ImageCryptoApp.h"

int main(const int argc, char** argv) {
    std::cout << "ImageCryptoApp starting." << std::endl;
    ImageCryptoApp app;
    app.registerAlgorithms();
    app.run(argc, argv);

    if (app.isInSteganographyMode()) {
        app.processSteganography();
    } else {
        app.processImageWithAlgorithm();
    }

    return 0;
}
