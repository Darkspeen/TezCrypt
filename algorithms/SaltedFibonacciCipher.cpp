#include "SaltedFibonacciCipher.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QStringList>
#include <QVector>
#include <algorithm>

namespace {

QByteArray buildKeystream(const QByteArray& saltBytes, int length) {
    QByteArray state = QCryptographicHash::hash(saltBytes, QCryptographicHash::Sha256);
    QByteArray stream;
    stream.reserve(length);

    for (int i = 0; i < length; ++i) {
        QByteArray input = saltBytes;
        input.append(QByteArray::number(i));
        input.append(state);
        state = QCryptographicHash::hash(input, QCryptographicHash::Sha256);
        stream.append(state.at(0));
    }

    return stream;
}

QByteArray encryptWithSalt(const QByteArray& plaintext, const QByteArray& saltBytes) {
    const QByteArray keystream = buildKeystream(saltBytes, plaintext.size());
    QByteArray result;
    result.reserve(plaintext.size());
    for (int i = 0; i < plaintext.size(); ++i) {
        result.append(plaintext.at(i) ^ keystream.at(i));
    }
    return result;
}

QByteArray decryptWithSalt(const QByteArray& ciphertext, const QByteArray& saltBytes) {
    return encryptWithSalt(ciphertext, saltBytes);
}

int startIndexForKey(const QByteArray& keyBytes) {
    const QByteArray digest = QCryptographicHash::hash(keyBytes, QCryptographicHash::Sha256);
    return static_cast<unsigned char>(digest.at(0)) % 12;
}

QVector<int> buildInsertionPositions(int baseLength, int saltLength, int fibonacciCount, int insertIndex) {
    QVector<int> positions;
    positions.reserve(saltLength);

    if (saltLength <= 0 || baseLength <= 0) {
        return positions;
    }

    QVector<int> fibonacci = {1, 1};
    while (fibonacci.size() < std::max(1, fibonacciCount)) {
        fibonacci.append(fibonacci[fibonacci.size() - 1] + fibonacci[fibonacci.size() - 2]);
    }

    const int totalLength = baseLength + saltLength;
    const int start = std::max(0, insertIndex % totalLength);
    QVector<bool> occupied(totalLength, false);

    for (int i = 0; i < saltLength; ++i) {
        const int index = std::min(i, static_cast<int>(fibonacci.size() - 1));
        const int step = fibonacci.at(index);
        int position = (start + (i * step)) % totalLength;
        while (occupied[position]) {
            position = (position + 1) % totalLength;
        }
        occupied[position] = true;
        positions.append(position);
    }

    std::sort(positions.begin(), positions.end());
    return positions;
}

struct SplitPayload {
    QString saltPayload;
    QString cipherPayload;
};

SplitPayload splitEmbeddedPayload(const QString& encodedText, int saltLength, int fibonacciCount, int insertIndex) {
    SplitPayload result;
    if (saltLength <= 0) {
        result.cipherPayload = encodedText;
        return result;
    }

    const int baseLength = std::max(0, static_cast<int>(encodedText.size() - saltLength));
    const QVector<int> positions = buildInsertionPositions(baseLength, saltLength, fibonacciCount, insertIndex);
    result.saltPayload.reserve(saltLength);
    result.cipherPayload.reserve(baseLength);

    int saltIndex = 0;
    for (int i = 0; i < encodedText.size(); ++i) {
        if (saltIndex < positions.size() && positions.at(saltIndex) == i) {
            result.saltPayload.append(encodedText.at(i));
            ++saltIndex;
        } else {
            result.cipherPayload.append(encodedText.at(i));
        }
    }

    return result;
}

QVector<int> buildLegacyKeyPositions(int baseLength, int keyLength, int start) {
    QVector<int> positions;
    positions.reserve(keyLength);

    qint64 basePosition = std::min(start, baseLength);
    qint64 firstJump = 1;
    qint64 secondJump = 1;
    const qint64 jumpLimit = std::max(1, baseLength);

    for (int i = 0; i < keyLength; ++i) {
        positions.append(static_cast<int>(basePosition + i));

        const qint64 jump = firstJump;
        firstJump = secondJump;
        secondJump = std::min(jumpLimit, firstJump + secondJump);
        basePosition = std::min<qint64>(baseLength, basePosition + jump);
    }

    return positions;
}

QVector<int> buildKeyPositions(int baseLength, int keyLength, int start) {
    QVector<int> positions;
    if (keyLength <= 0) {
        return positions;
    }

    const int totalLength = baseLength + keyLength;
    positions.reserve(keyLength);
    QVector<bool> occupied(totalLength, false);
    qint64 position = start % totalLength;
    qint64 firstJump = 1;
    qint64 secondJump = 1;

    for (int i = 0; i < keyLength; ++i) {
        while (occupied.at(static_cast<int>(position))) {
            position = (position + 1) % totalLength;
        }
        positions.append(static_cast<int>(position));
        occupied[static_cast<int>(position)] = true;

        const qint64 jump = firstJump;
        firstJump = secondJump;
        secondJump = std::min<qint64>(totalLength, firstJump + secondJump);
        position = (position + jump) % totalLength;
    }

    std::sort(positions.begin(), positions.end());
    return positions;
}

QString weaveKey(
    const QString& encodedText,
    const QString& encodedKey,
    int start,
    bool wrappedLayout
) {
    const int baseLength = encodedText.size();
    const int totalLength = baseLength + encodedKey.size();
    const QVector<int> positions = wrappedLayout
        ? buildKeyPositions(baseLength, encodedKey.size(), start)
        : buildLegacyKeyPositions(baseLength, encodedKey.size(), start);

    QString result;
    result.reserve(totalLength);
    int keyIndex = 0;
    int textIndex = 0;
    for (int i = 0; i < totalLength; ++i) {
        if (keyIndex < positions.size() && positions.at(keyIndex) == i) {
            result.append(encodedKey.at(keyIndex++));
        } else {
            result.append(encodedText.at(textIndex++));
        }
    }
    return result;
}

bool unweaveKey(
    const QString& payload,
    int encodedKeyLength,
    int start,
    bool legacyLayout,
    QString& encodedText,
    QString& encodedKey
) {
    if (encodedKeyLength < 0 || encodedKeyLength > payload.size() || encodedKeyLength % 2 != 0
        || start < 0 || start >= 12) {
        return false;
    }

    const int baseLength = payload.size() - encodedKeyLength;
    const QVector<int> positions = legacyLayout
        ? buildLegacyKeyPositions(baseLength, encodedKeyLength, start)
        : buildKeyPositions(baseLength, encodedKeyLength, start);
    encodedText.reserve(baseLength);
    encodedKey.reserve(encodedKeyLength);

    int keyIndex = 0;
    for (int i = 0; i < payload.size(); ++i) {
        if (keyIndex < positions.size() && positions.at(keyIndex) == i) {
            encodedKey.append(payload.at(i));
            ++keyIndex;
        } else {
            encodedText.append(payload.at(i));
        }
    }
    return keyIndex == encodedKeyLength;
}

bool decryptEmbeddedPayload(
    const QString& payload,
    int start,
    int encodedKeyLength,
    bool legacyLayout,
    QString& plaintext
) {
    QString encodedText;
    QString encodedKey;
    if (!unweaveKey(payload, encodedKeyLength, start, legacyLayout, encodedText, encodedKey)) {
        return false;
    }

    const QByteArray keyBytes = QByteArray::fromHex(encodedKey.toLatin1());
    if (keyBytes.toHex() != encodedKey.toLatin1().toLower()) {
        return false;
    }

    const QByteArray encryptedBytes = QByteArray::fromBase64(encodedText.toLatin1());
    if (encryptedBytes.toBase64() != encodedText.toLatin1()) {
        return false;
    }

    plaintext = QString::fromUtf8(decryptWithSalt(encryptedBytes, keyBytes));
    return true;
}

bool decryptDerivedEmbeddedPayload(
    const QString& payload,
    int encodedKeyLength,
    QString& plaintext
) {
    if (encodedKeyLength < 0 || encodedKeyLength > payload.size() || encodedKeyLength % 2 != 0) {
        return false;
    }

    for (int start = 0; start < 12; ++start) {
        QString encodedText;
        QString encodedKey;
        if (!unweaveKey(payload, encodedKeyLength, start, true, encodedText, encodedKey)) {
            continue;
        }

        const QByteArray keyBytes = QByteArray::fromHex(encodedKey.toLatin1());
        if (keyBytes.toHex() != encodedKey.toLatin1().toLower()
            || startIndexForKey(keyBytes) != start) {
            continue;
        }

        const QByteArray encryptedBytes = QByteArray::fromBase64(encodedText.toLatin1());
        if (encryptedBytes.toBase64() != encodedText.toLatin1()) {
            continue;
        }

        plaintext = QString::fromUtf8(decryptWithSalt(encryptedBytes, keyBytes));
        return true;
    }
    return false;
}

bool decryptStandalonePayload(const QString& ciphertext, QString& plaintext) {
    static const QRegularExpression legacyHeaderPattern(QStringLiteral("^SF1:(\\d+):(\\d+):"));
    const auto legacyHeader = legacyHeaderPattern.match(ciphertext);
    if (legacyHeader.hasMatch()) {
        bool startOk = false;
        bool keyLengthOk = false;
        const int start = legacyHeader.captured(1).toInt(&startOk);
        const int encodedKeyLength = legacyHeader.captured(2).toInt(&keyLengthOk);
        if (!startOk || !keyLengthOk) {
            return false;
        }
        return decryptEmbeddedPayload(
            ciphertext.mid(legacyHeader.capturedEnd()),
            start,
            encodedKeyLength,
            true,
            plaintext
        );
    }

    static const QRegularExpression currentHeaderPattern(
        QStringLiteral("^(1|2):(\\d+):(\\d+):(W:)?"));
    const auto currentHeader = currentHeaderPattern.match(ciphertext);
    if (!currentHeader.hasMatch()) {
        return false;
    }
    bool versionOk = false;
    const int version = currentHeader.captured(1).toInt(&versionOk);
    bool startOk = false;
    const int start = currentHeader.captured(2).toInt(&startOk);
    bool keyLengthOk = false;
    const int encodedKeyLength = currentHeader.captured(3).toInt(&keyLengthOk);
    const bool wrappedLayout = version == 2 || !currentHeader.captured(4).isEmpty();
    return versionOk && startOk && keyLengthOk && decryptEmbeddedPayload(
        ciphertext.mid(currentHeader.capturedEnd()),
        start,
        encodedKeyLength,
        !wrappedLayout,
        plaintext
    );
}

QByteArray recoverSaltBytes(const QString& payload, int fibonacciCount, int insertIndex) {
    for (int saltLength = 1; saltLength <= payload.size(); ++saltLength) {
        const SplitPayload split = splitEmbeddedPayload(payload, saltLength, fibonacciCount, insertIndex);
        const QByteArray candidateSalt = QByteArray::fromBase64(split.saltPayload.toUtf8());
        if (candidateSalt.isEmpty()) {
            continue;
        }

        const QByteArray candidateCiphertext = QByteArray::fromBase64(split.cipherPayload.toUtf8());
        if (candidateCiphertext.isEmpty()) {
            continue;
        }

        const QByteArray candidatePlaintext = decryptWithSalt(candidateCiphertext, candidateSalt);
        const QString candidateText = QString::fromUtf8(candidatePlaintext);
        bool mostlyPrintable = true;
        for (const QChar& ch : candidateText) {
            if (!ch.isPrint() && !ch.isSpace()) {
                mostlyPrintable = false;
                break;
            }
        }

        if (mostlyPrintable) {
            return candidateSalt;
        }
    }

    return {};
}

struct PayloadBundle {
    QString body;
    int start;
    int encodedKeyLength;
    bool wrappedLayout;
};

PayloadBundle buildPayload(const QString& plaintext, const QString& key) {
    const QByteArray saltBytes = key.toUtf8();
    const QByteArray plaintextBytes = plaintext.toUtf8();
    const QByteArray encryptedBytes = encryptWithSalt(plaintextBytes, saltBytes);
    const QString encodedSalt = QString::fromLatin1(saltBytes.toHex());
    const QString encodedText = QString::fromLatin1(encryptedBytes.toBase64());
    const int start = startIndexForKey(saltBytes);
    const bool wrappedLayout = encodedSalt.size() > encodedText.size();
    const QString embedded = weaveKey(encodedText, encodedSalt, start, wrappedLayout);
    return {embedded, start, static_cast<int>(encodedSalt.size()), wrappedLayout};
}

QString trailerParameters(const PayloadBundle& payload) {
    return QStringLiteral("%1,%2")
        .arg(payload.start)
        .arg(payload.encodedKeyLength);
}

bool parseTrailerParameter(const QString& parameter, QString& hexIndex, int& fibonacciCount) {
    if (parameter.isEmpty()) {
        return false;
    }

    hexIndex = parameter.left(1);
    QString digits;
    int cursor = 1;
    while (cursor < parameter.size() && parameter.at(cursor).isDigit()) {
        digits.append(parameter.at(cursor));
        ++cursor;
    }

    if (hexIndex.isEmpty() || digits.isEmpty()) {
        return false;
    }

    fibonacciCount = digits.toInt();
    return true;
}

} // namespace

QString SaltedFibonacciCipher::name() const {
    return QStringLiteral("XOR Key-Weaver");
}

QString SaltedFibonacciCipher::id() const {
    return QStringLiteral("xor_key_weaver");
}

QString SaltedFibonacciCipher::metadataTag() const {
    return QStringLiteral("XFA");
}

QString SaltedFibonacciCipher::parameterPlaceholder() const {
    return QStringLiteral("key");
}

QString SaltedFibonacciCipher::encrypt(const QString& plaintext) const {
    const PayloadBundle payload = buildPayload(plaintext, QStringLiteral("tezcrypt"));
    return QStringLiteral("%1:%2:%3:%4%5")
        .arg(EncryptionAlgorithm::TAGGED_FORMAT_VERSION)
        .arg(payload.start)
        .arg(payload.encodedKeyLength)
        .arg(payload.wrappedLayout ? QStringLiteral("W:") : QString())
        .arg(payload.body);
}

QString SaltedFibonacciCipher::decrypt(const QString& ciphertext) const {
    QString plaintext;
    return decryptStandalonePayload(ciphertext, plaintext) ? plaintext : ciphertext;
}

QString SaltedFibonacciCipher::encryptSegment(const QString& segment, const QStringList& params) const {
    if (segment.isEmpty()) {
        return {};
    }

    const QString key = params.value(0, QStringLiteral("tezcrypt"));
    return buildPayload(segment, key).body;
}

QString SaltedFibonacciCipher::encryptTaggedSegment(
    const QString& segment,
    const QStringList& params,
    QString& trailerParams
) const {
    if (segment.isEmpty()) {
        const QByteArray keyBytes = params.value(0, QStringLiteral("tezcrypt")).toUtf8();
        trailerParams = QStringLiteral("%1,%2,%3")
            .arg(EncryptionAlgorithm::TAGGED_FORMAT_VERSION)
            .arg(startIndexForKey(keyBytes))
            .arg(keyBytes.toHex().size());
        if (!keyBytes.isEmpty()) {
            trailerParams += QStringLiteral(",W");
        }
        return {};
    }

    const QString key = params.value(0, QStringLiteral("tezcrypt"));
    const PayloadBundle payload = buildPayload(segment, key);
    trailerParams = QStringLiteral("%1,%2%3")
        .arg(EncryptionAlgorithm::TAGGED_FORMAT_VERSION)
        .arg(trailerParameters(payload))
        .arg(payload.wrappedLayout ? QStringLiteral(",W") : QString());
    return payload.body;
}

QString SaltedFibonacciCipher::trailerParameter(const QString& segment, const QStringList& params) const {
    if (segment.isEmpty()) {
        return QStringLiteral("0");
    }

    const QString key = params.value(0, QStringLiteral("tezcrypt"));
    const PayloadBundle payload = buildPayload(segment, key);
    return trailerParameters(payload)
        + (payload.wrappedLayout ? QStringLiteral(",W") : QString());
}

int SaltedFibonacciCipher::trailerParameterCount() const {
    return 2;
}

QString SaltedFibonacciCipher::decryptTaggedSegment(
    const QString& segment,
    const QStringList& params
) const {
    if (params.size() == 4
        && (params.at(0) != QStringLiteral("1") || params.at(3) != QStringLiteral("W"))) {
        return segment;
    }

    if (params.size() == 3 || params.size() == 4) {
        bool versionOk = false;
        bool startOk = false;
        bool keyLengthOk = false;
        const int version = params.at(0).toInt(&versionOk);
        const int start = params.at(1).toInt(&startOk);
        const int encodedKeyLength = params.at(2).toInt(&keyLengthOk);
        const bool wrappedLayout = version == 2
            || (version == 1 && params.size() == 4 && params.at(3) == QStringLiteral("W"));
        if (versionOk && startOk && keyLengthOk && (version == 1 || version == 2)
            && (params.size() == 3 || (version == 1 && wrappedLayout))) {
            QString plaintext;
            if (decryptEmbeddedPayload(
                    segment,
                    start,
                    encodedKeyLength,
                    !wrappedLayout,
                    plaintext)) {
                return plaintext;
            }
            return segment;
        }
    }

    return EncryptionAlgorithm::decryptTaggedSegment(segment, params);
}

QString SaltedFibonacciCipher::decryptSegment(const QString& segment, const QStringList& params) const {
    QString versionOnePlaintext;
    if (segment.startsWith(QStringLiteral("1:")) || segment.startsWith(QStringLiteral("2:"))
        || segment.startsWith(QStringLiteral("SF1:"))) {
        return decryptStandalonePayload(segment, versionOnePlaintext) ? versionOnePlaintext : segment;
    }

    if (params.size() == 2 || params.size() == 3) {
        bool startOk = false;
        bool keyLengthOk = false;
        const int start = params.at(0).toInt(&startOk);
        const int encodedKeyLength = params.at(1).toInt(&keyLengthOk);
        const bool wrappedLayout = params.size() == 3 && params.at(2) == QStringLiteral("W");
        if (startOk && keyLengthOk
            && decryptEmbeddedPayload(segment, start, encodedKeyLength, !wrappedLayout, versionOnePlaintext)) {
            return versionOnePlaintext;
        }
        return segment;
    }

    if ((params.size() == 3 || params.size() == 4)
        && (params.at(0) == QStringLiteral("1") || params.at(0) == QStringLiteral("2")
            || params.at(0) == QStringLiteral("SF1"))) {
        bool startOk = false;
        bool keyLengthOk = false;
        const int start = params.at(1).toInt(&startOk);
        const int encodedKeyLength = params.at(2).toInt(&keyLengthOk);
        const bool wrappedLayout = params.at(0) == QStringLiteral("2")
            || (params.size() == 4 && params.at(3) == QStringLiteral("W"));
        if (startOk && keyLengthOk
            && decryptEmbeddedPayload(segment, start, encodedKeyLength, !wrappedLayout, versionOnePlaintext)) {
            return versionOnePlaintext;
        }
        return segment;
    }

    if (params.size() == 1) {
        bool keyLengthOk = false;
        const int encodedKeyLength = params.first().toInt(&keyLengthOk);
        if (keyLengthOk
            && decryptDerivedEmbeddedPayload(segment, encodedKeyLength, versionOnePlaintext)) {
            return versionOnePlaintext;
        }
    }

    QString hexIndex;
    int fibonacciCount = 0;
    QString payload = segment;

    if (!params.isEmpty() && parseTrailerParameter(params.value(0), hexIndex, fibonacciCount)) {
        // Use the trailer parameter for the salt-derived prefix metadata.
    } else {
        QString fallback = segment;
        if (fallback.size() >= 2 && fallback.at(0).isLetterOrNumber()) {
            hexIndex = fallback.left(1);
            int cursor = 1;
            QString digits;
            while (cursor < fallback.size() && fallback.at(cursor).isDigit()) {
                digits.append(fallback.at(cursor));
                ++cursor;
            }
            fibonacciCount = digits.toInt();
            fallback.remove(0, cursor);
            if (!fallback.isEmpty()) {
                payload = fallback;
            }
        }
    }

    if (hexIndex.isEmpty() || fibonacciCount <= 0) {
        return payload;
    }

    const int insertIndex = hexIndex.toInt(nullptr, 16);
    const QByteArray saltBytes = recoverSaltBytes(payload, fibonacciCount, insertIndex);
    if (saltBytes.isEmpty()) {
        return payload;
    }

    const SplitPayload split = splitEmbeddedPayload(payload, saltBytes.size(), fibonacciCount, insertIndex);
    const QByteArray decoded = QByteArray::fromBase64(split.cipherPayload.toUtf8());
    return QString::fromUtf8(decryptWithSalt(decoded, saltBytes));
}
