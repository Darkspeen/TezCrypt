#include "MainWindow.h"
#include "EncryptionManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QWidget>
#include <QTextCursor>
#include <QFileDialog>
#include <QFile>
#include <QClipboard>
#include <QApplication>

MainWindow::MainWindow(std::shared_ptr<EncryptionManager> manager, 
                       QWidget* parent)
    : QMainWindow(parent), m_manager(manager) {
    setWindowTitle(QStringLiteral("TezCrypt - Encryption/Decryption Utility"));
    setMinimumSize(1200, 800);
    
    setupUI();
    populateAlgorithms();
    
    // Connect signals
    connect(m_algorithmCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onAlgorithmSelectionChanged);
    connect(m_uploadButton, &QPushButton::clicked, this, &MainWindow::onUploadFileClicked);
    connect(m_wrapButton, &QPushButton::clicked, this, &MainWindow::onWrapTagClicked);
    connect(m_encryptButton, &QPushButton::clicked, this, &MainWindow::onEncryptClicked);
    connect(m_decryptButton, &QPushButton::clicked, this, &MainWindow::onDecryptClicked);
    connect(m_copyButton, &QPushButton::clicked, this, &MainWindow::onCopyOutputClicked);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    // Create central widget and main layout
    auto centralWidget = new QWidget(this);
    auto mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(10);

    // Apply dark theme stylesheet
    QString darkStylesheet = QStringLiteral(
        "QMainWindow { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2a2a2a, stop:1 #4a4a4a); }"
        "QWidget { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2a2a2a, stop:1 #4a4a4a); color: #ffffff; }"
        "QLabel { color: #ffffff; background-color: transparent; }"
        "QComboBox { background-color: #2a2a2a; color: #ffffff; border: 1px solid #555555; border-radius: 4px; padding: 4px 8px; }"
        "QComboBox::drop-down { border: none; }"
        "QComboBox::down-arrow { image: none; }"
        "QComboBox QAbstractItemView { background-color: #2a2a2a; color: #ffffff; selection-background-color: #555555; }"
        "QTextEdit { background-color: #000000; color: #ffffff; border: 1px solid #555555; border-radius: 4px; padding: 4px; font-family: 'Courier New'; }"
        "QPushButton { border-radius: 4px; padding: 6px 12px; }"
    );
    this->setStyleSheet(darkStylesheet);

    QString wrapButtonStyle = QStringLiteral(
        "QPushButton { background-color: #e8e8e8; color: #000000; border-radius: 4px; padding: 6px 12px; }"
        "QPushButton:hover { background-color: #f0f0f0; }"
        "QPushButton:pressed { background-color: #d0d0d0; }"
    );

    QString uploadButtonStyle = QStringLiteral(
        "QPushButton { background-color: #b3e5fc; color: #000000; border-radius: 4px; padding: 6px 12px; }"
        "QPushButton:hover { background-color: #c8f4ff; }"
        "QPushButton:pressed { background-color: #80deea; }"
    );

    QString encryptButtonStyle = QStringLiteral(
        "QPushButton { background-color: #ffe0b2; color: #000000; border-radius: 4px; padding: 6px 12px; }"
        "QPushButton:hover { background-color: #ffecb3; }"
        "QPushButton:pressed { background-color: #ffc266; }"
    );

    QString decryptButtonStyle = QStringLiteral(
        "QPushButton { background-color: #c8e6c9; color: #000000; border-radius: 4px; padding: 6px 12px; }"
        "QPushButton:hover { background-color: #d5f4d5; }"
        "QPushButton:pressed { background-color: #a5d6a7; }"
    );

    QString copyButtonStyle = QStringLiteral(
        "QPushButton { background-color: #f8bbd0; color: #000000; border-radius: 4px; padding: 6px 12px; }"
        "QPushButton:hover { background-color: #fcc2d7; }"
        "QPushButton:pressed { background-color: #f48fb1; }"
    );

    // Algorithm selection
    auto algorithmLayout = new QHBoxLayout();
    auto algorithmLabel = new QLabel(QStringLiteral("Algorithm:"), this);
    m_algorithmCombo = new QComboBox(this);
    algorithmLayout->addWidget(algorithmLabel);
    algorithmLayout->addWidget(m_algorithmCombo);
    algorithmLayout->addStretch();
    
    m_uploadButton = new QPushButton(QStringLiteral("Upload File"), this);
    m_uploadButton->setStyleSheet(uploadButtonStyle);
    m_uploadButton->setCursor(Qt::PointingHandCursor);
    algorithmLayout->addWidget(m_uploadButton);
    
    mainLayout->addLayout(algorithmLayout);

    // Tag configuration
    auto tagLayout = new QHBoxLayout();
    auto subAlgorithmLabel = new QLabel(QStringLiteral("Sub-algorithm:"), this);
    m_subAlgorithmCombo = new QComboBox(this);
    m_subAlgorithmCombo->addItem(QStringLiteral("Simple"));

    auto variantLabel = new QLabel(QStringLiteral("Variant:"), this);
    m_variantCombo = new QComboBox(this);
    m_variantCombo->addItems({QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")});

    tagLayout->addWidget(subAlgorithmLabel);
    tagLayout->addWidget(m_subAlgorithmCombo);
    tagLayout->addSpacing(12);
    tagLayout->addWidget(variantLabel);
    tagLayout->addWidget(m_variantCombo);
    tagLayout->addStretch();
    mainLayout->addLayout(tagLayout);

    // Wrap Tag button (top left of input area)
    auto wrapLayout = new QHBoxLayout();
    m_wrapButton = new QPushButton(QStringLiteral("Wrap Tag"), this);
    m_wrapButton->setStyleSheet(wrapButtonStyle);
    m_wrapButton->setCursor(Qt::PointingHandCursor);
    wrapLayout->addWidget(m_wrapButton);
    wrapLayout->addStretch();
    mainLayout->addLayout(wrapLayout);

    // Input text
    auto inputLabel = new QLabel(QStringLiteral("Input Text:"), this);
    inputLabel->setAlignment(Qt::AlignCenter);
    QFont boldFont = inputLabel->font();
    boldFont.setBold(true);
    inputLabel->setFont(boldFont);
    m_inputText = new QTextEdit(this);
    m_inputText->setPlaceholderText(QStringLiteral("Enter text to encrypt or decrypt..."));
    mainLayout->addWidget(inputLabel);
    mainLayout->addWidget(m_inputText);

    // Encrypt/Decrypt buttons at bottom right of input
    auto actionButtonLayout = new QHBoxLayout();
    m_encryptButton = new QPushButton(QStringLiteral("Encrypt"), this);
    m_decryptButton = new QPushButton(QStringLiteral("Decrypt"), this);
    m_encryptButton->setStyleSheet(encryptButtonStyle);
    m_decryptButton->setStyleSheet(decryptButtonStyle);
    m_encryptButton->setCursor(Qt::PointingHandCursor);
    m_decryptButton->setCursor(Qt::PointingHandCursor);
    actionButtonLayout->addStretch();
    actionButtonLayout->addWidget(m_encryptButton);
    actionButtonLayout->addWidget(m_decryptButton);
    mainLayout->addLayout(actionButtonLayout);

    // Output text
    auto outputLabel = new QLabel(QStringLiteral("Output Text:"), this);
    outputLabel->setAlignment(Qt::AlignCenter);
    outputLabel->setFont(boldFont);
    m_outputText = new QTextEdit(this);
    m_outputText->setReadOnly(true);
    m_outputText->setPlaceholderText(QStringLiteral("Result will appear here..."));
    mainLayout->addWidget(outputLabel);
    mainLayout->addWidget(m_outputText);
    
    // Copy button below output
    auto copyLayout = new QHBoxLayout();
    copyLayout->addStretch();
    m_copyButton = new QPushButton(QStringLiteral("Copy Output"), this);
    m_copyButton->setStyleSheet(copyButtonStyle);
    m_copyButton->setCursor(Qt::PointingHandCursor);
    copyLayout->addWidget(m_copyButton);
    mainLayout->addLayout(copyLayout);

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

void MainWindow::onAlgorithmSelectionChanged(int /*index*/) {
    if (!m_manager || m_algorithmCombo->count() == 0) {
        return;
    }

    m_subAlgorithmCombo->setCurrentText(QStringLiteral("Simple"));
    m_variantCombo->setCurrentText(QStringLiteral("A"));
}

void MainWindow::onWrapTagClicked() {
    if (!m_manager || m_algorithmCombo->count() == 0) {
        return;
    }

    QString selectedName = m_algorithmCombo->currentText();
    EncryptionAlgorithmPtr selectedAlgorithm;
    for (const QString& id : m_manager->getAlgorithmIds()) {
        auto algorithm = m_manager->getAlgorithm(id);
        if (algorithm && algorithm->name() == selectedName) {
            selectedAlgorithm = algorithm;
            break;
        }
    }

    if (!selectedAlgorithm) {
        return;
    }

    QString mainTagLetter = selectedAlgorithm->metadataTag().left(1);
    QString subAlgorithmSelection = m_subAlgorithmCombo->currentText();
    QString variantSelection = m_variantCombo->currentText();
    QString selectedTagLetters = QStringLiteral("%1%2%3")
        .arg(mainTagLetter)
        .arg(subAlgorithmSelection.left(1).toUpper())
        .arg(variantSelection.left(1).toUpper());

    QString params = selectedAlgorithm->parameterPlaceholder();
    QString wrappingPrefix = QStringLiteral("|#%1:%2|")
        .arg(selectedTagLetters)
        .arg(params);
    QString wrappingSuffix = QStringLiteral("|#|");

    QTextCursor cursor = m_inputText->textCursor();
    if (cursor.hasSelection()) {
        QString selectedText = cursor.selectedText();
        cursor.insertText(wrappingPrefix + selectedText + wrappingSuffix);
    } else {
        cursor.insertText(wrappingPrefix + QStringLiteral("TEXT_TO_ENCRYPT") + wrappingSuffix);
        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, wrappingSuffix.size());
        m_inputText->setTextCursor(cursor);
    }
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
            if (inputText.contains(QStringLiteral("|#"))) {
                result = m_manager->encryptTaggedText(inputText);
            } else {
                result = algorithm->encrypt(inputText);
            }
        } else {
            if (m_manager->hasTaggedText(inputText)) {
                result = m_manager->decryptTaggedText(inputText);
            } else {
                result = algorithm->decrypt(inputText);
            }
        }
        m_outputText->setText(result);
    } catch (const std::exception& e) {
        m_outputText->setText(QStringLiteral("Error: ") + QString::fromStdString(e.what()));
    }
}

void MainWindow::onUploadFileClicked() {
    QString filePath = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("Open File for Encryption/Decryption"),
        QString(),
        QStringLiteral("All Files (*);;Text Files (*.txt)")
    );

    if (filePath.isEmpty()) {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_outputText->setText(QStringLiteral("Error: Could not open file."));
        return;
    }

    QString fileContent = QString::fromUtf8(file.readAll());
    file.close();

    fileContent = fileContent.trimmed();
    m_inputText->setPlainText(fileContent);
}

void MainWindow::onCopyOutputClicked() {
    QString outputText = m_outputText->toPlainText();
    if (outputText.isEmpty()) {
        return;
    }

    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(outputText);
}
