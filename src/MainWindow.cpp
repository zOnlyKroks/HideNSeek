#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QApplication>
#include <QTemporaryFile>
#include <QFileDialog>
#include <QRadioButton>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QTabWidget>
#include <QIntValidator>
#include <cstring>
#include <iostream>

#include "WorkerThread.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("Image Crypto App");
    resize(800, 600);

    // Create central widget first
    auto centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Create main layout
    auto mainLayout = new QVBoxLayout(centralWidget);

    // Create tab widget
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    setupEncryptionTab();
    setupSteganographyTab();
    setupLoggingArea(mainLayout);

    // Connect log message signal
    connect(this, &MainWindow::logMessage, this, &MainWindow::appendLogMessage);

    app.setLogFunction([this](const std::string& msg) {
        emit logMessage(QString::fromStdString(msg));
    });
}

MainWindow::~MainWindow() {}

void MainWindow::setupEncryptionTab() {
    encryptionTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(encryptionTab);

    // Mode selection
    QGroupBox *modeGroup = new QGroupBox("Operation Mode");
    QHBoxLayout *modeLayout = new QHBoxLayout;
    encryptRadio = new QRadioButton("Encrypt");
    decryptRadio = new QRadioButton("Decrypt");
    encryptRadio->setChecked(true);
    modeLayout->addWidget(encryptRadio);
    modeLayout->addWidget(decryptRadio);
    modeGroup->setLayout(modeLayout);
    layout->addWidget(modeGroup);

    // File selection
    auto *fileGroup = new QGroupBox("Files");
    auto *fileLayout = new QGridLayout;
    fileLayout->addWidget(new QLabel("Input File:"), 0, 0);
    inputFileEdit = new QLineEdit;
    auto *browseInputBtn = new QPushButton("Browse...");
    connect(browseInputBtn, &QPushButton::clicked, this, &MainWindow::browseInputFile);
    fileLayout->addWidget(inputFileEdit, 0, 1);
    fileLayout->addWidget(browseInputBtn, 0, 2);

    fileLayout->addWidget(new QLabel("Output File:"), 1, 0);
    outputFileEdit = new QLineEdit;
    QPushButton *browseOutputBtn = new QPushButton("Browse...");
    connect(browseOutputBtn, &QPushButton::clicked, this, &MainWindow::browseOutputFile);
    fileLayout->addWidget(outputFileEdit, 1, 1);
    fileLayout->addWidget(browseOutputBtn, 1, 2);
    fileGroup->setLayout(fileLayout);
    layout->addWidget(fileGroup);

    // Password
    auto *passGroup = new QGroupBox("Security");
    auto *passLayout = new QVBoxLayout;
    passLayout->addWidget(new QLabel("Master Password:"));
    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);
    passLayout->addWidget(passwordEdit);
    passGroup->setLayout(passLayout);
    layout->addWidget(passGroup);

    // Steps table
    auto *stepsGroup = new QGroupBox("Encryption Steps");
    auto *stepsLayout = new QVBoxLayout;

    stepsTable = new QTableWidget(0, 2);
    stepsTable->setHorizontalHeaderLabels({"Algorithm", "Parameters"});
    stepsTable->horizontalHeader()->setStretchLastSection(true);
    stepsLayout->addWidget(stepsTable);

    auto *btnLayout = new QHBoxLayout;
    addStepButton = new QPushButton("Add Step");
    removeStepButton = new QPushButton("Remove Selected");
    connect(addStepButton, &QPushButton::clicked, this, &MainWindow::addEncryptionStep);
    connect(removeStepButton, &QPushButton::clicked, this, &MainWindow::removeEncryptionStep);
    btnLayout->addWidget(addStepButton);
    btnLayout->addWidget(removeStepButton);
    stepsLayout->addLayout(btnLayout);

    stepsGroup->setLayout(stepsLayout);
    layout->addWidget(stepsGroup);

    tabWidget->addTab(encryptionTab, "Encryption/Decryption");
}

void MainWindow::setupSteganographyTab() {
    stegTab = new QWidget;
    auto *layout = new QVBoxLayout(stegTab);

    // Mode selection
    auto *modeGroup = new QGroupBox("Operation Mode");
    auto *modeLayout = new QHBoxLayout;
    hideRadio = new QRadioButton("Hide Data");
    extractRadio = new QRadioButton("Extract Data");
    hideRadio->setChecked(true);
    connect(hideRadio, &QRadioButton::toggled, this, &MainWindow::updateUiForSteganography);
    modeLayout->addWidget(hideRadio);
    modeLayout->addWidget(extractRadio);
    modeGroup->setLayout(modeLayout);
    layout->addWidget(modeGroup);

    // File selection
    auto *fileGroup = new QGroupBox("Files");
    auto *fileLayout = new QGridLayout;
    fileLayout->addWidget(new QLabel("Input Image:"), 0, 0);
    stegInputFileEdit = new QLineEdit;
    auto *browseStegInputBtn = new QPushButton("Browse...");
    connect(browseStegInputBtn, &QPushButton::clicked, this, &MainWindow::browseStegInputFile);
    fileLayout->addWidget(stegInputFileEdit, 0, 1);
    fileLayout->addWidget(browseStegInputBtn, 0, 2);

    fileLayout->addWidget(new QLabel("Output File:"), 1, 0);
    stegOutputFileEdit = new QLineEdit;
    auto *browseStegOutputBtn = new QPushButton("Browse...");
    connect(browseStegOutputBtn, &QPushButton::clicked, this, &MainWindow::browseStegOutputFile);
    fileLayout->addWidget(stegOutputFileEdit, 1, 1);
    fileLayout->addWidget(browseStegOutputBtn, 1, 2);
    fileGroup->setLayout(fileLayout);
    layout->addWidget(fileGroup);

    // Algorithm and options
    auto *optionsGroup = new QGroupBox("Options");
    auto *optionsLayout = new QGridLayout;

    optionsLayout->addWidget(new QLabel("Algorithm:"), 0, 0);
    algorithmCombo = new QComboBox;
    algorithmCombo->addItems({"LSB", "PVD"});
    optionsLayout->addWidget(algorithmCombo, 0, 1);

    optionsLayout->addWidget(new QLabel("Password:"), 1, 0);
    stegPasswordEdit = new QLineEdit;
    stegPasswordEdit->setEchoMode(QLineEdit::Password);
    optionsLayout->addWidget(stegPasswordEdit, 1, 1);

    asImageCheck = new QCheckBox("Treat data as image");
    optionsLayout->addWidget(asImageCheck, 2, 0, 1, 2);

    optionsGroup->setLayout(optionsLayout);
    layout->addWidget(optionsGroup);

    // Data section
    auto *dataGroup = new QGroupBox("Data");
    auto *dataLayout = new QVBoxLayout;

    dataFileEdit = new QLineEdit;
    auto *browseDataBtn = new QPushButton("Browse Data File...");
    connect(browseDataBtn, &QPushButton::clicked, this, &MainWindow::browseDataFile);
    dataLayout->addWidget(new QLabel("Data File:"));
    dataLayout->addWidget(dataFileEdit);
    dataLayout->addWidget(browseDataBtn);

    dataLayout->addWidget(new QLabel("Or enter text:"));
    dataTextEdit = new QTextEdit;
    dataLayout->addWidget(dataTextEdit);

    dataGroup->setLayout(dataLayout);
    layout->addWidget(dataGroup);

    tabWidget->addTab(stegTab, "Steganography");
    updateUiForSteganography();
}

void MainWindow::setupLoggingArea(QVBoxLayout *mainLayout) {
    auto *bottomWidget = new QWidget;
    auto *bottomLayout = new QVBoxLayout(bottomWidget);

    bottomLayout->addWidget(new QLabel("Operation Log:"));
    logArea = new QTextEdit;
    logArea->setReadOnly(true);
    logArea->setMaximumHeight(150); // Limit height so tabs are visible
    bottomLayout->addWidget(logArea);

    runButton = new QPushButton("Run Operation");
    connect(runButton, &QPushButton::clicked, this, &MainWindow::runOperation);
    bottomLayout->addWidget(runButton);

    mainLayout->addWidget(bottomWidget);
}

void MainWindow::browseInputFile() {
    const QString file = QFileDialog::getOpenFileName(this, "Select Input File", "",
                                              "Image Files (*.png *.jpg *.bmp)");
    if (!file.isEmpty()) {
        inputFileEdit->setText(file);
    }
}

void MainWindow::browseStegInputFile() {
    const QString file = QFileDialog::getOpenFileName(this, "Select Input Image", "",
                                              "Image Files (*.png *.jpg *.bmp)");
    if (!file.isEmpty()) {
        stegInputFileEdit->setText(file);
    }
}

void MainWindow::browseOutputFile() {
    const QString file = QFileDialog::getSaveFileName(this, "Select Output File", "",
                                              "Image Files (*.png *.jpg *.bmp)");
    if (!file.isEmpty()) {
        outputFileEdit->setText(file);
    }
}

void MainWindow::browseStegOutputFile() {
    const QString file = QFileDialog::getSaveFileName(this, "Select Output File", "",
                                              "Image Files (*.png *.jpg *.bmp)");
    if (!file.isEmpty()) {
        stegOutputFileEdit->setText(file);
    }
}

void MainWindow::browseDataFile() {
    if (const QString file = QFileDialog::getOpenFileName(this, "Select Data File"); !file.isEmpty()) {
        dataFileEdit->setText(file);
    }
}

void MainWindow::updateUiForSteganography() const {
    const bool isHideMode = hideRadio->isChecked();
    dataFileEdit->setEnabled(isHideMode);
    dataTextEdit->setEnabled(isHideMode);
    asImageCheck->setEnabled(isHideMode);
}

void MainWindow::addEncryptionStep() const {
    const int row = stepsTable->rowCount();
    stepsTable->insertRow(row);

    auto *algoCombo = new QComboBox;
    algoCombo->addItems({"addbit", "xor", "rotn", "bitnot", "channelswap", "pixelperm", "aes256"});
    stepsTable->setCellWidget(row, 0, algoCombo);

    auto *paramEdit = new QLineEdit;
    paramEdit->setPlaceholderText("Runs (1-99)");

    const auto *validator = new QIntValidator(1, 99, paramEdit);
    paramEdit->setValidator(validator);

    stepsTable->setCellWidget(row, 1, paramEdit);
}

void MainWindow::removeEncryptionStep() const {
    if (int row = stepsTable->currentRow(); row >= 0) {
        stepsTable->removeRow(row);
    }
}

void MainWindow::appendLogMessage(const QString& message) const {
    logArea->append(message);
}

void MainWindow::handleOperationComplete(bool success, const QString& message) {
    operationRunning = false;
    runButton->setEnabled(true);
    appendLogMessage(success ? "Operation completed successfully!" : "Operation failed!");
    appendLogMessage(message);

    if (!success) {
        QMessageBox::critical(this, "Error", message);
    }
}

void MainWindow::runOperation() {
    if (operationRunning) return;

    logArea->clear();
    operationRunning = true;
    runButton->setEnabled(false);

    // Collect arguments based on current tab
    QList<QByteArray> args;
    args.append("ImageCryptoApp");  // Dummy executable name

    if (tabWidget->currentIndex() == 0) {  // Encryption tab
        // Mode
        if (decryptRadio->isChecked()) {
            args.append("--decrypt");
        }

        // Files
        args.append("--inputFile");
        args.append(inputFileEdit->text().toUtf8());
        args.append("--outputFile");
        args.append(outputFileEdit->text().toUtf8());

        // Password
        args.append("--masterPassword");
        args.append(passwordEdit->text().toUtf8());

        for (int i = 0; i < stepsTable->rowCount(); i++) {
            const auto algo = qobject_cast<QComboBox*>(stepsTable->cellWidget(i, 0));

            if (const auto param = qobject_cast<QLineEdit*>(stepsTable->cellWidget(i, 1)); algo && param) {
                QString step = algo->currentText();

                if (QString paramText = param->text().trimmed(); !paramText.isEmpty()) {
                    bool ok;
                    if (const int runs = paramText.toInt(&ok); !ok || runs < 1 || runs > 99) {
                        appendLogMessage(QString("Invalid parameter '%1' for step %2. Must be integer 1-99.")
                                       .arg(paramText).arg(i + 1));
                        operationRunning = false;
                        runButton->setEnabled(true);
                        return;
                    }
                    step += ":" + paramText;
                }
                args.append("--steps");
                args.append(step.toUtf8());
            }
        }

        std::cout << "Arguments: ";
        for (int i = 0; i < args.size(); i++) {
            std::cout << args[i].constData();
            if (i < args.size() - 1) {
                std::cout << " ";
            }
        }
        std::cout << std::endl;
    } else {  // Steganography tab
        // Mode
        args.append("--steg");
        args.append(hideRadio->isChecked() ? "hide" : "extract");

        // Algorithm
        args.append("--algo");
        args.append(algorithmCombo->currentText().toLower().toUtf8());

        // Files
        args.append("--inputFile");
        args.append(stegInputFileEdit->text().toUtf8());
        args.append("--outputFile");
        args.append(stegOutputFileEdit->text().toUtf8());

        // Password
        if (!stegPasswordEdit->text().isEmpty()) {
            args.append("--pass");
            args.append(stegPasswordEdit->text().toUtf8());
        }

        // Data options
        if (hideRadio->isChecked()) {
            if (asImageCheck->isChecked()) {
                args.append("--image");
            }

            // Determine data source
            if (!dataFileEdit->text().isEmpty()) {
                args.append("--data");
                args.append(dataFileEdit->text().toUtf8());
            } else if (!dataTextEdit->toPlainText().isEmpty()) {
                // Save text to temp file
                if (const auto tempFile = new QTemporaryFile(this); tempFile->open()) {
                    tempFile->write(dataTextEdit->toPlainText().toUtf8());
                    tempFile->close();
                    args.append("--data");
                    args.append(tempFile->fileName().toUtf8());
                }
            }
        }
    }

    // Prepare arguments for worker thread
    const int argc = args.size();
    const auto argv = new char*[argc + 1];
    for (int i = 0; i < argc; i++) {
        argv[i] = new char[args[i].size() + 1];
        std::strcpy(argv[i], args[i].constData());
    }
    argv[argc] = nullptr;
    
    // Run in background thread
    const auto worker = new WorkerThread(&app, argc, argv);
    connect(worker, &WorkerThread::finished, this, &MainWindow::handleOperationComplete);
    connect(worker, &WorkerThread::finished, worker, &QObject::deleteLater);
    worker->start();
}