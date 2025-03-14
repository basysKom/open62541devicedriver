// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QChar>
#include <QDebug>
#include <QMap>
#include <QObject>

namespace XmlTags {
inline const QString Models = QStringLiteral("Models");
inline const QString RequiredModel = QStringLiteral("RequiredModel");
inline const QString Model = QStringLiteral("Model");
inline const QString ModelUri = QStringLiteral("ModelUri");
inline const QString Namespace = QStringLiteral("Namespace");
inline const QString Uri = QStringLiteral("Uri");
inline const QString UANode = QStringLiteral("UANode");
inline const QString UAObject = QStringLiteral("UAObject");
inline const QString UADataType = QStringLiteral("UADataType");
inline const QString UAVariable = QStringLiteral("UAVariable");
inline const QString UAMethod = QStringLiteral("UAMethod");
inline const QString UAVariableType = QStringLiteral("UAVariableType");
inline const QString UAObjectType = QStringLiteral("UAObjectType");
inline const QString References = QStringLiteral("References");
inline const QString Reference = QStringLiteral("Reference");
inline const QString ReferenceType = QStringLiteral("ReferenceType");
inline const QString TargetId = QStringLiteral("TargetId");
inline const QString BrowseName = QStringLiteral("BrowseName");
inline const QString DisplayName = QStringLiteral("DisplayName");
inline const QString Description = QStringLiteral("Description");
inline const QString ParentNodeId = QStringLiteral("ParentNodeId");
inline const QString NodeId = QStringLiteral("NodeId");
inline const QString IsAbstract = QStringLiteral("IsAbstract");
inline const QString IsOptional = QStringLiteral("IsOptional");
inline const QString DataType = QStringLiteral("DataType");
inline const QString UANodeSet = QStringLiteral("UANodeSet");
inline const QString IsForward = QStringLiteral("IsForward");
inline const QString Definition = QStringLiteral("Definition");
inline const QString Name = QStringLiteral("Name");
inline const QString Field = QStringLiteral("Field");
inline const QString Value = QStringLiteral("Value");
inline const QString ValueRank = QStringLiteral("ValueRank");
inline const QString ArrayDimensions = QStringLiteral("ArrayDimensions");
inline const QString Aliases = QStringLiteral("Aliases");
inline const QString Alias = QStringLiteral("Alias");
inline const QString Mandatory = QStringLiteral("Mandatory");
inline const QString Optional = QStringLiteral("Optional");
inline const QString Arguments = QStringLiteral("Arguments");
inline const QString HasSubtype = QStringLiteral("HasSubtype");
inline const QString HasProperty = QStringLiteral("HasProperty");
inline const QString HasComponent = QStringLiteral("HasComponent");
inline const QString HasTypeDefinition = QStringLiteral("HasTypeDefinition");
inline const QString HasModellingRule = QStringLiteral("HasModellingRule");
inline const QString HasEncoding = QStringLiteral("HasEncoding");

} // namespace XmlTags

class QQmlEngine;
class QJSEngine;

class Utils : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int fontSize READ fontSize CONSTANT)
    Q_PROPERTY(int defaultFontSize READ defaultFontSize CONSTANT)
    Q_PROPERTY(int smallSpacing READ smallSpacing CONSTANT)
    Q_PROPERTY(int normalSpacing READ normalSpacing CONSTANT)
    Q_PROPERTY(int defaultSpacing READ defaultSpacing CONSTANT)
    Q_PROPERTY(int preferredLabelWidth READ preferredLabelWidth CONSTANT)
    Q_PROPERTY(int defaultIconSize READ defaultIconSize CONSTANT)
    Q_PROPERTY(int smallIconSize READ smallIconSize CONSTANT)
    Q_PROPERTY(int smallFontSize READ smallFontSize CONSTANT)
    Q_PROPERTY(int bigFontSize READ bigFontSize CONSTANT)
    Q_PROPERTY(int defaultIndent READ defaultIndent CONSTANT)
    Q_PROPERTY(int defaultPadding READ defaultPadding CONSTANT)
    Q_PROPERTY(int mainWindowWidth READ mainWindowWidth CONSTANT)
    Q_PROPERTY(int mainWindowHeight READ mainWindowHeight CONSTANT)
    Q_PROPERTY(int defaultRowHeight READ defaultRowHeight CONSTANT)
    Q_PROPERTY(QMap<QString, QMap<int, QString>> currentNameSpaceMaps READ currentNameSpaceMaps
                   WRITE setcurrentNameSpaceMaps NOTIFY currentNameSpaceMapsChanged)

public:
    static Utils* instance();
    static QObject* create(QQmlEngine* engine, QJSEngine* jsEngine);
    ~Utils();

    explicit Utils(QObject* parent = nullptr);

    int fontSize() const { return m_fontSize; }
    int defaultFontSize() const { return m_defaultFontSize; }
    int smallSpacing() const { return m_smallSpacing; }
    int normalSpacing() const { return m_normalSpacing; }
    int defaultSpacing() const { return m_defaultSpacing; }
    int preferredLabelWidth() const { return m_preferredLabelWidth; }
    int defaultIconSize() const { return m_defaultIconSize; }
    int smallIconSize() const { return m_smallIconSize; }
    int smallFontSize() const { return m_smallFontSize; }
    int bigFontSize() const { return m_bigFontSize; }
    int defaultIndent() const { return m_defaultIndent; }
    int defaultPadding() const { return m_defaultPadding; }

    QMap<QString, QMap<int, QString>> currentNameSpaceMaps() const;
    void setcurrentNameSpaceMaps(const QMap<QString, QMap<int, QString>>& newCurrentNameSpaceMaps);
    void addNameSpaceMap(const QString& nameSpace, const QMap<int, QString>& map);
    Q_INVOKABLE QString extractIdentifier(const QString& nodeId) const;
    Q_INVOKABLE int extractNamespaceIndex(const QString& nodeId) const;
    Q_INVOKABLE QString extractNameFromNamespaceString(const QString& namespaceString) const;
    QString getNamespaceByIndex(const QString& namespaceOrigin, int index) const;
    QVariantList convertQMapToVariantList(const QMap<QString, QString>& map) const;
    QString removeNamespaceIndexFromName(const QString& nodeName) const;
    QString lowerFirstChar(const QString& str) const;
    QString sanitizeName(const QString& name) const;

    int mainWindowHeight() const;

    int mainWindowWidth() const;

    int defaultRowHeight() const;

signals:
    void currentNameSpaceMapsChanged();

private:
    const int m_fontSize;
    const int m_smallSpacing;
    const int m_normalSpacing;
    const int m_defaultSpacing;
    const int m_preferredLabelWidth;
    const int m_defaultIconSize;
    const int m_smallIconSize;
    const int m_smallFontSize;
    const int m_defaultFontSize;
    const int m_bigFontSize;
    const int m_defaultIndent;
    const int m_defaultPadding;
    const int m_mainWindowWidth;
    const int m_mainWindowHeight;
    const int m_defaultRowHeight;

    QMap<QString, QMap<int, QString>> m_currentNameSpaceMaps;

    static Utils* s_instance;
};

Q_DECLARE_METATYPE(Utils*);
