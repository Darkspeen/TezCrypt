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
    QTextEdit* m_inputText;
    QTextEdit* m_outputText;
    QPushButton* m_encryptButton;
    QPushButton* m_decryptButton;

    // Manager for algorithm access
    std::shared_ptr<EncryptionManager> m_manager;
};

#endif // MAIN_WINDOW_H
