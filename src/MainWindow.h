#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QTableWidget>
#include <QFileDialog>
#include <QVBoxLayout>

#include "ImageCryptoApp.h"

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    signals:
        void logMessage(const QString& message);

private slots:
    void browseInputFile();
    void browseOutputFile();
    void browseDataFile();
    void browseStegInputFile();
    void browseStegOutputFile();
    void runOperation();
    void updateUiForSteganography() const;
    void addEncryptionStep() const;
    void removeEncryptionStep() const;
    void appendLogMessage(const QString &message) const;
    void handleOperationComplete(bool success, const QString& message);

private:
    void setupEncryptionTab();
    void setupSteganographyTab();
    void setupLoggingArea(QVBoxLayout *mainLayout);

    QTabWidget *tabWidget;
    QTextEdit *logArea;

    // Encryption Tab Widgets
    QWidget *encryptionTab;
    QRadioButton *encryptRadio;
    QRadioButton *decryptRadio;
    QLineEdit *inputFileEdit;
    QLineEdit *outputFileEdit;
    QLineEdit *passwordEdit;
    QTableWidget *stepsTable;
    QPushButton *addStepButton;
    QPushButton *removeStepButton;

    // Steganography Tab Widgets
    QWidget *stegTab;
    QRadioButton *hideRadio;
    QRadioButton *extractRadio;
    QLineEdit *stegInputFileEdit;
    QLineEdit *stegOutputFileEdit;
    QLineEdit *stegPasswordEdit;
    QComboBox *algorithmCombo;
    QCheckBox *asImageCheck;
    QLineEdit *dataFileEdit;
    QTextEdit *dataTextEdit;

    // Common Widgets
    QPushButton *runButton;
    ImageCryptoApp app;
    bool operationRunning = false;
};