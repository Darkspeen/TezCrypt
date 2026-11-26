#ifndef SALTED_FIBONACCI_CIPHER_H
#define SALTED_FIBONACCI_CIPHER_H

#include "EncryptionAlgorithm.h"

/**
 * Demonstration cipher that encrypts with a user-supplied key and weaves its
 * recoverable key encoding into the output using Fibonacci-spaced positions.
 * Variant A uses hex; variant B uses Base64 and chunked placement.
 * Because the key is embedded, this provides obfuscation, not secure encryption.
 */
class SaltedFibonacciCipher : public EncryptionAlgorithm {
public:
    QString name() const override;
    QString id() const override;
    QString metadataTag() const override;
    QStringList metadataTagVariants() const override;
    QString parameterPlaceholder() const override;
    QString encrypt(const QString& plaintext) const override;
    QString decrypt(const QString& ciphertext) const override;
    QString encryptSegment(const QString& segment, const QStringList& params) const override;
    QString encryptTaggedSegment(
        const QString& segment,
        const QStringList& params,
        QString& trailerParams
    ) const override;
    QString encryptTaggedSegmentForTag(
        const QString& segment,
        const QStringList& params,
        const QString& tag,
        QString& trailerParams
    ) const override;
    QString decryptTaggedSegment(const QString& segment, const QStringList& params) const override;
    QString decryptTaggedSegmentForTag(
        const QString& segment,
        const QStringList& params,
        const QString& tag
    ) const override;
    QString decryptSegment(const QString& segment, const QStringList& params) const override;
    QString trailerParameter(const QString& segment, const QStringList& params) const override;
    int trailerParameterCount() const override;
};

#endif // SALTED_FIBONACCI_CIPHER_H
