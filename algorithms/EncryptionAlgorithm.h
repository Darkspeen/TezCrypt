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
    // Shared version of the tagged-trailer parameter format.
    static constexpr int TAGGED_FORMAT_VERSION = 1;

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
     * @brief Returns the trailer parameter text for a wrapped segment.
     * The default preserves the opening-tag parameters for compatibility.
     */
    virtual QString trailerParameter(const QString& segment, const QStringList& params) const {
        Q_UNUSED(segment);
        return params.join(QStringLiteral(","));
    }

    /**
     * @brief Returns the number of algorithm-specific trailer parameters.
     * Override this when the trailer shape differs from the opening-tag
     * parameter placeholder.
     */
    virtual int trailerParameterCount() const {
        const QString placeholder = parameterPlaceholder();
        return placeholder.isEmpty() ? 0 : placeholder.count(QLatin1Char(',')) + 1;
    }

    /**
     * @brief Encrypts a tagged segment and produces its matching trailer parameters.
     * Algorithms with per-encryption metadata can override this to keep the
     * encrypted segment and its trailer parameters consistent. The default
     * trailer starts with TAGGED_FORMAT_VERSION, followed by algorithm parameters.
     */
    virtual QString encryptTaggedSegment(
        const QString& segment,
        const QStringList& params,
        QString& trailerParams
    ) const {
        QString encrypted = encryptSegment(segment, params);
        const QString algorithmParams = trailerParameter(segment, params);
        trailerParams = QString::number(TAGGED_FORMAT_VERSION);
        if (!algorithmParams.isEmpty()) {
            trailerParams += QStringLiteral(",") + algorithmParams;
        }
        return encrypted;
    }

    /**
     * @brief Decrypts a tagged segment, consuming the shared format version.
     * Trailers without a recognized version are passed through unchanged for
     * compatibility with existing tagged text.
     */
    virtual QString decryptTaggedSegment(const QString& segment, const QStringList& params) const {
        if (params.size() == trailerParameterCount() + 1) {
            bool versionOk = false;
            const int version = params.first().toInt(&versionOk);
            if (versionOk) {
                if (version == TAGGED_FORMAT_VERSION) {
                    return decryptSegment(segment, params.mid(1));
                }
                return segment;
            }
        }
        return decryptSegment(segment, params);
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
