#include "SaltedFibonacciCipher.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QStringList>
#include <QVector>
#include <algorithm>

namespace {

QString computeHexIndex(const QString& salt) {
    const QByteArray saltBytes = salt.toUtf8();
    const QByteArray digest = QCryptographicHash::hash(saltBytes, QCryptographicHash::Sha256);
    if (digest.isEmpty()) {
        return QStringLiteral("0");
    }

    const int byteValue = static_cast<unsigned char>(digest.at(0));
    return QString::number(byteValue % 16, 16).toUpper();
}

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

int computeFibonacciCount(int baseLength, int saltLength, int insertIndex) {
    Q_UNUSED(insertIndex);
    if (baseLength <= 0 || saltLength <= 0) {
        return 0;
    }

    QVector<int> fibonacci = {1, 1};
    int count = 0;
    int total = 0;
    while (count < saltLength) {
        const int step = fibonacci[count % fibonacci.size()];
        if (total + step > baseLength + saltLength) {
            break;
        }
        total += step;
        ++count;
        if (fibonacci.size() <= count) {
            fibonacci.append(fibonacci[fibonacci.size() - 1] + fibonacci[fibonacci.size() - 2]);
        }
    }

    return std::min(count, saltLength);
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

QString injectSaltIntoBase64(const QString& encodedText, const QString& encodedSalt, int fibonacciCount, int insertIndex) {
    if (encodedSalt.isEmpty()) {
        return encodedText;
    }

    const int baseLength = encodedText.size();
    const int totalLength = baseLength + encodedSalt.size();
    const QVector<int> positions = buildInsertionPositions(baseLength, encodedSalt.size(), fibonacciCount, insertIndex);

    QString result(totalLength, QChar::Null);
    int saltIndex = 0;
    int textIndex = 0;
    for (int i = 0; i < totalLength; ++i) {
        if (saltIndex < positions.size() && positions.at(saltIndex) == i) {
            result[i] = encodedSalt.at(saltIndex);
            ++saltIndex;
        } else {
            result[i] = encodedText.at(textIndex++);
        }
    }

    return result;
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

struct PayloadBundle {
    QString body;
    QString trailerParameter;
};

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

PayloadBundle buildPayload(const QString& plaintext, const QString& salt) {
    const QByteArray saltBytes = salt.toUtf8();
    const QByteArray plaintextBytes = plaintext.toUtf8();
    const QByteArray encryptedBytes = encryptWithSalt(plaintextBytes, saltBytes);
    const QString encodedSalt = QString::fromUtf8(saltBytes.toBase64());
    const QString encodedText = QString::fromUtf8(encryptedBytes.toBase64());
    const QString hexIndex = computeHexIndex(salt);
    const int insertIndex = hexIndex.toInt(nullptr, 16);
    const int fibonacciCount = computeFibonacciCount(encodedText.size(), encodedSalt.size(), insertIndex);

    const QString embedded = injectSaltIntoBase64(encodedText, encodedSalt, fibonacciCount, insertIndex);
    return {embedded, QStringLiteral("%1%2").arg(hexIndex).arg(fibonacciCount)};
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
    return QStringLiteral("salt");
}

QString SaltedFibonacciCipher::encrypt(const QString& plaintext) const {
    return encryptSegment(plaintext, {QStringLiteral("tezcrypt")});
}

QString SaltedFibonacciCipher::decrypt(const QString& ciphertext) const {
    return decryptSegment(ciphertext, {QStringLiteral("tezcrypt")});
}

QString SaltedFibonacciCipher::encryptSegment(const QString& segment, const QStringList& params) const {
    if (segment.isEmpty()) {
        return {};
    }

    const QString salt = params.value(0, QStringLiteral("tezcrypt"));
    return buildPayload(segment, salt).body;
}

QString SaltedFibonacciCipher::trailerParameter(const QString& segment, const QStringList& params) const {
    if (segment.isEmpty()) {
        return {};
    }

    const QString salt = params.value(0, QStringLiteral("tezcrypt"));
    return buildPayload(segment, salt).trailerParameter;
}

QString SaltedFibonacciCipher::decryptSegment(const QString& segment, const QStringList& params) const {
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
