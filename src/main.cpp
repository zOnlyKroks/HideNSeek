#include <iostream>

#include "ImageCryptoApp.h"

int main(const int argc, char** argv) {
    std::cout << "ImageCryptoApp starting." << std::endl;
    ImageCryptoApp app;
    app.run(argc, argv);

    return 0;
}
