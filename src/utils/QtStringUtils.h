#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QRegularExpression>

namespace Lyricstator {

class QtStringUtils : public QObject {
    Q_OBJECT

public:
    static QtStringUtils& getInstance();
    
    // String manipulation
    static QString toUpper(const QString& str);
    static QString toLower(const QString& str);
    static QString capitalize(const QString& str);
    static QString titleCase(const QString& str);
    static QString reverse(const QString& str);
    
    // String searching and replacement
    static bool contains(const QString& str, const QString& substring, Qt::CaseSensitivity sensitivity = Qt::CaseSensitive);
    static bool startsWith(const QString& str, const QString& prefix, Qt::CaseSensitivity sensitivity = Qt::CaseSensitive);
    static bool endsWith(const QString& str, const QString& suffix, Qt::CaseSensitivity sensitivity = Qt::CaseSensitive);
    static int indexOf(const QString& str, const QString& substring, int from = 0, Qt::CaseSensitivity sensitivity = Qt::CaseSensitive);
    static int lastIndexOf(const QString& str, const QString& substring, int from = -1, Qt::CaseSensitivity sensitivity = Qt::CaseSensitive);
    
    // String replacement
    static QString replace(const QString& str, const QString& before, const QString& after, Qt::CaseSensitivity sensitivity = Qt::CaseSensitive);
    static QString replaceAll(const QString& str, const QString& before, const QString& after, Qt::CaseSensitivity sensitivity = Qt::CaseSensitive);
    static QString replaceRegex(const QString& str, const QString& pattern, const QString& replacement);
    
    // String splitting and joining
    static QStringList split(const QString& str, const QString& separator, QString::SplitBehavior behavior = QString::KeepEmptyParts, Qt::CaseSensitivity sensitivity = Qt::CaseSensitive);
    static QStringList splitRegex(const QString& str, const QString& pattern);
    static QString join(const QStringList& list, const QString& separator);
    
    // String trimming
    static QString trim(const QString& str);
    static QString trimLeft(const QString& str);
    static QString trimRight(const QString& str);
    
    // String validation
    static bool isEmpty(const QString& str);
    static bool isBlank(const QString& str);
    static bool isNumeric(const QString& str);
    static bool isAlpha(const QString& str);
    static bool isAlphaNumeric(const QString& str);
    static bool isEmail(const QString& str);
    static bool isUrl(const QString& str);
    
    // String formatting
    static QString format(const QString& format, const QVariantList& args);
    static QString formatNumber(double number, int precision = 2);
    static QString formatFileSize(qint64 bytes);
    static QString formatDuration(int milliseconds);
    static QString formatTime(int seconds);
    
    // String encoding and escaping
    static QString escapeHtml(const QString& str);
    static QString unescapeHtml(const QString& str);
    static QString escapeXml(const QString& str);
    static QString unescapeXml(const QString& str);
    static QString escapeJson(const QString& str);
    static QString unescapeJson(const QString& str);
    
    // String comparison
    static bool equals(const QString& str1, const QString& str2, Qt::CaseSensitivity sensitivity = Qt::CaseSensitive);
    static bool equalsIgnoreCase(const QString& str1, const QString& str2);
    static int compare(const QString& str1, const QString& str2, Qt::CaseSensitivity sensitivity = Qt::CaseSensitive);
    static int compareIgnoreCase(const QString& str1, const QString& str2);
    
    // String metrics
    static int length(const QString& str);
    static int wordCount(const QString& str);
    static int lineCount(const QString& str);
    static int charCount(const QString& str);
    
    // String transformation
    static QString slugify(const QString& str);
    static QString normalize(const QString& str);
    static QString transliterate(const QString& str);
    static QString removeAccents(const QString& str);
    
    // Utility methods
    static QString randomString(int length = 8);
    static QString uuid();
    static QString hash(const QString& str, const QString& algorithm = "md5");

private:
    QtStringUtils(QObject* parent = nullptr);
    ~QtStringUtils();
};

// Global instance
#define StringUtils QtStringUtils::getInstance()

} // namespace Lyricstator