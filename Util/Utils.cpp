// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Util/Utils.h"
#include <qregularexpression.h>

Utils* Utils::s_instance = nullptr;

void Utils::setcurrentNameSpaceMaps(const QMap<QString, QMap<int, QString>>& newCurrentNameSpaceMaps)
{
    if (m_currentNameSpaceMaps == newCurrentNameSpaceMaps)
        return;
    m_currentNameSpaceMaps = newCurrentNameSpaceMaps;
    emit currentNameSpaceMapsChanged();
}

void Utils::addNameSpaceMap(const QString& nameSpace, const QMap<int, QString>& map)
{
    m_currentNameSpaceMaps.insert(nameSpace, map);
    emit currentNameSpaceMapsChanged();
}

QMap<QString, QMap<int, QString>> Utils::currentNameSpaceMaps() const
{
    return m_currentNameSpaceMaps;
}

QString Utils::getNamespaceByIndex(const QString& namespaceOrigin, int index) const
{
    return m_currentNameSpaceMaps.value(namespaceOrigin).value(index);
}

Utils::Utils(QObject* parent)
    : QObject(parent)
    , m_mainWindowWidth(1280)
    , m_mainWindowHeight(720)
    , m_fontSize(20)
    , m_smallSpacing(5)
    , m_normalSpacing(15)
    , m_defaultSpacing(20)
    , m_preferredLabelWidth(150)
    , m_defaultIconSize(32)
    , m_smallIconSize(20)
    , m_smallFontSize(12)
    , m_defaultFontSize(20)
    , m_bigFontSize(30)
    , m_defaultIndent(20)
    , m_defaultPadding(10)
    , m_defaultRowHeight(24)
{}

Utils::~Utils()
{
    if (s_instance == this)
        s_instance = nullptr;
}

Utils* Utils::instance()
{
    if (!s_instance)
        s_instance = new Utils();

    return s_instance;
}

QObject* Utils::create(QQmlEngine*, QJSEngine*)
{
    return instance();
}

QString Utils::extractIdentifier(const QString& nodeId) const
{
    static const QRegularExpression regex(QStringLiteral("i=(\\d+)"));
    QRegularExpressionMatch match = regex.match(nodeId);

    if (match.hasMatch()) {
        return match.captured(1);
    } else {
        return QStringLiteral("");
    }
}

int Utils::extractNamespaceIndex(const QString& nodeId) const
{
    static const QRegularExpression regex(QStringLiteral("ns=(\\d+);"));
    QRegularExpressionMatch match = regex.match(nodeId);
    if (match.hasMatch()) {
        return match.captured(1).toInt();
    }
    // if we dont get a match, the nodeId has no ns=X part. In this case we assume the namespace is the UA Namespace (0)
    return 0;
}

QString Utils::extractNameFromNamespaceString(const QString& namespaceString) const
{
    static const QRegularExpression regex(QStringLiteral("UA/([^/]+)"));
    QRegularExpressionMatch match = regex.match(namespaceString);

    if (match.hasMatch()) {
        return match.captured(1).toLower();
    }

    return QStringLiteral("");
}

QVariantList Utils::convertQMapToVariantList(const QMap<QString, QString>& map) const
{
    QVariantList list;
    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        QVariantMap entry;
        entry[QStringLiteral("key")] = it.key();
        entry[QStringLiteral("value")] = it.value();
        list.append(entry);
    }
    return list;
}

QString Utils::removeNamespaceIndexFromName(const QString& nodeName) const
{
    // Check if the first character is numeric and there is a ':'
    if (nodeName.size() > 0 && nodeName[0].isDigit()) {
        int colonIndex = nodeName.indexOf(QChar::fromLatin1(':'));
        if (colonIndex != -1) {
            return nodeName.mid(colonIndex + 1);
        }
    }

    return nodeName;
}

QString Utils::lowerFirstChar(const QString& str) const
{
    QString returnStr;
    if (!str.isEmpty()) {
        // Get the first character and convert it to lowercase
        QChar firstChar = str[0].toLower();

        // Rebuild the string with the lowercase first character and the rest of the original string
        returnStr = firstChar + str.mid(1);
        return returnStr;
    }

    return str;
}

QString Utils::sanitizeName(const QString& name) const
{
    // We need to make sure that the variable names are valid identifiers for C
    // so we remove any character that is not a letter, a digit or an underscore
    // and we make sure that the first character is a letter or an underscore
    QString sanitized = name;
    static const QRegularExpression regex(QStringLiteral("[^a-zA-Z0-9_]"));
    sanitized.remove(regex);

    if (!sanitized.isEmpty() && !sanitized[0].isLetter() && sanitized[0] != QStringLiteral("_")) {
        sanitized.prepend(
            QStringLiteral("_")); // Prefix an underscore if the first character is invalid
    }

    return sanitized;
}

int Utils::mainWindowHeight() const
{
    return m_mainWindowHeight;
}

int Utils::mainWindowWidth() const
{
    return m_mainWindowWidth;
}

int Utils::defaultRowHeight() const
{
    return m_defaultRowHeight;
}
