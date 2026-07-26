#ifndef ROT13_H
#define ROT13_H

#include "EncryptionAlgorithm.h"

/**
 * @class ROT13
 * @brief ROT13 cipher implementation
 * 
 * ROT13 is a special case of Caesar cipher that rotates by 13 positions.
 * With ROT13, encoding and decoding are the same operation.
 */
class ROT13 : public EncryptionAlgorithm {
public:
    ROT13() = default;
    ~ROT13() override = default;

    QString name() const override;
    QString metadataTag() const override;
    QString parameterPlaceholder() const override;
    QString encrypt(const QString& plaintext) const override;
    QString decrypt(const QString& ciphertext) const override;

private:
    static constexpr int ROT13_SHIFT = 13;

    /**
     * @brief Helper function to shift a single character by ROT13
     * @param ch The character to rotate
     * @return The rotated character
     */
    QChar rotateChar(QChar ch) const;
};

#endif // ROT13_H
