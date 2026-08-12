#include "DelimitedWeaverSimpleA.h"
#include <QVector>
#include <QPair>
#include <algorithm>

const QString DelimitedWeaverSimpleA::OpenTagPrefix = QStringLiteral("|#");
const QString DelimitedWeaverSimpleA::Delimiter = QStringLiteral("|#|");
const QString DelimitedWeaverSimpleA::ClosingDelimiter = QStringLiteral("|#|");
const QRegularExpression DelimitedWeaverSimpleA::OpenTagPattern(
    QStringLiteral("(?:\\|\\|#|\\|#)([A-Za-z0-9]{2,8})(?::([^|]*))?(?:\\|\\||\\|)")
);
const QRegularExpression DelimitedWeaverSimpleA::TagSuffixPattern(
    QStringLiteral("^x([0-9A-Fa-f]{6})x([0-9A-Fa-f]{6})#([A-Za-z0-9]{2,8})(?::([^|]*))?$")
);

QString DelimitedWeaverSimpleA::name() const {
    return QStringLiteral("Weaver Simple A");
}

QString DelimitedWeaverSimpleA::id() const {
    return QStringLiteral("weaver_simple_a");
}

QString DelimitedWeaverSimpleA::metadataTag() const {
    return QStringLiteral("WSA");
}

QString DelimitedWeaverSimpleA::parameterPlaceholder() const {
    return QString();
}

QStringList DelimitedWeaverSimpleA::parseParameters(const QString& parameterString) {
    if (parameterString.isEmpty()) {
        return {};
    }
    return parameterString.split(',', Qt::KeepEmptyParts);
}

QString DelimitedWeaverSimpleA::serializeParameters(const QStringList& params) {
    if (params.isEmpty()) {
        return QString();
    }
    return QStringLiteral(":") + params.join(',');
}

QString DelimitedWeaverSimpleA::encrypt(const QString& plaintext) const {
    QString output;
    output.reserve(plaintext.size());

    struct SegmentRange {
        int start;
        int end;
        QString algorithmTag;
        QStringList params;
    };

    QVector<SegmentRange> ranges;
    int cursor = 0;

    while (cursor < plaintext.size()) {
        auto match = OpenTagPattern.match(plaintext, cursor);
        if (!match.hasMatch()) {
            break;
        }

        int openStart = match.capturedStart();
        int openEnd = match.capturedEnd();
        if (openStart != cursor) {
            int nextOpen = plaintext.indexOf(OpenTagPrefix, cursor);
            if (nextOpen < 0) {
                break;
            }
            output += plaintext.mid(cursor, nextOpen - cursor);
            cursor = nextOpen;
            continue;
        }

        output += plaintext.mid(cursor, openStart - cursor);

        QString algorithmTag = match.captured(1);
        QStringList parameters = parseParameters(match.captured(2));
        int segmentStart = openEnd;
        int closeIndex = plaintext.indexOf(ClosingDelimiter, segmentStart);
        if (closeIndex < 0) {
            break;
        }

        QString segment = plaintext.mid(segmentStart, closeIndex - segmentStart);
        int encryptedStart = output.size();
        QString encryptedSegment = encryptSegment(segment, parameters);
        output += encryptedSegment;
        int encryptedEnd = output.size();

        ranges.append({encryptedStart, encryptedEnd, algorithmTag, parameters});
        cursor = closeIndex + ClosingDelimiter.size();
    }

    output += plaintext.mid(cursor);

    for (const auto& range : ranges) {
        output += QStringLiteral("||x%1x%2#%3%4||")
            .arg(range.start, PositionHexWidth, 16, QLatin1Char('0'))
            .arg(range.end, PositionHexWidth, 16, QLatin1Char('0'))
            .arg(range.algorithmTag)
            .arg(serializeParameters(range.params));
    }

    return output;
}

QString DelimitedWeaverSimpleA::decrypt(const QString& ciphertext) const {
    QString body = ciphertext;
    struct RangeEntry {
        int start;
        int end;
        QStringList params;
    };
    QVector<RangeEntry> ranges;

    static const QRegularExpression trailerPattern(
        QStringLiteral("^(?:\\|\\|)?x([0-9A-Fa-f]{6})x([0-9A-Fa-f]{6})#([A-Za-z0-9]{2,8})(?::([^|]*))?(?:\\|\\|)?$")
    );

    while (!body.isEmpty()) {
        int trailerStart = body.lastIndexOf(QStringLiteral("||x"));
        if (trailerStart < 0) {
            trailerStart = body.lastIndexOf(QStringLiteral("|x"));
        }
        if (trailerStart < 0) {
            break;
        }

        QString trailingText = body.mid(trailerStart);
        auto match = trailerPattern.match(trailingText);
        if (!match.hasMatch()) {
            break;
        }

        bool okStart = false;
        bool okEnd = false;
        int start = match.captured(1).toInt(&okStart, 16);
        int end = match.captured(2).toInt(&okEnd, 16);
        QString algorithmTag = match.captured(3);
        QString parameterText = match.captured(4);
        QStringList params = parseParameters(parameterText);

        if (!okStart || !okEnd || algorithmTag != metadataTag()) {
            break;
        }

        ranges.prepend({start, end, params});
        body = body.left(trailerStart);
    }

    if (ranges.isEmpty()) {
        return ciphertext;
    }

    QString result = body;
    for (const RangeEntry& range : ranges) {
        if (range.start < 0 || range.end > result.size() || range.start > range.end) {
            continue;
        }

        QString segment = result.mid(range.start, range.end - range.start);
        QString decryptedSegment = decryptSegment(segment, range.params);
        result.replace(range.start, range.end - range.start, decryptedSegment);
    }

    return result;
}

QString DelimitedWeaverSimpleA::encryptSegment(
    const QString& segment,
    const QStringList& params) const
{
    Q_UNUSED(params);

    if (segment.size() <= 1)
        return segment;

    const int split = segment.size() / 2;

    QString firstHalf = segment.left(split);
    QString secondHalf = segment.mid(split);

    std::reverse(secondHalf.begin(), secondHalf.end());

    QString result;
    result.reserve(segment.size());

    const int maxLength = std::max(firstHalf.size(), secondHalf.size());

    for (int i = 0; i < maxLength; ++i)
    {
        if (i < firstHalf.size())
            result += firstHalf[i];

        if (i < secondHalf.size())
            result += secondHalf[i];
    }

    return result;
}

QString DelimitedWeaverSimpleA::decryptSegment(
    const QString& segment,
    const QStringList& params) const
{
    Q_UNUSED(params);

    if (segment.size() <= 1)
        return segment;

    const int firstSize  = segment.size() / 2;
    const int secondSize = segment.size() - firstSize;

    QString firstHalf;
    QString reversedSecondHalf;

    firstHalf.reserve(firstSize);
    reversedSecondHalf.reserve(secondSize);

    int index = 0;

    while (index < segment.size())
    {
        if (firstHalf.size() < firstSize)
            firstHalf += segment[index++];

        if (index < segment.size() &&
            reversedSecondHalf.size() < secondSize)
        {
            reversedSecondHalf += segment[index++];
        }
    }

    std::reverse(reversedSecondHalf.begin(), reversedSecondHalf.end());

    return firstHalf + reversedSecondHalf;
}
