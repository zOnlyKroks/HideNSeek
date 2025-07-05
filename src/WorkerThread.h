#pragma once

#include <QThread>
#include "ImageCryptoApp.h"

class WorkerThread final : public QThread {
    Q_OBJECT
public:
    WorkerThread(ImageCryptoApp* app, int argc, char** argv)
        : app(app), argc(argc), argv(argv) {}

    void run() override {
        try {
            app->run(argc, argv);
            emit finished(true, "Operation completed successfully");
        } catch (const std::exception& e) {
            emit finished(false, QString("Error: ") + e.what());
        }

        // Cleanup
        for (int i = 0; i < argc; i++) {
            delete[] argv[i];
        }
        delete[] argv;
    }

    signals:
        void finished(bool success, const QString& message);

private:
    ImageCryptoApp* app;
    int argc;
    char** argv;
};