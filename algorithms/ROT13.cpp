#include "ROT13.h"

QString ROT13::name() const {
    return QStringLiteral("ROT13");
}

QString ROT13::metadataTag() const {
    return QStringLiteral("RSA");
}

QString ROT13::parameterPlaceholder() const {
    return QString();
}

QString ROT13::encrypt(const QString& plaintext) const {
    QString result;
    result.reserve(plaintext.size());
    
    for (const QChar& ch : plaintext) {
        result.append(rotateChar(ch));
    }
    
    return result;
}

QString ROT13::decrypt(const QString& ciphertext) const {
    // ROT13 is symmetric: decryption is the same as encryption
    return encrypt(ciphertext);
}

QChar ROT13::rotateChar(QChar ch) const {
    if (ch.isUpper()) {
        int pos = ch.unicode() - 'A';
        pos = (pos + ROT13_SHIFT) % 26;
        return QChar('A' + pos);
    } else if (ch.isLower()) {
        int pos = ch.unicode() - 'a';
        pos = (pos + ROT13_SHIFT) % 26;
        return QChar('a' + pos);
    }
    return ch;
}
