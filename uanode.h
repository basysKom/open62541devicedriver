// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef UANODE_H
#define UANODE_H

#include "Util/Utils.h"
#include <QMap>
#include <QString>
#include <qobject.h>
#include <qvariant.h>

struct Argument
{
    QString name;
    QString dataTypeIdentifier;
    int valueRank = -1;
};

class Reference;

class UANode
{
public:
    UANode(){};
    UANode(
        const QString& nodeId,
        const QString& browseName,
        QList<std::shared_ptr<Reference>> references = {},
        const QString& parentNodeId = QStringLiteral(""),
        const QString& displayName = QStringLiteral(""),
        const QString& description = QStringLiteral(""))
        : m_nodeId(nodeId)
        , m_browseName(browseName)
        , m_references(references)
        , m_parentNodeId(parentNodeId)
        , m_displayName(displayName)
        , m_description(description)
        , m_baseBrowseName(browseName)
    {
        qDebug() << "we are in the node constructor" << "Nodeid" << nodeId << " browseName "
                 << browseName << "base" << m_baseBrowseName;
    }

    virtual ~UANode();

    virtual std::shared_ptr<UANode> clone() const { return std::make_shared<UANode>(*this); };
    virtual QString typeName() const { return XmlTags::UANode; }

    UANode(const UANode& other);
    UANode& operator=(const UANode& other);

    QString nodeId() const;
    void setNodeId(const QString& nodeId);

    QString browseName() const;
    void setBrowseName(const QString& browseName);

    QString displayName() const;
    void setDisplayName(const QString& displayName);

    QList<std::shared_ptr<Reference>> references() const;
    void addReference(std::shared_ptr<Reference> reference);

    QString parentNodeId() const;
    void setParentNodeId(const QString& parentNodeId);

    QString description() const;
    void setDescription(const QString& description);

    std::weak_ptr<UANode> parentNode() const;
    void setParentNode(std::weak_ptr<UANode> parentNode);

    QString namespaceString() const;
    void setNamespaceString(const QString& newNamespaceString);

    bool isOptional() const;
    void setIsOptional(bool newIsOptional);

    void changeNamespaceId(const int newNamespaceId);

    bool isRootNode() const;
    void setIsRootNode(bool newIsRootNode);

    QString nodeVariableName() const;
    void setNodeVariableName();

    QString baseBrowseName() const;

    void setBaseBrowseName(const QString& newBaseBrowseName);

    QString uniqueBaseBrowseName() const;
    void setUniqueBaseBrowseName(const QString& newUniqueBaseBrowseName);

signals:
    void uniqueBaseBrowseNameChanged();

private:
    QString m_nodeId;
    QString m_browseName;
    QString m_baseBrowseName;
    QString m_uniqueBaseBrowseName;
    QString m_displayName;
    QString m_nodeVariableName;
    QString m_description;
    QList<std::shared_ptr<Reference>> m_references;
    QString m_parentNodeId;
    QString m_namespaceString;
    std::weak_ptr<UANode> m_parentNode;
    bool m_isOptional = false;
    bool m_isRootNode = false;
    Q_PROPERTY(QString uniqueBaseBrowseName READ uniqueBaseBrowseName WRITE setUniqueBaseBrowseName
                   NOTIFY uniqueBaseBrowseNameChanged FINAL)
};

class UADataType : public UANode
{
public:
    UADataType(){};
    UADataType(const QString& nodeId, const QString& browseName)
        : UANode(nodeId, browseName)
    {}

    virtual ~UADataType() {}

    QString typeName() const override { return XmlTags::UADataType; }

    std::shared_ptr<UANode> clone() const override { return std::make_shared<UADataType>(*this); }

    UADataType(const UADataType& other);
    UADataType& operator=(const UADataType& other);
    bool operator==(const UADataType& other) const;

    QString definitionName() const;
    void setDefinitionName(const QString& definitionName);

    QMap<QString, QString> definitionFields() const;
    void addDefinitionField(const QString& fieldName, const QString& fieldType);

    bool isEnum() const;
    void setIsEnum(bool newIsEnum);

private:
    QString m_definitionName;
    // DefinitionName -> DataType -> Value. If there is no DataType, we have an enum.
    QMap<QString, QString> m_definitionFields;
    bool m_isEnum = false;
};

class UAObject : public UANode
{
public:
    UAObject(){};
    UAObject(const QString& nodeId, const QString& browseName)
        : UANode(nodeId, browseName)
    {}

    virtual ~UAObject() {}

    QString typeName() const override { return XmlTags::UAObject; }

    std::shared_ptr<UANode> clone() const override { return std::make_shared<UAObject>(*this); }
    UAObject(const UAObject& other);
    UAObject& operator=(const UAObject& other);
};

class UAVariable : public UANode
{
public:
    UAVariable(){};
    UAVariable(const QString& nodeId, const QString& browseName)
        : UANode(nodeId, browseName)
    {}

    virtual ~UAVariable() {}

    QString typeName() const override { return XmlTags::UAVariable; }

    std::shared_ptr<UANode> clone() const override { return std::make_shared<UAVariable>(*this); }

    UAVariable(const UAVariable& other);
    UAVariable& operator=(const UAVariable& other);

    UADataType dataType() const;
    void setDataType(const UADataType& dataType);

    QVariant value() const;
    void setValue(const QVariant& newValue);

    QStringList valueModel() const;

    int valueModelIndex() const;
    void setValueModelIndex(int newValueModelIndex);

    QList<Argument> arguments() const;
    void setArguments(const QList<Argument>& newArguments);

    int arrayDimensions() const;
    void setArrayDimensions(int newArrayDimensions);

    int valueRank() const;
    void setValueRank(int newValueRank);

private:
    UADataType m_dataType;
    QList<Argument> m_arguments;
    int m_arrayDimensions = 1;
    int m_valueRank = -1;
};

class UAMethod : public UANode
{
public:
    UAMethod(){};
    UAMethod(const QString& nodeId, const QString& browseName)
        : UANode(nodeId, browseName)
    {}

    virtual ~UAMethod() {}

    QString typeName() const override { return XmlTags::UAMethod; }

    std::shared_ptr<UANode> clone() const override { return std::make_shared<UAMethod>(*this); }

    UAMethod(const UAMethod& other);
    UAMethod& operator=(const UAMethod& other);

    void setInputArgument(std::shared_ptr<UAVariable> var);
    void setOutputArgument(std::shared_ptr<UAVariable> var);

    std::shared_ptr<UAVariable> inputArgument() const;
    std::shared_ptr<UAVariable> outputArgument() const;

private:
    std::shared_ptr<UAVariable> m_inputArgument = nullptr;
    std::shared_ptr<UAVariable> m_outputArgument = nullptr;
};

class UAVariableType : public UAVariable
{
public:
    UAVariableType(){};
    UAVariableType(const QString& nodeId, const QString& browseName, bool isAbstract = false)
        : UAVariable(nodeId, browseName)
        , m_isAbstract(isAbstract)
    {}

    virtual ~UAVariableType() {}

    QString typeName() const override { return XmlTags::UAVariableType; }

    std::shared_ptr<UANode> clone() const override
    {
        return std::make_shared<UAVariableType>(*this);
    }

    UAVariableType(const UAVariableType& other);
    UAVariableType& operator=(const UAVariableType& other);

    bool isAbstract() const;
    void setIsAbstract(bool isAbstract);

private:
    bool m_isAbstract;
};

class UAObjectType : public UANode
{
public:
    UAObjectType(){};
    UAObjectType(const QString& nodeId, const QString& browseName, bool isAbstract = false)
        : UANode(nodeId, browseName)
        , m_isAbstract(isAbstract)
    {}

    virtual ~UAObjectType() {}

    QString typeName() const override { return XmlTags::UAObjectType; }

    std::shared_ptr<UANode> clone() const override { return std::make_shared<UAObjectType>(*this); }

    UAObjectType(const UAObjectType& other);
    UAObjectType& operator=(const UAObjectType& other);

    bool isAbstract() const;
    void setIsAbstract(bool isAbstract);

private:
    bool m_isAbstract;
};

class Reference
{
public:
    virtual ~Reference() = default;
    Reference(){};
    Reference(
        const QString& referenceType,
        const QString& targetNodeId,
        bool isForward = true,
        const QString& namespaceString = QStringLiteral(""),
        std::weak_ptr<UANode> node = std::weak_ptr<UANode>{})
        : m_referenceType(referenceType)
        , m_targetNodeId(targetNodeId)
        , m_isForward(isForward)
        , m_namespaceString(namespaceString)
        , m_node(node)
    {}

    QString typeName() const { return XmlTags::Reference; }

    Reference(const Reference& other);
    Reference& operator=(const Reference& other);

    QString referenceType() const;
    void setReferenceType(const QString& referenceType);

    QString targetNodeId() const;
    void setTargetNodeId(const QString& targetId);

    bool isForward() const;
    void setIsForward(bool isForward);

    QString namespaceString() const;
    void setNamespaceString(const QString& newNamespaceString);

    std::shared_ptr<UANode> node() const;
    void setNode(std::shared_ptr<UANode> node);

private:
    QString m_referenceType;
    QString m_targetNodeId;
    bool m_isForward;
    QString m_namespaceString;
    std::weak_ptr<UANode> m_node;
};

#endif // UANODE_H
