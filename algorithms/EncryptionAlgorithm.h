#ifndef ENCRYPTION_ALGORITHM_H
#define ENCRYPTION_ALGORITHM_H

#include <QString>
#include <memory>

/**
 * @class EncryptionAlgorithm
 * @brief Abstract base class for all encryption algorithms
 * 
 * Every encryption algorithm must inherit from this class and implement
 * the pure virtual functions. This design enables polymorphic behavior
 * without the GUI needing to know concrete algorithm types.
 */
class EncryptionAlgorithm {
public:
    virtual ~EncryptionAlgorithm() = default;

    /**
     * @brief Returns the human-readable name of this algorithm
     * @return Algorithm name (e.g., "Caesar Cipher", "ROT13" )
     */
    virtual QString name() const = 0;

    /**
     * @brief Encrypts the given plaintext
     * @param plaintext The text to encrypt
     * @return The encrypted text
     */
    virtual QString encrypt(const QString& plaintext) const = 0;

    /**
     * @brief Decrypts the given ciphertext
     * @param ciphertext The text to decrypt
     * @return The decrypted text
     */
    virtual QString decrypt(const QString& ciphertext) const = 0;

    /**
     * @brief Returns a unique identifier for this algorithm
     * Used internally for algorithm selection
     * @return Algorithm identifier
     */
    virtual QString id() const {
        return name().toLower().replace(" ", "_");
    }
};

using EncryptionAlgorithmPtr = std::shared_ptr<EncryptionAlgorithm>;

#endif // ENCRYPTION_ALGORITHM_H
