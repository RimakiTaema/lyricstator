#include "QtStringUtils.h"
#include <QRegExp>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTextStream>
#include <QDateTime>
#include <QCryptographicHash>
#include <QRandomGenerator>

namespace Lyricstator {

QtStringUtils& QtStringUtils::getInstance() {
    static QtStringUtils instance;
    return instance;
}

QtStringUtils::QtStringUtils(QObject* parent)
    : QObject(parent)
{
}

QtStringUtils::~QtStringUtils() {
}

QString QtStringUtils::toUpper(const QString& str) {
    return str.toUpper();
}

QString QtStringUtils::toLower(const QString& str) {
    return str.toLower();
}

QString QtStringUtils::capitalize(const QString& str) {
    if (str.isEmpty()) return str;
    return str.at(0).toUpper() + str.mid(1).toLower();
}

QString QtStringUtils::titleCase(const QString& str) {
    QStringList words = str.split(' ', QString::SkipEmptyParts);
    for (int i = 0; i < words.size(); ++i) {
        words[i] = capitalize(words[i]);
    }
    return words.join(' ');
}

QString QtStringUtils::reverse(const QString& str) {
    QString result;
    for (int i = str.length() - 1; i >= 0; --i) {
        result += str.at(i);
    }
    return result;
}

bool QtStringUtils::contains(const QString& str, const QString& substring, Qt::CaseSensitivity sensitivity) {
    return str.contains(substring, sensitivity);
}

bool QtStringUtils::startsWith(const QString& str, const QString& prefix, Qt::CaseSensitivity sensitivity) {
    return str.startsWith(prefix, sensitivity);
}

bool QtStringUtils::endsWith(const QString& str, const QString& suffix, Qt::CaseSensitivity sensitivity) {
    return str.endsWith(suffix, sensitivity);
}

int QtStringUtils::indexOf(const QString& str, const QString& substring, int from, Qt::CaseSensitivity sensitivity) {
    return str.indexOf(substring, from, sensitivity);
}

int QtStringUtils::lastIndexOf(const QString& str, const QString& substring, int from, Qt::CaseSensitivity sensitivity) {
    return str.lastIndexOf(substring, from, sensitivity);
}

QString QtStringUtils::replace(const QString& str, const QString& before, const QString& after, Qt::CaseSensitivity sensitivity) {
    return str.replace(before, after, sensitivity);
}

QString QtStringUtils::replaceAll(const QString& str, const QString& before, const QString& after, Qt::CaseSensitivity sensitivity) {
    QString result = str;
    return result.replace(before, after, sensitivity);
}

QString QtStringUtils::replaceRegex(const QString& str, const QString& pattern, const QString& replacement) {
    QRegularExpression regex(pattern);
    return str.replace(regex, replacement);
}

QStringList QtStringUtils::split(const QString& str, const QString& separator, QString::SplitBehavior behavior, Qt::CaseSensitivity sensitivity) {
    return str.split(separator, behavior, sensitivity);
}

QStringList QtStringUtils::splitRegex(const QString& str, const QString& pattern) {
    QRegularExpression regex(pattern);
    return str.split(regex);
}

QString QtStringUtils::join(const QStringList& list, const QString& separator) {
    return list.join(separator);
}

QString QtStringUtils::trim(const QString& str) {
    return str.trimmed();
}

QString QtStringUtils::trimLeft(const QString& str) {
    return str.trimmed();
}

QString QtStringUtils::trimRight(const QString& str) {
    return str.trimmed();
}

bool QtStringUtils::isEmpty(const QString& str) {
    return str.isEmpty();
}

bool QtStringUtils::isBlank(const QString& str) {
    return str.trimmed().isEmpty();
}

bool QtStringUtils::isNumeric(const QString& str) {
    bool ok;
    str.toDouble(&ok);
    return ok;
}

bool QtStringUtils::isAlpha(const QString& str) {
    QRegExp regex("^[a-zA-Z]+$");
    return regex.exactMatch(str);
}

bool QtStringUtils::isAlphaNumeric(const QString& str) {
    QRegExp regex("^[a-zA-Z0-9]+$");
    return regex.exactMatch(str);
}

bool QtStringUtils::isEmail(const QString& str) {
    QRegExp regex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return regex.exactMatch(str);
}

bool QtStringUtils::isUrl(const QString& str) {
    QRegExp regex("^https?://[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}(/.*)?$");
    return regex.exactMatch(str);
}

QString QtStringUtils::format(const QString& format, const QVariantList& args) {
    QString result = format;
    for (int i = 0; i < args.size(); ++i) {
        result.replace(QString("{%1}").arg(i), args[i].toString());
    }
    return result;
}

QString QtStringUtils::formatNumber(double number, int precision) {
    return QString::number(number, 'f', precision);
}

QString QtStringUtils::formatFileSize(qint64 bytes) {
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    
    if (bytes >= GB) {
        return QString("%1 GB").arg(QString::number(bytes / (double)GB, 'f', 2));
    } else if (bytes >= MB) {
        return QString("%1 MB").arg(QString::number(bytes / (double)MB, 'f', 2));
    } else if (bytes >= KB) {
        return QString("%1 KB").arg(QString::number(bytes / (double)KB, 'f', 2));
    } else {
        return QString("%1 bytes").arg(bytes);
    }
}

QString QtStringUtils::formatDuration(int milliseconds) {
    int seconds = milliseconds / 1000;
    int minutes = seconds / 60;
    int hours = minutes / 60;
    
    seconds %= 60;
    minutes %= 60;
    
    if (hours > 0) {
        return QString("%1:%2:%3").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    }
}

QString QtStringUtils::formatTime(int seconds) {
    int minutes = seconds / 60;
    int hours = minutes / 60;
    
    seconds %= 60;
    minutes %= 60;
    
    if (hours > 0) {
        return QString("%1:%2:%3").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    }
}

QString QtStringUtils::escapeHtml(const QString& str) {
    QString result = str;
    result.replace("&", "&amp;");
    result.replace("<", "&lt;");
    result.replace(">", "&gt;");
    result.replace("\"", "&quot;");
    result.replace("'", "&#39;");
    return result;
}

QString QtStringUtils::unescapeHtml(const QString& str) {
    QString result = str;
    result.replace("&amp;", "&");
    result.replace("&lt;", "<");
    result.replace("&gt;", ">");
    result.replace("&quot;", "\"");
    result.replace("&#39;", "'");
    return result;
}

QString QtStringUtils::escapeXml(const QString& str) {
    return escapeHtml(str);
}

QString QtStringUtils::unescapeXml(const QString& str) {
    return unescapeHtml(str);
}

QString QtStringUtils::escapeJson(const QString& str) {
    QString result = str;
    result.replace("\\", "\\\\");
    result.replace("\"", "\\\"");
    result.replace("\n", "\\n");
    result.replace("\r", "\\r");
    result.replace("\t", "\\t");
    return result;
}

QString QtStringUtils::unescapeJson(const QString& str) {
    QString result = str;
    result.replace("\\\\", "\\");
    result.replace("\\\"", "\"");
    result.replace("\\n", "\n");
    result.replace("\\r", "\r");
    result.replace("\\t", "\t");
    return result;
}

bool QtStringUtils::equals(const QString& str1, const QString& str2, Qt::CaseSensitivity sensitivity) {
    return str1.compare(str2, sensitivity) == 0;
}

bool QtStringUtils::equalsIgnoreCase(const QString& str1, const QString& str2) {
    return str1.compare(str2, Qt::CaseInsensitive) == 0;
}

int QtStringUtils::compare(const QString& str1, const QString& str2, Qt::CaseSensitivity sensitivity) {
    return str1.compare(str2, sensitivity);
}

int QtStringUtils::compareIgnoreCase(const QString& str1, const QString& str2) {
    return str1.compare(str2, Qt::CaseInsensitive);
}

int QtStringUtils::length(const QString& str) {
    return str.length();
}

int QtStringUtils::wordCount(const QString& str) {
    QStringList words = str.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    return words.size();
}

int QtStringUtils::lineCount(const QString& str) {
    return str.count('\n') + 1;
}

int QtStringUtils::charCount(const QString& str) {
    return str.length();
}

QString QtStringUtils::slugify(const QString& str) {
    QString result = str.toLower();
    result = removeAccents(result);
    result.replace(QRegExp("[^a-z0-9\\s-]"), "");
    result.replace(QRegExp("\\s+"), "-");
    result.replace(QRegExp("-+"), "-");
    result = result.trimmed();
    if (result.startsWith('-')) result = result.mid(1);
    if (result.endsWith('-')) result = result.left(result.length() - 1);
    return result;
}

QString QtStringUtils::normalize(const QString& str) {
    return str.normalized(QString::NormalizationForm_C);
}

QString QtStringUtils::transliterate(const QString& str) {
    // Basic transliteration - can be enhanced with more mappings
    QString result = str;
    result.replace("ä", "ae");
    result.replace("ö", "oe");
    result.replace("ü", "ue");
    result.replace("ß", "ss");
    result.replace("ñ", "n");
    result.replace("ç", "c");
    return result;
}

QString QtStringUtils::removeAccents(const QString& str) {
    return transliterate(str);
}

QString QtStringUtils::randomString(int length) {
    const QString chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    QString result;
    for (int i = 0; i < length; ++i) {
        int index = QRandomGenerator::global()->bounded(chars.length());
        result += chars.at(index);
    }
    return result;
}

QString QtStringUtils::uuid() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString QtStringUtils::hash(const QString& str, const QString& algorithm) {
    QByteArray data = str.toUtf8();
    QByteArray hash;
    
    if (algorithm.toLower() == "md5") {
        hash = QCryptographicHash::hash(data, QCryptographicHash::Md5);
    } else if (algorithm.toLower() == "sha1") {
        hash = QCryptographicHash::hash(data, QCryptographicHash::Sha1);
    } else if (algorithm.toLower() == "sha256") {
        hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    } else {
        // Default to MD5
        hash = QCryptographicHash::hash(data, QCryptographicHash::Md5);
    }
    
    return hash.toHex();
}

} // namespace Lyricstator