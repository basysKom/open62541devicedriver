// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

// treeitem.cpp
#include "treeitem.h"

TreeItem::TreeItem(std::shared_ptr<UANode> node, std::shared_ptr<TreeItem> parentItem)
    : m_node(node)
    , m_parentItem(parentItem)
{
    if (m_parentItem.lock() != nullptr && m_parentItem.lock()->getNode() != nullptr) {
        m_node->setParentNode(m_parentItem.lock()->getNode());
        m_node->setParentNodeId(m_parentItem.lock()->nodeId());
    }
}

TreeItem::~TreeItem()
{
    m_childItems.clear();
    m_parentItem.reset();
}

void TreeItem::appendChild(std::shared_ptr<TreeItem> child)
{
    child->m_parentItem = shared_from_this();
    m_childItems.append(child);
}

void TreeItem::removeChild(int index)
{
    m_childItems.removeAt(index);
}

std::shared_ptr<TreeItem> TreeItem::child(int row)
{
    return m_childItems.value(row);
}

int TreeItem::childCount() const
{
    return m_childItems.count();
}

int TreeItem::row() const
{
    if (m_parentItem.lock())
        return m_parentItem.lock()->m_childItems.indexOf(shared_from_this());
    return 0;
}

QVariant TreeItem::data(const QString role)
{
    return this->property(role.toStdString().c_str());
}

void TreeItem::setData(const QString role, const QVariant& value)
{
    // data function to set data from the userInputMask fields.
    this->setProperty(role.toStdString().c_str(), value);
}

void TreeItem::setValue(const QString valueRole, const QVariant& value)
{
    m_valueMap.insert(valueRole, value);
}

QVariant TreeItem::getValue(const QString valueRole)
{
    return m_valueMap.value(valueRole);
}

std::shared_ptr<TreeItem> TreeItem::parentItem()
{
    return m_parentItem.lock();
}

std::shared_ptr<UANode> TreeItem::getNode() const
{
    return m_node;
}

QString TreeItem::nodeId() const
{
    return m_node->nodeId();
}

void TreeItem::setNodeId(const QString& newNodeId)
{
    if (m_node->nodeId() == newNodeId)
        return;
    m_node->setNodeId(newNodeId);
    emit nodeIdChanged();
}

QString TreeItem::parentNodeId() const
{
    return m_node->parentNodeId();
}

void TreeItem::setParentNodeId(const QString& newParentNodeId)
{
    if (m_node->parentNodeId() == newParentNodeId)
        return;
    m_node->setParentNodeId(newParentNodeId);
    emit parentNodeIdChanged();
}

QString TreeItem::browseName() const
{
    return m_node->browseName();
}

QString TreeItem::baseBrowseName() const
{
    return m_node->baseBrowseName();
}

void TreeItem::setBrowseName(const QString& newBrowseName)
{
    if (m_node->browseName() == newBrowseName)
        return;
    m_node->setBrowseName(newBrowseName);
    emit browseNameChanged();
}

QString TreeItem::displayName() const
{
    return m_node->displayName();
}

void TreeItem::setDisplayName(const QString& newDisplayName)
{
    if (m_node->displayName() == newDisplayName)
        return;
    m_node->setDisplayName(newDisplayName);
    emit displayNameChanged();
}

QList<std::shared_ptr<Reference>> TreeItem::references() const
{
    return m_node->references();
}

QString TreeItem::description() const
{
    return m_node->description();
}

void TreeItem::setDescription(const QString& newDescription)
{
    if (m_node->description() == newDescription)
        return;
    m_node->setDescription(newDescription);
    emit descriptionChanged();
}

std::weak_ptr<UANode> TreeItem::parentNode() const
{
    return m_node->parentNode();
}

void TreeItem::setParentNode(std::weak_ptr<UANode> newParentNode)
{
    auto currentParent = m_node->parentNode().lock();
    auto newParent = newParentNode.lock();

    if (currentParent == newParent)
        return;

    m_node->setParentNode(newParentNode);
    emit parentNodeChanged();
}

QString TreeItem::namespaceString() const
{
    return m_node->namespaceString();
}

void TreeItem::setNamespaceString(const QString& newNamespaceString)
{
    if (m_node->namespaceString() == newNamespaceString)
        return;
    m_node->setNamespaceString(newNamespaceString);
    emit namespaceStringChanged();
}

QString TreeItem::definitionName() const
{
    // only UADataType has a definitionName member
    if (const UADataType* dataType = dynamic_cast<const UADataType*>(m_node.get()))
        return dataType->definitionName();

    if (const UAVariable* variable = dynamic_cast<const UAVariable*>(m_node.get()))
        return variable->dataType().definitionName();

    return QStringLiteral("");
}

void TreeItem::setDefinitionName(const QString& newDefinitionName)
{
    if (UADataType* dataType = dynamic_cast<UADataType*>(m_node.get())) {
        if (dataType->definitionName() == newDefinitionName)
            return;
        dataType->setDefinitionName(newDefinitionName);
        emit definitionNameChanged();
    }
}

QVariantList TreeItem::definitionFields() const
{
    if (const UADataType* dataType = dynamic_cast<const UADataType*>(m_node.get()))
        return Utils::instance()->convertQMapToVariantList(dataType->definitionFields());

    if (const UAVariable* variable = dynamic_cast<const UAVariable*>(m_node.get())) {
        if (variable->dataType().definitionFields().size() > 0) {
            return Utils::instance()->convertQMapToVariantList(
                variable->dataType().definitionFields());
        } else {
            return Utils::instance()->convertQMapToVariantList(
                QMap<QString, QString>{{browseName(), definitionName()}});
        }
    }

    qWarning() << "Access to non existing definitionFields member from " << m_node->typeName();
    return QVariantList();
}

UADataType TreeItem::dataType() const
{
    // only UAVariable and UAVariableType have a dataType member.
    if (const UAVariable* variable = dynamic_cast<const UAVariable*>(m_node.get()))
        return variable->dataType();
    if (const UAVariableType* variableType = dynamic_cast<const UAVariableType*>(m_node.get()))
        return variableType->dataType();

    qWarning() << "Access to non existing UADataType member from " << m_node->typeName();
    return UADataType();
}

void TreeItem::setDataType(const UADataType& newDataType)
{
    if (UAVariable* variable = dynamic_cast<UAVariable*>(m_node.get())) {
        if (variable->dataType() == newDataType)
            return;
        variable->setDataType(newDataType);
    }
    if (UAVariableType* variableType = dynamic_cast<UAVariableType*>(m_node.get())) {
        if (variableType->dataType() == newDataType)
            return;
        variableType->setDataType(newDataType);
    } else {
        qWarning() << "Access to non existing UADataType member from " << m_node->typeName();
        return;
    }
    emit dataTypeChanged();
}

bool TreeItem::isAbstract() const
{
    // only UAVariable and UAVariableType have a dataType member
    if (const UAVariableType* variableType = dynamic_cast<const UAVariableType*>(m_node.get()))
        return variableType->isAbstract();
    if (const UAObjectType* objectType = dynamic_cast<const UAObjectType*>(m_node.get()))
        return objectType->isAbstract();

    qWarning() << "Access to non existing UADataType member from " << m_node->typeName();
    return false;
}

void TreeItem::setIsAbstract(bool newIsAbstract)
{
    if (UAVariableType* variableType = dynamic_cast<UAVariableType*>(m_node.get())) {
        if (variableType->isAbstract() == newIsAbstract)
            return;
        variableType->setIsAbstract(newIsAbstract);
    }
    if (UAObjectType* objectType = dynamic_cast<UAObjectType*>(m_node.get())) {
        if (objectType->isAbstract() == newIsAbstract)
            return;
        objectType->setIsAbstract(newIsAbstract);
    } else {
        qWarning() << "Access to non existing UADataType member from " << m_node->typeName();
        return;
    }
    emit isAbstractChanged();
}

QString TreeItem::referenceType() const
{
    if (const Reference* reference = dynamic_cast<const Reference*>(m_node.get()))
        return reference->referenceType();

    qWarning() << "Access to non existing referenceType member from " << m_node->typeName();
    return QStringLiteral("");
}

void TreeItem::setReferenceType(const QString& newReferenceType)
{
    if (Reference* reference = dynamic_cast<Reference*>(m_node.get())) {
        if (reference->referenceType() == newReferenceType)
            return;
        reference->setReferenceType(newReferenceType);
    } else {
        qWarning() << "Access to non existing referenceType member from " << m_node->typeName();
        return;
    }
}

bool TreeItem::isForward() const
{
    if (const Reference* reference = dynamic_cast<const Reference*>(m_node.get()))
        return reference->isForward();

    qWarning() << "Access to non existing isForward member from " << m_node->typeName();
    return false;
}

bool TreeItem::setIsForward(const bool& newIsForward)
{
    if (Reference* reference = dynamic_cast<Reference*>(m_node.get())) {
        if (reference->isForward() == newIsForward)
            return false;
        reference->setIsForward(newIsForward);
        return true;
    } else {
        qWarning() << "Access to non existing isForward member from " << m_node->typeName();
        return false;
    }
}

QString TreeItem::targetNodeId() const
{
    if (const Reference* reference = dynamic_cast<const Reference*>(m_node.get()))
        return reference->targetNodeId();

    qWarning() << "Access to non existing targetNodeId member from " << m_node->typeName();
    return QStringLiteral("");
}

void TreeItem::setTargetNodeId(const QString& newTargetNodeId)
{
    if (Reference* reference = dynamic_cast<Reference*>(m_node.get())) {
        if (reference->targetNodeId() == newTargetNodeId)
            return;
        reference->setTargetNodeId(newTargetNodeId);
    } else {
        qWarning() << "Access to non existing targetNodeId member from " << m_node->typeName();
        return;
    }
    emit targetNodeIdChanged();
}
UANode* TreeItem::referenceNode() const
{
    if (const Reference* reference = dynamic_cast<const Reference*>(m_node.get()))
        return reference->node().get();

    qWarning() << "Access to non existing referenceNode member from " << m_node->typeName();
    return nullptr;
}

void TreeItem::setReferenceNode(UANode* newNode)
{
    if (Reference* reference = dynamic_cast<Reference*>(m_node.get())) {
        if (reference->node().get() == newNode)
            return;

        reference->setNode(newNode->clone());
    } else {
        qWarning() << "Access to non existing referenceNode member from " << m_node->typeName();
        return;
    }
    emit referenceTypeChanged();
}

bool TreeItem::isOptional() const
{
    return m_node->isOptional();
}

void TreeItem::setIsOptional(bool newIsOptional)
{
    if (m_node->isOptional() == newIsOptional)
        return;
    m_node->setIsOptional(newIsOptional);
    emit isOptionalChanged();
}

QString TreeItem::typeName() const
{
    return m_node->typeName();
}

QStringList TreeItem::userInputMask() const
{
    if (UAVariable* variable = dynamic_cast<UAVariable*>(m_node.get())) {
        return {
            QStringLiteral("displayName"),
            QStringLiteral("description"),
            QStringLiteral("browseName")};
    }
    return {
        QStringLiteral("displayName"),
        QStringLiteral("description"),
        QStringLiteral("browseName"),
        QStringLiteral("nodeId"),
    };
}

bool TreeItem::isRootNode() const
{
    return m_node->isRootNode();
}

void TreeItem::setIsRootNode(bool newIsRootNode)
{
    if (m_node->isRootNode() == newIsRootNode)
        return;
    m_node->setIsRootNode(newIsRootNode);
    emit isRootNodeChanged();
}

bool TreeItem::isSelected() const
{
    return m_isSelected;
}

void TreeItem::updateChildrenSelected(bool selected)
{
    for (int i = 0; i < m_childItems.count(); ++i) {
        TreeItem* child = m_childItems.at(i).get();
        child->setIsParentSelected(selected);

        if (selected && !child->isOptional()) {
            child->setSelected(selected);
        }
        if (child->isSelected() && !selected) {
            child->setSelected(selected);
        }
    }
}

void TreeItem::setSelected(bool newIsSelected)
{
    if (m_isSelected == newIsSelected)
        return;
    m_isSelected = newIsSelected;
    updateChildrenSelected(newIsSelected);
    emit isSelectedChanged();
}

QString TreeItem::nodeVariableName() const
{
    return m_node->nodeVariableName();
}

bool TreeItem::isParentSelected() const
{
    if (!isRootNode() && m_parentItem.lock()->isRootNode())
        return true;
    return m_isParentSelected;
}

void TreeItem::setIsParentSelected(bool newIsParentSelected)
{
    if (m_isParentSelected == newIsParentSelected)
        return;
    m_isParentSelected = newIsParentSelected;
    if (!m_isParentSelected) {
        setSelected(false);
        emit forceUpdate();
    }
    emit isParentSelectedChanged();
}

QString TreeItem::uniqueBaseBrowseName() const
{
    return m_node->uniqueBaseBrowseName();
}

void TreeItem::setUniqueBaseBrowseName(const QString& newUniqueBaseBrowseName)
{
    if (m_node->uniqueBaseBrowseName() == newUniqueBaseBrowseName)
        return;
    m_node->setUniqueBaseBrowseName(newUniqueBaseBrowseName);
    emit uniqueBaseBrowseNameChanged();
}
