#ifndef DELIMITED_WEAVER_SIMPLE_A_H
#define DELIMITED_WEAVER_SIMPLE_A_H

#include "EncryptionAlgorithm.h"
#include <QRegularExpression>
#include <QStringList>

/**
 * @class DelimitedWeaverSimpleA
 * @brief A custom delimiter-based algorithm for tagged in-band encryption.
 *
 * This algorithm encrypts only text that is wrapped inside the delimiter
 * sequence "|#|" and appends metadata trailers to the end of the output.
 * Each trailer encodes the encrypted range and the algorithm tag.
 */
class DelimitedWeaverSimpleA : public EncryptionAlgorithm {
public:
    DelimitedWeaverSimpleA() = default;
    ~DelimitedWeaverSimpleA() override = default;

    QString name() const override;
    QString id() const override;
    QString metadataTag() const override;
    QString parameterPlaceholder() const override;
    QString encrypt(const QString& plaintext) const override;
    QString decrypt(const QString& ciphertext) const override;
    QString encryptSegment(const QString& segment, const QStringList& params) const override;
    QString decryptSegment(const QString& segment, const QStringList& params) const override;

private:
    static QStringList parseParameters(const QString& parameterString);
    static QString serializeParameters(const QStringList& params);
    static constexpr int PositionHexWidth = 6;
    static const QString OpenTagPrefix;
    static const QString Delimiter;
    static const QString ClosingDelimiter;
    static const QRegularExpression OpenTagPattern;
    static const QRegularExpression TagSuffixPattern;
};

#endif // DELIMITED_WEAVER_SIMPLE_A_H
