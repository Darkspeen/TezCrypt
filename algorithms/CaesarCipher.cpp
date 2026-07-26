#include "CaesarCipher.h"

QString CaesarCipher::name() const {
    return QStringLiteral("Caesar Cipher");
}

QString CaesarCipher::metadataTag() const {
    return QStringLiteral("CSA");
}

QString CaesarCipher::parameterPlaceholder() const {
    return QStringLiteral("offset");
}

QString CaesarCipher::encrypt(const QString& plaintext) const {
    QString result;
    result.reserve(plaintext.size());
    
    for (const QChar& ch : plaintext) {
        result.append(shiftChar(ch, DEFAULT_SHIFT));
    }
    
    return result;
}

QString CaesarCipher::decrypt(const QString& ciphertext) const {
    QString result;
    result.reserve(ciphertext.size());
    
    for (const QChar& ch : ciphertext) {
        result.append(shiftChar(ch, -DEFAULT_SHIFT));
    }
    
    return result;
}

QString CaesarCipher::encryptSegment(const QString& segment, const QStringList& params) const {
    int shift = DEFAULT_SHIFT;
    if (!params.isEmpty()) {
        bool ok = false;
        int parsed = params.first().toInt(&ok);
        if (ok) {
            shift = parsed;
        }
    }

    QString result;
    result.reserve(segment.size());
    for (const QChar& ch : segment) {
        result.append(shiftChar(ch, shift));
    }
    return result;
}

QString CaesarCipher::decryptSegment(const QString& segment, const QStringList& params) const {
    int shift = DEFAULT_SHIFT;
    if (!params.isEmpty()) {
        bool ok = false;
        int parsed = params.first().toInt(&ok);
        if (ok) {
            shift = parsed;
        }
    }

    QString result;
    result.reserve(segment.size());
    for (const QChar& ch : segment) {
        result.append(shiftChar(ch, -shift));
    }
    return result;
}

QChar CaesarCipher::shiftChar(QChar ch, int shift) const {
    if (ch.isUpper()) {
        int pos = ch.unicode() - 'A';
        pos = (pos + shift) % 26;
        if (pos < 0) pos += 26;
        return QChar('A' + pos);
    } else if (ch.isLower()) {
        int pos = ch.unicode() - 'a';
        pos = (pos + shift) % 26;
        if (pos < 0) pos += 26;
        return QChar('a' + pos);
    }
    return ch;
}
