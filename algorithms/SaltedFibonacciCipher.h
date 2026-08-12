#ifndef SALTED_FIBONACCI_CIPHER_H
#define SALTED_FIBONACCI_CIPHER_H

#include "EncryptionAlgorithm.h"

class SaltedFibonacciCipher : public EncryptionAlgorithm {
public:
    QString name() const override;
    QString id() const override;
    QString metadataTag() const override;
    QString parameterPlaceholder() const override;
    QString encrypt(const QString& plaintext) const override;
    QString decrypt(const QString& ciphertext) const override;
    QString encryptSegment(const QString& segment, const QStringList& params) const override;
    QString decryptSegment(const QString& segment, const QStringList& params) const override;
    QString trailerParameter(const QString& segment, const QStringList& params) const override;
};

#endif // SALTED_FIBONACCI_CIPHER_H
