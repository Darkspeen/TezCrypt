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

QVector<int> buildChunkPositions(int baseLength, int keyLength, int start) {
    QVector<int> positions;
    qint64 position = start;
    qint64 first = 1;
    qint64 second = 1;

    while (positions.size() < keyLength) {
        position += first;
        if (position >= baseLength) {
            break;
        }
        positions.append(static_cast<int>(position));

        const qint64 next = std::min<qint64>(baseLength, first + second);
        first = second;
        second = next;
    }

    return positions;
}

int chunkLength(int keyLength, int chunkCount) {
    return keyLength / chunkCount + (keyLength % chunkCount != 0);
}

QString weaveKeyInChunks(const QString& encodedText, const QString& encodedKey, int start) {
    const QVector<int> positions = buildChunkPositions(
        encodedText.size(),
        encodedKey.size(),
        start
    );
    if (positions.size() < 2) {
        return encodedText + encodedKey;
    }

    QString result;
    result.reserve(encodedText.size() + encodedKey.size());
    int textIndex = 0;
    int keyIndex = 0;
    int chunkIndex = 0;
    const int chunkSize = chunkLength(encodedKey.size(), positions.size());

    for (int position = 0; position < encodedText.size(); ++position) {
        if (chunkIndex < positions.size() && positions.at(chunkIndex) == position) {
            const int currentChunkSize = std::min(
                chunkSize,
                static_cast<int>(encodedKey.size()) - keyIndex
            );
            result.append(encodedKey.mid(keyIndex, currentChunkSize));
            keyIndex += currentChunkSize;
            ++chunkIndex;
        }
        result.append(encodedText.at(textIndex++));
    }

    return result;
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
    bool chunkedLayout,
    QString& encodedText,
    QString& encodedKey
) {
    if (encodedKeyLength < 0 || encodedKeyLength > payload.size() || encodedKeyLength % 2 != 0
        || start < 0 || start >= 12) {
        return false;
    }

    const int baseLength = payload.size() - encodedKeyLength;
    if (chunkedLayout) {
        const QVector<int> chunkPositions = buildChunkPositions(baseLength, encodedKeyLength, start);
        if (chunkPositions.size() < 2) {
            encodedText = payload.left(baseLength);
            encodedKey = payload.mid(baseLength);
            return true;
        }

        const int chunkSize = chunkLength(encodedKeyLength, chunkPositions.size());
        int payloadIndex = 0;
        int chunkIndex = 0;
        for (int textPosition = 0; textPosition < baseLength; ++textPosition) {
            if (chunkIndex < chunkPositions.size()
                && chunkPositions.at(chunkIndex) == textPosition) {
                const int currentChunkSize = std::min(
                    chunkSize,
                    encodedKeyLength - static_cast<int>(encodedKey.size())
                );
                encodedKey.append(payload.mid(payloadIndex, currentChunkSize));
                payloadIndex += currentChunkSize;
                ++chunkIndex;
            }
            encodedText.append(payload.at(payloadIndex++));
        }
        return chunkIndex == chunkPositions.size() && encodedKey.size() == encodedKeyLength;
    }

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
    QString& plaintext,
    bool chunkedLayout = false,
    bool base64Key = false
) {
    QString encodedText;
    QString encodedKey;
    if (!unweaveKey(
            payload,
            encodedKeyLength,
            start,
            legacyLayout,
            chunkedLayout,
            encodedText,
            encodedKey)) {
        return false;
    }

    QByteArray keyBytes;
    if (base64Key) {
        const QByteArray hexKey = QByteArray::fromBase64(encodedKey.toLatin1());
        if (hexKey.toBase64() != encodedKey.toLatin1()
            || hexKey.size() % 2 != 0
            || QByteArray::fromHex(hexKey).toHex() != hexKey.toLower()) {
            return false;
        }
        keyBytes = QByteArray::fromHex(hexKey);
    } else {
        keyBytes = QByteArray::fromHex(encodedKey.toLatin1());
        if (keyBytes.toHex() != encodedKey.toLatin1().toLower()) {
            return false;
        }
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
        if (!unweaveKey(
                payload,
                encodedKeyLength,
                start,
                true,
                false,
                encodedText,
                encodedKey)) {
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
};

PayloadBundle buildPayload(
    const QString& plaintext,
    const QString& key,
    bool chunkedLayout = false,
    bool base64Key = false
) {
    const QByteArray saltBytes = key.toUtf8();
    const QByteArray plaintextBytes = plaintext.toUtf8();
    const QByteArray encryptedBytes = encryptWithSalt(plaintextBytes, saltBytes);
    const QByteArray hexKey = saltBytes.toHex();
    const QString encodedSalt = QString::fromLatin1(
        base64Key ? hexKey.toBase64() : hexKey
    );
    const QString encodedText = QString::fromLatin1(encryptedBytes.toBase64());
    const int start = startIndexForKey(saltBytes);
    const QString embedded = chunkedLayout
        ? weaveKeyInChunks(encodedText, encodedSalt, start)
        : weaveKey(encodedText, encodedSalt, start, false);
    return {embedded, start, static_cast<int>(encodedSalt.size())};
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

QStringList SaltedFibonacciCipher::metadataTagVariants() const {
    return {QStringLiteral("XFA"), QStringLiteral("XFB")};
}

QString SaltedFibonacciCipher::parameterPlaceholder() const {
    return QStringLiteral("key");
}

QString SaltedFibonacciCipher::encrypt(const QString& plaintext) const {
    const PayloadBundle payload = buildPayload(plaintext, QStringLiteral("tezcrypt"));
    return QStringLiteral("%1:%2:%3:%4")
        .arg(EncryptionAlgorithm::TAGGED_FORMAT_VERSION)
        .arg(payload.start)
        .arg(payload.encodedKeyLength)
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
    return encryptTaggedSegmentForTag(segment, params, metadataTag(), trailerParams);
}

QString SaltedFibonacciCipher::encryptTaggedSegmentForTag(
    const QString& segment,
    const QStringList& params,
    const QString& tag,
    QString& trailerParams
) const {
    const bool wrappedLayout = tag.compare(QStringLiteral("XFB"), Qt::CaseInsensitive) == 0;
    if (segment.isEmpty()) {
        const QByteArray keyBytes = params.value(0, QStringLiteral("tezcrypt")).toUtf8();
        const QByteArray hexKey = keyBytes.toHex();
        const QString encodedKey = QString::fromLatin1(
            wrappedLayout ? hexKey.toBase64() : hexKey
        );
        trailerParams = QStringLiteral("%1,%2,%3")
            .arg(EncryptionAlgorithm::TAGGED_FORMAT_VERSION)
            .arg(startIndexForKey(keyBytes))
            .arg(encodedKey.size());
        return {};
    }

    const QString key = params.value(0, QStringLiteral("tezcrypt"));
    const PayloadBundle payload = buildPayload(segment, key, wrappedLayout, wrappedLayout);
    trailerParams = QStringLiteral("%1,%2")
        .arg(EncryptionAlgorithm::TAGGED_FORMAT_VERSION)
        .arg(trailerParameters(payload));
    return payload.body;
}

QString SaltedFibonacciCipher::trailerParameter(const QString& segment, const QStringList& params) const {
    if (segment.isEmpty()) {
        return QStringLiteral("0");
    }

    const QString key = params.value(0, QStringLiteral("tezcrypt"));
    const PayloadBundle payload = buildPayload(segment, key);
    return trailerParameters(payload);
}

int SaltedFibonacciCipher::trailerParameterCount() const {
    return 2;
}

QString SaltedFibonacciCipher::decryptTaggedSegment(
    const QString& segment,
    const QStringList& params
) const {
    return decryptTaggedSegmentForTag(segment, params, metadataTag());
}

QString SaltedFibonacciCipher::decryptTaggedSegmentForTag(
    const QString& segment,
    const QStringList& params,
    const QString& tag
) const {
    if (params.size() == 3 || params.size() == 4) {
        bool versionOk = false;
        bool startOk = false;
        bool keyLengthOk = false;
        const int version = params.at(0).toInt(&versionOk);
        const int start = params.at(1).toInt(&startOk);
        const int encodedKeyLength = params.at(2).toInt(&keyLengthOk);
        const bool legacyWrappedMarker = params.size() == 4 && params.at(3) == QStringLiteral("W");
        const bool wrappedLayout = tag.compare(QStringLiteral("XFB"), Qt::CaseInsensitive) == 0
            || version == 2
            || legacyWrappedMarker;
        if (versionOk && startOk && keyLengthOk && (version == 1 || version == 2)
            && (params.size() == 3 || legacyWrappedMarker)) {
            QString plaintext;
            if (decryptEmbeddedPayload(
                    segment,
                    start,
                    encodedKeyLength,
                    version == 1 && !wrappedLayout,
                    plaintext,
                    tag.compare(QStringLiteral("XFB"), Qt::CaseInsensitive) == 0,
                    tag.compare(QStringLiteral("XFB"), Qt::CaseInsensitive) == 0)) {
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
