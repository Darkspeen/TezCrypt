#include "EncryptionManager.h"
#include <QRegularExpression>
#include <QVector>

void EncryptionManager::registerAlgorithm(EncryptionAlgorithmPtr algorithm) {
    if (algorithm) {
        m_algorithms[algorithm->id().toStdString()] = algorithm;
        const QString tag = algorithm->metadataTag().toLower();
        if (!tag.isEmpty()) {
            m_algorithmTags[tag.toStdString()] = algorithm;
        }
    }
}

EncryptionAlgorithmPtr EncryptionManager::getAlgorithm(const QString& id) const {
    auto it = m_algorithms.find(id.toStdString());
    if (it != m_algorithms.end()) {
        return it->second;
    }
    return nullptr;
}

EncryptionAlgorithmPtr EncryptionManager::getAlgorithmByTag(const QString& tag) const {
    QString normalizedTag = tag.toLower();
    auto it = m_algorithmTags.find(normalizedTag.toStdString());
    if (it != m_algorithmTags.end()) {
        return it->second;
    }
    return nullptr;
}

QStringList EncryptionManager::getAlgorithmNames() const {
    QStringList names;
    for (const auto& [id, algorithm] : m_algorithms) {
        names.append(algorithm->name());
    }
    names.sort();
    return names;
}

QStringList EncryptionManager::getAlgorithmIds() const {
    QStringList ids;
    for (const auto& [id, algorithm] : m_algorithms) {
        ids.append(algorithm->id());
    }
    ids.sort();
    return ids;
}

bool EncryptionManager::hasAlgorithm(const QString& id) const {
    return m_algorithms.find(id.toStdString()) != m_algorithms.end();
}

bool EncryptionManager::hasTaggedText(const QString& text) const {
    static const QRegularExpression tagPattern(QStringLiteral("\\|x[0-9A-Fa-f]{6}x[0-9A-Fa-f]{6}#[A-Za-z0-9]{2,8}(?::[^|]*)?\\|$") );
    return tagPattern.match(text).hasMatch();
}

QString EncryptionManager::encryptTaggedText(const QString& plaintext) const {
    static const QRegularExpression openPattern(QStringLiteral("\\|#([A-Za-z0-9]{2,8})(?::([^|]*))?\\|"));
    QString result;
    result.reserve(plaintext.size());

    int cursor = 0;
    QStringList trailers;
    while (cursor < plaintext.size()) {
        auto match = openPattern.match(plaintext, cursor);
        if (!match.hasMatch()) {
            result += plaintext.mid(cursor);
            break;
        }

        int openStart = match.capturedStart();
        int openEnd = match.capturedEnd();
        result += plaintext.mid(cursor, openStart - cursor);

        QString algorithmTag = match.captured(1);
        QString parameterText = match.captured(2);
        QStringList parameters;
        if (!parameterText.isEmpty()) {
            parameters = parameterText.split(',', Qt::KeepEmptyParts);
        }

        int contentStart = openEnd;
        int closeIndex = plaintext.indexOf(QStringLiteral("|#|"), contentStart);
        if (closeIndex < 0) {
            result += plaintext.mid(openStart);
            break;
        }

        QString segment = plaintext.mid(contentStart, closeIndex - contentStart);
        auto algorithm = getAlgorithmByTag(algorithmTag);
        if (!algorithm) {
            result += plaintext.mid(openStart, closeIndex + 3 - openStart);
            cursor = closeIndex + 3;
            continue;
        }

        QString encryptedSegment = algorithm->encryptSegment(segment, parameters);
        int segmentStart = result.size();
        result += encryptedSegment;
        int segmentEnd = result.size();

        trailers.append(QStringLiteral("|x%1x%2#%3%4|")
            .arg(segmentStart, 6, 16, QLatin1Char('0'))
            .arg(segmentEnd, 6, 16, QLatin1Char('0'))
            .arg(algorithmTag)
            .arg(parameterText.isEmpty() ? QString() : QStringLiteral(":") + parameterText));

        cursor = closeIndex + 3;
    }

    for (const QString& trailer : trailers) {
        result += trailer;
    }

    return result;
}

QString EncryptionManager::decryptTaggedText(const QString& ciphertext) const {
    static const QRegularExpression tagPattern(QStringLiteral("^x([0-9A-Fa-f]{6})x([0-9A-Fa-f]{6})#([A-Za-z0-9]{2,8})(?::([^|]*))?$") );
    QString body = ciphertext;

    struct TaggedRange {
        int start;
        int end;
        QString algorithmTag;
        QStringList params;
    };

    QVector<TaggedRange> ranges;
    while (!body.isEmpty() && body.endsWith(QLatin1Char('|'))) {
        int closingPipe = body.lastIndexOf(QLatin1Char('|'), body.size() - 2);
        if (closingPipe < 0) {
            break;
        }

        QString inner = body.mid(closingPipe + 1, body.size() - closingPipe - 2);
        auto match = tagPattern.match(inner);
        if (!match.hasMatch()) {
            break;
        }

        bool okStart = false;
        bool okEnd = false;
        int start = match.captured(1).toInt(&okStart, 16);
        int end = match.captured(2).toInt(&okEnd, 16);
        QString algorithmTag = match.captured(3);
        QString parameterText = match.captured(4);
        QStringList params;
        if (!parameterText.isEmpty()) {
            params = parameterText.split(',', Qt::KeepEmptyParts);
        }

        if (!okStart || !okEnd) {
            break;
        }

        ranges.prepend({start, end, algorithmTag, params});
        body.chop(body.size() - closingPipe);
    }

    if (ranges.isEmpty()) {
        return ciphertext;
    }

    QString result = body;
    for (const TaggedRange& taggedRange : ranges) {
        auto algorithm = getAlgorithmByTag(taggedRange.algorithmTag);
        if (!algorithm) {
            continue;
        }

        if (taggedRange.start < 0 || taggedRange.end > result.size() || taggedRange.start > taggedRange.end) {
            continue;
        }

        QString segment = result.mid(taggedRange.start, taggedRange.end - taggedRange.start);
        QString decryptedSegment = algorithm->decryptSegment(segment, taggedRange.params);
        result.replace(taggedRange.start, taggedRange.end - taggedRange.start, decryptedSegment);
    }

    return result;
}

size_t EncryptionManager::algorithmCount() const {
    return m_algorithms.size();
}
