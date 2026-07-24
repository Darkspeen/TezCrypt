#include "MainWindow.h"
#include "EncryptionManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QWidget>

MainWindow::MainWindow(std::shared_ptr<EncryptionManager> manager, 
                       QWidget* parent)
    : QMainWindow(parent), m_manager(manager) {
    setWindowTitle(QStringLiteral("TezCrypt - Encryption/Decryption Utility"));
    setMinimumSize(600, 500);
    
    setupUI();
    populateAlgorithms();
    
    // Connect signals
    connect(m_encryptButton, &QPushButton::clicked, this, &MainWindow::onEncryptClicked);
    connect(m_decryptButton, &QPushButton::clicked, this, &MainWindow::onDecryptClicked);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    // Create central widget and main layout
    auto centralWidget = new QWidget(this);
    auto mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(10);

    // Algorithm selection
    auto algorithmLayout = new QHBoxLayout();
    auto algorithmLabel = new QLabel(QStringLiteral("Algorithm:"), this);
    m_algorithmCombo = new QComboBox(this);
    algorithmLayout->addWidget(algorithmLabel);
    algorithmLayout->addWidget(m_algorithmCombo);
    algorithmLayout->addStretch();
    mainLayout->addLayout(algorithmLayout);

    // Input text
    auto inputLabel = new QLabel(QStringLiteral("Input Text:"), this);
    m_inputText = new QTextEdit(this);
    m_inputText->setPlaceholderText(QStringLiteral("Enter text to encrypt or decrypt..."));
    mainLayout->addWidget(inputLabel);
    mainLayout->addWidget(m_inputText);

    // Buttons (side by side)
    auto buttonLayout = new QHBoxLayout();
    m_encryptButton = new QPushButton(QStringLiteral("Encrypt"), this);
    m_decryptButton = new QPushButton(QStringLiteral("Decrypt"), this);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_encryptButton);
    buttonLayout->addWidget(m_decryptButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    // Output text
    auto outputLabel = new QLabel(QStringLiteral("Output Text:"), this);
    m_outputText = new QTextEdit(this);
    m_outputText->setReadOnly(true);
    m_outputText->setPlaceholderText(QStringLiteral("Result will appear here..."));
    mainLayout->addWidget(outputLabel);
    mainLayout->addWidget(m_outputText);

    setCentralWidget(centralWidget);
}

void MainWindow::populateAlgorithms() {
    if (!m_manager) {
        return;
    }

    QStringList names = m_manager->getAlgorithmNames();
    m_algorithmCombo->addItems(names);
}

void MainWindow::onEncryptClicked() {
    performCrypto(true);
}

void MainWindow::onDecryptClicked() {
    performCrypto(false);
}

void MainWindow::performCrypto(bool encrypt) {
    if (!m_manager || m_algorithmCombo->count() == 0) {
        m_outputText->setText(QStringLiteral("Error: No algorithms available."));
        return;
    }

    QString inputText = m_inputText->toPlainText();
    if (inputText.isEmpty()) {
        m_outputText->clear();
        return;
    }

    // Get the selected algorithm name
    QString selectedName = m_algorithmCombo->currentText();

    // Find the algorithm ID that matches this name
    QStringList algorithmIds = m_manager->getAlgorithmIds();
    QString algorithmId;
    
    for (const QString& id : algorithmIds) {
        auto algorithm = m_manager->getAlgorithm(id);
        if (algorithm && algorithm->name() == selectedName) {
            algorithmId = id;
            break;
        }
    }

    if (algorithmId.isEmpty()) {
        m_outputText->setText(QStringLiteral("Error: Algorithm not found."));
        return;
    }

    // Get the algorithm and perform encryption/decryption
    auto algorithm = m_manager->getAlgorithm(algorithmId);
    if (!algorithm) {
        m_outputText->setText(QStringLiteral("Error: Failed to load algorithm."));
        return;
    }

    QString result;
    try {
        if (encrypt) {
            result = algorithm->encrypt(inputText);
        } else {
            result = algorithm->decrypt(inputText);
        }
        m_outputText->setText(result);
    } catch (const std::exception& e) {
        m_outputText->setText(QStringLiteral("Error: ") + QString::fromStdString(e.what()));
    }
}
