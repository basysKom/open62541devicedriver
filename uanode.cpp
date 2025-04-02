// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

#include "uanode.h"
#include <qdebug.h>

UANode::~UANode()
{
    m_references.clear();
}

UANode::UANode(const UANode& other)
    : m_nodeId(other.m_nodeId)
    , m_browseName(other.m_browseName)
    , m_displayName(other.m_displayName)
    , m_description(other.m_description)
    , m_parentNodeId(other.m_parentNodeId)
    , m_namespaceString(other.m_namespaceString)
    , m_isOptional(other.m_isOptional)
    , m_nodeVariableName(other.m_nodeVariableName)
    , m_parentNode(other.m_parentNode)
    , m_isRootNode(other.m_isRootNode)
    , m_baseBrowseName(other.m_baseBrowseName)
{
    // Deep copy of references
    for (const std::shared_ptr<Reference>& ref : other.m_references) {
        m_references.append(std::make_shared<Reference>(*ref));
    }
}

UANode& UANode::operator=(const UANode& other)
{
    if (this != &other) {
        // clear existing references to prevent memory leaks
        m_references.clear();

        m_nodeId = other.m_nodeId;
        m_browseName = other.m_browseName;
        m_displayName = other.m_displayName;
        m_description = other.m_description;
        m_parentNodeId = other.m_parentNodeId;
        m_namespaceString = other.m_namespaceString;
        m_isOptional = other.m_isOptional;
        m_nodeVariableName = other.m_nodeVariableName;
        m_isRootNode = other.m_isRootNode;
        m_baseBrowseName = other.m_baseBrowseName;

        // Deep copy of references
        m_references.reserve(other.m_references.size());
        for (const auto& ref : other.m_references) {
            m_references.append(std::make_shared<Reference>(*ref));
        }

        m_parentNode = std::weak_ptr<UANode>(other.m_parentNode);
    }
    return *this;
}

QString UANode::nodeId() const
{
    return m_nodeId;
}

void UANode::setNodeId(const QString& nodeId)
{
    m_nodeId = nodeId;
}

QString UANode::browseName() const
{
    return m_browseName;
}

void UANode::setBrowseName(const QString& browseName)
{
    m_browseName = browseName;
    setNodeVariableName();
}

QString UANode::displayName() const
{
    return m_displayName;
}

void UANode::setDisplayName(const QString& displayName)
{
    m_displayName = displayName;
}

QList<std::shared_ptr<Reference>> UANode::references() const
{
    return m_references;
}

void UANode::addReference(std::shared_ptr<Reference> reference)
{
    m_references.append(std::move(reference));
}

QString UANode::parentNodeId() const
{
    return m_parentNodeId;
}

void UANode::setParentNodeId(const QString& parentNodeId)
{
    m_parentNodeId = parentNodeId;
}

QString UANode::description() const
{
    return m_description;
}

void UANode::setDescription(const QString& description)
{
    m_description = description;
}

std::weak_ptr<UANode> UANode::parentNode() const
{
    return m_parentNode;
}

void UANode::setParentNode(std::weak_ptr<UANode> parentNode)
{
    m_parentNode = parentNode;
}

QString UANode::namespaceString() const
{
    return m_namespaceString;
}

void UANode::setNamespaceString(const QString& newNamespaceString)
{
    m_namespaceString = newNamespaceString;
}

bool UANode::isOptional() const
{
    return m_isOptional;
}

void UANode::setIsOptional(bool newIsOptional)
{
    m_isOptional = newIsOptional;
}

void UANode::changeNamespaceId(const int newNamespaceId)
{
    int nsPosition = m_nodeId.indexOf(QStringLiteral("ns="));
    if (nsPosition != -1) {
        int semicolonPosition = m_nodeId.indexOf(QStringLiteral(";"), nsPosition);
        if (semicolonPosition != -1) {
            // Replace the namespace number with the new one
            QString newNodeId = m_nodeId.replace(
                nsPosition + 3,
                semicolonPosition - (nsPosition + 3),
                QString::number(newNamespaceId));
            // Set the new NodeId
            setNodeId(newNodeId);
        }
    }
}

bool UANode::isRootNode() const
{
    return m_isRootNode;
}

void UANode::setIsRootNode(bool newIsRootNode)
{
    m_isRootNode = newIsRootNode;
}

QString UANode::nodeVariableName() const
{
    return m_nodeVariableName;
}

void UANode::setNodeVariableName()
{
    QString variableNameStr = Utils::instance()->removeNamespaceIndexFromName(m_browseName)
                              + QStringLiteral("_")
                              + Utils::instance()->extractIdentifier(m_nodeId);

    m_nodeVariableName = variableNameStr;
}

QString UANode::baseBrowseName() const
{
    return m_baseBrowseName;
}

void UANode::setBaseBrowseName(const QString& newBaseBrowseName)
{
    m_baseBrowseName = newBaseBrowseName;
}

QString UANode::uniqueBaseBrowseName() const
{
    return m_uniqueBaseBrowseName;
}

void UANode::setUniqueBaseBrowseName(const QString& newUniqueBaseBrowseName)
{
    m_uniqueBaseBrowseName = newUniqueBaseBrowseName;
}

UADataType::UADataType(const UADataType& other)
    : UANode(other)
    , m_definitionName(other.m_definitionName)
    , m_definitionFields(other.m_definitionFields)
    , m_isEnum(other.m_isEnum)
{}
UADataType& UADataType::operator=(const UADataType& other)
{
    if (this != &other) {
        UANode::operator=(other);
        m_definitionName = other.m_definitionName;
        m_definitionFields = other.m_definitionFields;
        m_isEnum = other.m_isEnum;
    }
    return *this;
}

bool UADataType::operator==(const UADataType& other) const
{
    return (m_definitionName == other.m_definitionName)
           && (m_definitionFields == other.m_definitionFields) && (m_isEnum == other.m_isEnum);
}

QString UADataType::definitionName() const
{
    return m_definitionName;
}

void UADataType::setDefinitionName(const QString& definitionName)
{
    m_definitionName = definitionName;
}

QMap<QString, QString> UADataType::definitionFields() const
{
    return m_definitionFields;
}

void UADataType::addDefinitionField(const QString& fieldName, const QString& fieldType)
{
    // TODO currently, the definition fields handle ENUM members and datatypes with their name.
    // We should store the enum definitions in a separate structure.
    m_definitionFields[fieldName] = fieldType;
}

bool UADataType::isEnum() const
{
    return m_isEnum;
}

void UADataType::setIsEnum(bool newIsEnum)
{
    m_isEnum = newIsEnum;
}

UAObject::UAObject(const UAObject& other)
    : UANode(other)
{}

UAObject& UAObject::operator=(const UAObject& other)
{
    if (this != &other) {
        UANode::operator=(other);
    }
    return *this;
}

UAVariable::UAVariable(const UAVariable& other)
    : UANode(other)
    , m_dataType(other.m_dataType)
    , m_arguments(other.m_arguments)
    , m_arrayDimensions(other.m_arrayDimensions)
    , m_valueRank(other.m_valueRank)
{}

UAVariable& UAVariable::operator=(const UAVariable& other)
{
    if (this != &other) {
        UANode::operator=(other);
        m_dataType = other.m_dataType;
        m_arguments = other.m_arguments;
        m_arrayDimensions = other.m_arrayDimensions;
        m_valueRank = other.m_valueRank;
    }
    return *this;
}

UADataType UAVariable::dataType() const
{
    return m_dataType;
}

void UAVariable::setDataType(const UADataType& dataType)
{
    m_dataType = dataType;
}

QList<Argument> UAVariable::arguments() const
{
    return m_arguments;
}

void UAVariable::setArguments(const QList<Argument>& newArguments)
{
    m_arguments = newArguments;
}

int UAVariable::arrayDimensions() const
{
    return m_arrayDimensions;
}

void UAVariable::setArrayDimensions(int newArrayDimensions)
{
    m_arrayDimensions = newArrayDimensions;
}

int UAVariable::valueRank() const
{
    return m_valueRank;
}

void UAVariable::setValueRank(int newValueRank)
{
    m_valueRank = newValueRank;
}

UAMethod::UAMethod(const UAMethod& other)
    : UANode(other)
    , m_inputArgument(
          other.m_inputArgument ? std::make_shared<UAVariable>(*other.m_inputArgument) : nullptr)
    , m_outputArgument(
          other.m_outputArgument ? std::make_shared<UAVariable>(*other.m_outputArgument) : nullptr)
{}

UAMethod& UAMethod::operator=(const UAMethod& other)
{
    if (this != &other) {
        UANode::operator=(other);
        m_inputArgument = other.m_inputArgument
                              ? std::make_shared<UAVariable>(*other.m_inputArgument)
                              : nullptr;
        m_outputArgument = other.m_outputArgument
                               ? std::make_shared<UAVariable>(*other.m_outputArgument)
                               : nullptr;
    }
    return *this;
}

void UAMethod::setInputArgument(std::shared_ptr<UAVariable> var)
{
    m_inputArgument = var;
}

void UAMethod::setOutputArgument(std::shared_ptr<UAVariable> var)
{
    m_outputArgument = var;
}

std::shared_ptr<UAVariable> UAMethod::inputArgument() const
{
    return m_inputArgument;
}

std::shared_ptr<UAVariable> UAMethod::outputArgument() const
{
    return m_outputArgument;
}

UAVariableType::UAVariableType(const UAVariableType& other)
    : UAVariable(other)
    , m_isAbstract(other.m_isAbstract)
{}

UAVariableType& UAVariableType::operator=(const UAVariableType& other)
{
    if (this != &other) {
        UAVariable::operator=(other);
        m_isAbstract = other.m_isAbstract;
    }
    return *this;
}

bool UAVariableType::isAbstract() const
{
    return m_isAbstract;
}

void UAVariableType::setIsAbstract(bool isAbstract)
{
    m_isAbstract = isAbstract;
}

UAObjectType::UAObjectType(const UAObjectType& other)
    : UANode(other)
    , m_isAbstract(other.m_isAbstract)
{}

UAObjectType& UAObjectType::operator=(const UAObjectType& other)
{
    if (this != &other) {
        UANode::operator=(other);
        m_isAbstract = other.m_isAbstract;
    }
    return *this;
}

bool UAObjectType::isAbstract() const
{
    return m_isAbstract;
}

void UAObjectType::setIsAbstract(bool isAbstract)
{
    m_isAbstract = isAbstract;
}

Reference::Reference(const Reference& other)
    : m_referenceType(other.m_referenceType)
    , m_targetNodeId(other.m_targetNodeId)
    , m_isForward(other.m_isForward)
    , m_namespaceString(other.m_namespaceString)
    , m_node(other.m_node)
{}

Reference& Reference::operator=(const Reference& other)
{
    if (this != &other) {
        m_referenceType = other.m_referenceType;
        m_targetNodeId = other.m_targetNodeId;
        m_isForward = other.m_isForward;
        m_namespaceString = other.m_namespaceString;
        m_node = other.m_node;
    }
    return *this;
}

QString Reference::referenceType() const
{
    return m_referenceType;
}

void Reference::setReferenceType(const QString& referenceType)
{
    m_referenceType = referenceType;
}

QString Reference::targetNodeId() const
{
    return m_targetNodeId;
}

void Reference::setTargetNodeId(const QString& targetId)
{
    m_targetNodeId = targetId;
}

bool Reference::isForward() const
{
    return m_isForward;
}

void Reference::setIsForward(bool isForward)
{
    m_isForward = isForward;
}

QString Reference::namespaceString() const
{
    return m_namespaceString;
}

void Reference::setNamespaceString(const QString& newNamespaceString)
{
    m_namespaceString = newNamespaceString;
}

std::shared_ptr<UANode> Reference::node() const
{
    return m_node.lock();
}

void Reference::setNode(std::shared_ptr<UANode> node)
{
    m_node = node;
}
