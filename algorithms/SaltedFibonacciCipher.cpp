#include "SaltedFibonacciCipher.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QRandomGenerator>
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

QVector<int> buildKeyPositions(int baseLength, int keyLength, int start) {
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

QString weaveKey(const QString& encodedText, const QString& encodedKey, int start) {
    const int baseLength = encodedText.size();
    const int totalLength = baseLength + encodedKey.size();
    const QVector<int> positions = buildKeyPositions(baseLength, encodedKey.size(), start);

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
    QString& encodedText,
    QString& encodedKey
) {
    if (encodedKeyLength < 0 || encodedKeyLength > payload.size() || encodedKeyLength % 2 != 0
        || start < 0 || start >= 12) {
        return false;
    }

    const int baseLength = payload.size() - encodedKeyLength;
    const QVector<int> positions = buildKeyPositions(baseLength, encodedKeyLength, start);
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
    QString& plaintext
) {
    QString encodedText;
    QString encodedKey;
    if (!unweaveKey(payload, encodedKeyLength, start, encodedText, encodedKey)) {
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

bool decryptStandalonePayload(const QString& ciphertext, QString& plaintext) {
    static const QRegularExpression headerPattern(QStringLiteral("^(?:1|SF1):(\\d+):(\\d+):"));
    const auto header = headerPattern.match(ciphertext);
    if (!header.hasMatch()) {
        return false;
    }

    bool startOk = false;
    bool keyLengthOk = false;
    const int start = header.captured(1).toInt(&startOk);
    const int encodedKeyLength = header.captured(2).toInt(&keyLengthOk);
    if (!startOk || !keyLengthOk) {
        return false;
    }

    return decryptEmbeddedPayload(ciphertext.mid(header.capturedEnd()), start, encodedKeyLength, plaintext);
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

PayloadBundle buildPayload(const QString& plaintext, const QString& key) {
    const QByteArray saltBytes = key.toUtf8();
    const QByteArray plaintextBytes = plaintext.toUtf8();
    const QByteArray encryptedBytes = encryptWithSalt(plaintextBytes, saltBytes);
    const QString encodedSalt = QString::fromLatin1(saltBytes.toHex());
    const QString encodedText = QString::fromLatin1(encryptedBytes.toBase64());
    const int start = static_cast<int>(QRandomGenerator::system()->bounded(12U));
    const QString embedded = weaveKey(encodedText, encodedSalt, start);
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
    return QStringLiteral("Salted Fibonacci Cipher");
}

QString SaltedFibonacciCipher::id() const {
    return QStringLiteral("salted_fibonacci_cipher");
}

QString SaltedFibonacciCipher::metadataTag() const {
    // This algorithm follows the X/Y/Z wrapper convention:
    // X = AES, Y = Fibonacci, Z = Variation A => AFA.
    return QStringLiteral("AFA");
}

QString SaltedFibonacciCipher::parameterPlaceholder() const {
    return QStringLiteral("key");
}

QString SaltedFibonacciCipher::encrypt(const QString& plaintext) const {
    const PayloadBundle payload = buildPayload(plaintext, QStringLiteral("tezcrypt"));
    return QStringLiteral("1:%1:%2:%3")
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
    if (segment.isEmpty()) {
        trailerParams = QStringLiteral("1,0,0");
        return {};
    }

    const QString key = params.value(0, QStringLiteral("tezcrypt"));
    const PayloadBundle payload = buildPayload(segment, key);
    trailerParams = QStringLiteral("%1,%2")
        .arg(EncryptionAlgorithm::TAGGED_FORMAT_VERSION)
        .arg(trailerParameters(payload));
    return payload.body;
}

QString SaltedFibonacciCipher::trailerParameter(const QString& segment, const QStringList& params) const {
    if (segment.isEmpty()) {
        return QStringLiteral("0,0");
    }

    const QString key = params.value(0, QStringLiteral("tezcrypt"));
    return trailerParameters(buildPayload(segment, key));
}

int SaltedFibonacciCipher::trailerParameterCount() const {
    return 2;
}

QString SaltedFibonacciCipher::decryptSegment(const QString& segment, const QStringList& params) const {
    QString versionOnePlaintext;
    if (segment.startsWith(QStringLiteral("1:")) || segment.startsWith(QStringLiteral("SF1:"))) {
        return decryptStandalonePayload(segment, versionOnePlaintext) ? versionOnePlaintext : segment;
    }

    if (params.size() == 2) {
        bool startOk = false;
        bool keyLengthOk = false;
        const int start = params.at(0).toInt(&startOk);
        const int encodedKeyLength = params.at(1).toInt(&keyLengthOk);
        if (startOk && keyLengthOk
            && decryptEmbeddedPayload(segment, start, encodedKeyLength, versionOnePlaintext)) {
            return versionOnePlaintext;
        }
        return segment;
    }

    if (params.size() == 3
        && (params.at(0) == QStringLiteral("1") || params.at(0) == QStringLiteral("SF1"))) {
        bool startOk = false;
        bool keyLengthOk = false;
        const int start = params.at(1).toInt(&startOk);
        const int encodedKeyLength = params.at(2).toInt(&keyLengthOk);
        if (startOk && keyLengthOk
            && decryptEmbeddedPayload(segment, start, encodedKeyLength, versionOnePlaintext)) {
            return versionOnePlaintext;
        }
        return segment;
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
