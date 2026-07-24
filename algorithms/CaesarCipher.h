#ifndef CAESAR_CIPHER_H
#define CAESAR_CIPHER_H

#include "EncryptionAlgorithm.h"

/**
 * @class CaesarCipher
 * @brief Caesar Cipher implementation (shift by 3)
 * 
 * A simple substitution cipher that shifts each letter by a fixed amount.
 * Classical Caesar cipher with fixed shift of 3 positions.
 */
class CaesarCipher : public EncryptionAlgorithm {
public:
    CaesarCipher() = default;
    ~CaesarCipher() override = default;

    QString name() const override;
    QString encrypt(const QString& plaintext) const override;
    QString decrypt(const QString& ciphertext) const override;

private:
    static constexpr int DEFAULT_SHIFT = 3;

    /**
     * @brief Helper function to shift a single character
     * @param ch The character to shift
     * @param shift The shift amount
     * @return The shifted character
     */
    QChar shiftChar(QChar ch, int shift) const;
};

#endif // CAESAR_CIPHER_H
