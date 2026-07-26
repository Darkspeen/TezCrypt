#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <memory>

class QComboBox;
class QTextEdit;
class QPushButton;
class EncryptionManager;

/**
 * @class MainWindow
 * @brief Main GUI window for the TezCrypt application
 * 
 * Provides the user interface for encryption/decryption operations.
 * The UI listens to the EncryptionManager for available algorithms
 * and remains independent of concrete algorithm implementations.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(std::shared_ptr<EncryptionManager> manager, 
                       QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    /**
     * @brief Encrypts the input text using the selected algorithm
     */
    void onEncryptClicked();

    /**
     * @brief Decrypts the input text using the selected algorithm
     */
    void onDecryptClicked();

    /**
     * @brief Wraps the current selection or cursor position with the selected algorithm tag delimiters
     */
    void onWrapTagClicked();

    /**
     * @brief Upload a file for encryption or decryption
     */
    void onUploadFileClicked();

    /**
     * @brief Copy the output text to clipboard
     */
    void onCopyOutputClicked();

    /**
     * @brief Update the tag letter combos when algorithm selection changes
     */
    void onAlgorithmSelectionChanged(int index);

private:
    /**
     * @brief Initialize all UI components
     */
    void setupUI();

    /**
     * @brief Populate the algorithm combo box from the manager
     */
    void populateAlgorithms();

    /**
     * @brief Perform encryption or decryption operation
     * @param encrypt If true, encrypt; if false, decrypt
     */
    void performCrypto(bool encrypt);

    // UI Components
    QComboBox* m_algorithmCombo;
    QComboBox* m_subAlgorithmCombo;
    QComboBox* m_variantCombo;
    QTextEdit* m_inputText;
    QTextEdit* m_outputText;
    QPushButton* m_uploadButton;
    QPushButton* m_encryptButton;
    QPushButton* m_decryptButton;
    QPushButton* m_wrapButton;
    QPushButton* m_copyButton;

    // Manager for algorithm access
    std::shared_ptr<EncryptionManager> m_manager;
};

#endif // MAIN_WINDOW_H
