#ifndef ENCRYPTION_ALGORITHM_H
#define ENCRYPTION_ALGORITHM_H

#include <QString>
#include <QStringList>
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

    /**
     * @brief Returns a short metadata tag for delimited output trailers
     * @return Algorithm metadata tag (e.g. "WSA")
     */
    virtual QString metadataTag() const {
        return id().toUpper();
    }

    /**
     * @brief Encrypts a single delimited segment.
     * By default, this defers to the parameterized implementation.
     * @param segment The segment to encrypt
     * @return The encrypted segment
     */
    virtual QString encryptSegment(const QString& segment) const {
        return encryptSegment(segment, {});
    }

    /**
     * @brief Encrypts a single delimited segment with algorithm parameters.
     * @param segment The segment to encrypt
     * @param params Algorithm parameters from the opening tag
     * @return The encrypted segment
     */
    virtual QString encryptSegment(const QString& segment, const QStringList& params) const {
        Q_UNUSED(params);
        return encrypt(segment);
    }

    /**
     * @brief Decrypts a single delimited segment.
     * By default, this defers to the parameterized implementation.
     * @param segment The segment to decrypt
     * @return The decrypted segment
     */
    virtual QString decryptSegment(const QString& segment) const {
        return decryptSegment(segment, {});
    }

    /**
     * @brief Decrypts a single delimited segment with algorithm parameters.
     * @param segment The segment to decrypt
     * @param params Algorithm parameters from the trailer tag
     * @return The decrypted segment
     */
    virtual QString decryptSegment(const QString& segment, const QStringList& params) const {
        Q_UNUSED(params);
        return decrypt(segment);
    }

    /**
     * @brief Returns a placeholder parameter list for opening tags.
     * Example: "salt,position,num_of_times"
     * This is used by the GUI helper when inserting a new wrapper.
     * @return Comma-separated parameter placeholder text, or empty if not supported
     */
    virtual QString parameterPlaceholder() const {
        return {};
    }
};

using EncryptionAlgorithmPtr = std::shared_ptr<EncryptionAlgorithm>;

#endif // ENCRYPTION_ALGORITHM_H
