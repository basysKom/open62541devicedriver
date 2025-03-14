// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

// treemodel.cpp
#include "treemodel.h"
#include "Util/Utils.h"

TreeModel::TreeModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    m_rootItem = std::make_shared<TreeItem>();
}

TreeModel::~TreeModel()
{
    m_currentItem.reset();
    m_rootItem.reset();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem* parentItem;

    if (!parent.isValid())
        parentItem = m_rootItem.get();
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    TreeItem* childItem = parentItem->child(row).get();
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem* childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem* parentItem = childItem->parentItem().get();

    if (parentItem == m_rootItem.get())
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex& parent) const
{
    TreeItem* parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_rootItem.get();
    else
        parentItem = static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int TreeModel::columnCount(const QModelIndex& parent) const
{
    return 1;
}

QVariant TreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());

    // TODO check if we need all those roles. If not, remove them and maybe go and remove them from the UANode as well.
    switch (role) {
    case Qt::DisplayRole:
        return item->displayName();
    case NodeIdRole:
        return item->nodeId();
    case BrowseNameRole:
        return item->browseName();
    case DisplayNameRole:
        return item->displayName();
    case DescriptionRole:
        return item->description();
    case ReferencesRole:
        return QVariant::fromValue(item->references());
    case ParentNodeIdRole:
        return item->parentNodeId();
    case DefinitionNameRole:
        return item->definitionName();
    case DefinitionFieldsRole:
        return QVariant::fromValue(item->definitionFields());
    case DataTypeRole:
        return QVariant::fromValue(item->dataType());
    case IsAbstractRole:
        return item->isAbstract();
    case ReferenceTypeRole:
        return item->referenceType();
    case TargetNodeIdRole:
        return item->targetNodeId();
    case IsForwardRole:
        return item->isForward();
    case ReferenceNodeRole:
        return QVariant::fromValue(item->referenceNode());
    case NamespaceStringRole:
        return item->namespaceString();
    case IsOptionalRole:
        return item->isOptional();
    case TypeNameRole:
        return item->typeName();
    case IsRootNodeRole:
        return item->isRootNode();
    case IsSelectedRole:
        return item->isSelected();
    case NodeIdVariableNameRole:
        return item->nodeVariableName();
    case UserInputMaskRole:
        return item->userInputMask();
    case IsParentSelectedRole:
        return item->isParentSelected();
    case ItemRole:
        return QVariant::fromValue(item);
    }

    return QVariant();
}

bool TreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
        return false;

    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    switch (role) {
    case Qt::DisplayRole:
        item->setDisplayName(value.toString());
        emit dataChanged(index, index, {role});
        return true;
    case NodeIdRole:
        item->setNodeId(value.toString());
        emit dataChanged(index, index, {role});
        return true;
    case BrowseNameRole:
        item->setBrowseName(value.toString());
        emit dataChanged(index, index, {role});
        return true;
    case DisplayNameRole:
        item->setDisplayName(value.toString());
        emit dataChanged(index, index, {role});
        return true;
    case DescriptionRole:
        item->setDescription(value.toString());
        emit dataChanged(index, index, {role});
        return true;
    case ParentNodeIdRole:
        item->setParentNodeId(value.toString());
        emit dataChanged(index, index, {role});
        return true;
    case DefinitionNameRole:
        item->setDefinitionName(value.toString());
        emit dataChanged(index, index, {role});
        return true;
    case DataTypeRole:
        item->setDataType(value.value<UADataType>());
        emit dataChanged(index, index, {role});
        return true;
    case IsAbstractRole:
        item->setIsAbstract(value.toBool());
        emit dataChanged(index, index, {role});
        return true;
    case ReferenceTypeRole:
        item->setReferenceType(value.toString());
        emit dataChanged(index, index, {role});
        return true;
    case TargetNodeIdRole:
        item->setTargetNodeId(value.toString());
        emit dataChanged(index, index, {role});
        return true;
    case IsForwardRole:
        item->setIsForward(value.toBool());
        emit dataChanged(index, index, {role});
        return true;
    case ReferenceNodeRole:
        item->setReferenceNode(value.value<UANode*>());
        emit dataChanged(index, index, {role});
        return true;
    case NamespaceStringRole:
        item->setNamespaceString(value.toString());
        emit dataChanged(index, index, {role});
        return true;
    case IsOptionalRole:
        item->setIsOptional(value.toBool());
        emit dataChanged(index, index, {role});
        return true;
    case TypeNameRole:
        //
        return true;
    case IsRootNodeRole:
        item->setIsRootNode(value.toBool());
        emit dataChanged(index, index, {role});
        return true;
    case IsSelectedRole:
        item->setSelected(value.toBool());
        emit dataChanged(
            index,
            index.sibling(index.row(), columnCount() - 1),
            {IsSelectedRole, IsParentSelectedRole});

        // If the item has children, emit dataChanged for all descendants
        if (rowCount(index) > 0) {
            emit dataChanged(
                getIndexFromItem(item->child(0).get()),
                getIndexFromItem(item->child(rowCount(index) - 1).get()),
                {IsSelectedRole, IsParentSelectedRole});
        }
        return true;
    // TODO Do we allow this? Rightt now the varaible name depends on the browsename and nodeid
    case NodeIdVariableNameRole:
        qWarning() << "Setting the variable name is not supported right now!";
        return false;
    case IsParentSelectedRole:
        item->setIsParentSelected(value.toBool());
        emit dataChanged(index, index, {role});
    }

    return false;
}

std::shared_ptr<TreeItem> TreeModel::getCurrentItem() const
{
    return m_currentItem;
}

void TreeModel::setCurrentItem(int indexInteger)
{
    if (indexInteger < 0 || indexInteger >= m_rootItem->childCount()) {
        qDebug() << "Index is out of range!";
        return;
    }

    QModelIndex index = createIndex(indexInteger, 0, m_rootItem->child(indexInteger).get());

    if (index.isValid()) {
        TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
        if (item) {
            m_currentItem = std::shared_ptr<TreeItem>(item);
            return;
        }
    } else {
        qDebug() << "Index is not valid!";
    }
}

QModelIndex TreeModel::getIndexFromItem(TreeItem* item) const
{
    return createIndex(item->row(), 0, item);
}

TreeItem* TreeModel::getItemFromIndex(const QModelIndex& index) const
{
    if (!index.isValid())
        return nullptr;

    return static_cast<TreeItem*>(index.internalPointer());
}

int TreeModel::getRoleByName(const QString& roleName) const
{
    auto roles = roleNames();
    for (auto [key, value] : roles.asKeyValueRange()) {
        if (value == roleName.toUtf8()) {
            return key;
        }
    }

    return -1;
}

std::shared_ptr<TreeItem> TreeModel::rootItem() const
{
    return m_rootItem;
}

std::shared_ptr<TreeItem> TreeModel::getItemByName(const QString& name) const
{
    auto found = findItemByBrowseNameRecursive(m_rootItem, name);
    return found;
}

bool TreeModel::isBrowseNameUnique(const QString& name) const
{
    return findBrowseNameRecursive(m_rootItem, name) == QStringLiteral("");
}

QString TreeModel::findBrowseNameRecursive(std::shared_ptr<TreeItem> parent, const QString& name) const
{
    if (parent->getNode() != nullptr)
        if (parent->browseName() == name)
            return parent->browseName();

    for (int i = 0; i < parent->childCount(); ++i) {
        QString foundName = findBrowseNameRecursive(parent->child(i), name);
        if (!foundName.isEmpty()) {
            return foundName;
        }
    }

    // Not found
    return QStringLiteral("");
}

std::shared_ptr<TreeItem> TreeModel::findItemByBrowseNameRecursive(
    std::shared_ptr<TreeItem> parent, const QString& name) const
{
    if (parent->getNode() != nullptr)
        if (parent->getNode()->browseName() == name)
            return parent;

    for (int i = 0; i < parent->childCount(); ++i) {
        std::shared_ptr<TreeItem> foundItem = findItemByBrowseNameRecursive(parent->child(i), name);
        if (foundItem != nullptr) {
            return foundItem;
        }
    }

    // Not found
    return nullptr;
}

QHash<int, QByteArray> TreeModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NodeIdRole] = "nodeId";
    roles[BrowseNameRole] = "browseName";
    roles[DisplayNameRole] = "displayName";
    roles[DescriptionRole] = "description";
    roles[ReferencesRole] = "references";
    roles[ParentNodeIdRole] = "parentNodeId";
    roles[ParentNode] = "parentNode";
    roles[DefinitionNameRole] = "definitionName";
    roles[DefinitionFieldsRole] = "definitionFields";
    roles[DataTypeRole] = "dataType";
    roles[IsAbstractRole] = "isAbstract";
    roles[ReferenceTypeRole] = "referenceType";
    roles[TargetNodeIdRole] = "targetNodeId";
    roles[IsForwardRole] = "isForward";
    roles[ReferenceNodeRole] = "referenceNode";
    roles[NamespaceStringRole] = "namespaceString";
    roles[IsOptionalRole] = "isOptional";
    roles[TypeNameRole] = "typeName";
    roles[IsRootNodeRole] = "isRootNode";
    roles[IsSelectedRole] = "isSelected";
    roles[NodeIdVariableNameRole] = "name";
    roles[UserInputMaskRole] = "userInputMask";
    roles[IsParentSelectedRole] = "isParentSelected";
    roles[ItemRole] = "item";

    return roles;
}

void TreeModel::setupModelData(std::shared_ptr<UANodeSet> nodeSet)
{
    beginResetModel();
    m_rootItem.reset();
    m_rootItem = std::make_shared<TreeItem>();

    for (const auto& node : nodeSet->nodes()) {
        // NOTE we want to skip the abstract nodes and only take ObjectType nodes
        if (node->typeName() == XmlTags::UAObjectType) {
            std::shared_ptr<UAObjectType> object = std::dynamic_pointer_cast<UAObjectType>(node);
            if (object->isAbstract())
                continue;
            addRootNode(node, false, false);
        }
    }
    endResetModel();
}

void TreeModel::addRootNode(
    std::shared_ptr<UANode> node, bool resolveSelection, bool useUniqueBrowseNames)
{
    if (useUniqueBrowseNames)
        node->setBrowseName(makeBrowseNameUnique(node->browseName()));
    node->setIsRootNode(true);
    std::shared_ptr<TreeItem> rootItem = std::make_shared<TreeItem>(node, m_rootItem);
    connect(rootItem.get(), &TreeItem::forceUpdate, this, &TreeModel::onForceUpdate);

    m_rootItem->appendChild(rootItem);
    addChildNodes(rootItem, useUniqueBrowseNames);

    // Collect and add inherited nodes
    QSet<std::shared_ptr<UANode>> inheritedNodes;
    collectInheritedNodes(node, inheritedNodes);

    for (const std::shared_ptr<UANode>& inheritedNode : std::as_const(inheritedNodes)) {
        if (!inheritedNode)
            continue;
        for (const auto& reference : inheritedNode->references()) {
            // FIXME we should allow to generate optional nodes but then there are duplicates...
            if (isValidChildNode(reference) /* && !reference->node()->isOptional()*/) {
                addNodeToTree(rootItem, reference->node(), useUniqueBrowseNames);
            }
        }
    }

    if (resolveSelection) {
        // Setting selected state for root node and children
        rootItem->setSelected(true);
    }
}

void TreeModel::addRootNodeToSelection(std::shared_ptr<UANode> node)
{
    int newRowIndex = m_rootItem->childCount();
    beginInsertRows(QModelIndex(), newRowIndex, newRowIndex);
    addRootNode(node, true, true);
    endInsertRows();
}

void TreeModel::removeRootNodeFromSelection(const int index)
{
    if (index >= 0 && index < m_rootItem->childCount()) {
        beginRemoveRows(QModelIndex(), index, index);
        m_rootItem->removeChild(index);
        endRemoveRows();
    }
}

void TreeModel::resetModel()
{
    beginResetModel();

    m_rootItem = std::make_shared<TreeItem>();
    m_currentItem.reset();

    endResetModel();
}

void TreeModel::collectInheritedNodes(
    std::shared_ptr<UANode> node, QSet<std::shared_ptr<UANode>>& nodes)
{
    if (!node)
        return;

    for (const auto& reference : node->references()) {
        if (shouldAddInheritedNode(reference)) {
            std::shared_ptr<UANode> refNode = reference->node();
            if (!nodes.contains(refNode)) {
                nodes.insert(refNode);
                collectInheritedNodes(refNode, nodes);
            }
        }
    }
}

void TreeModel::addChildNodes(std::shared_ptr<TreeItem> parent, bool useUniqueBrowseNames)
{
    if (!parent)
        return;
    auto node = parent->getNode();
    if (!node)
        return;

    static QSet<QString> visitedNodes;
    QString nodeId = node->nodeId();

    if (visitedNodes.contains(nodeId))
        return;

    visitedNodes.insert(nodeId);

    for (const auto& reference : node->references()) {
        if (isValidChildNode(reference) && reference->isForward()) {
            addNodeToTree(parent, reference->node(), useUniqueBrowseNames);
        }
    }

    visitedNodes.remove(nodeId);
}

bool TreeModel::isValidChildNode(const std::shared_ptr<Reference> reference) const
{
    if (!reference->node())
        return false;
    const QString& typeName = reference->node()->typeName();
    if (typeName != XmlTags::UAObject && typeName != XmlTags::UAMethod
        && typeName != XmlTags::UAVariable)
        return false;

    const QString& browseName = reference->node()->browseName();
    if (browseName.contains(XmlTags::Mandatory) || browseName.contains(XmlTags::Optional)
        || browseName.contains(XmlTags::Arguments))
        return false;

    return true;
}

bool TreeModel::shouldAddInheritedNode(const std::shared_ptr<Reference> reference) const
{
    if (reference->isForward())
        return false;

    const QString& refType = reference->referenceType();
    return (refType == XmlTags::HasSubtype || refType == XmlTags::HasTypeDefinition);
}

void TreeModel::addNodeToTree(
    std::shared_ptr<TreeItem> parentItem,
    std::shared_ptr<UANode> childNode,
    bool useUniqueBrowseNames)
{
    if (!parentItem || !childNode)
        return;

    auto cildNodeCopy = childNode->clone();
    if (useUniqueBrowseNames)
        cildNodeCopy->setBrowseName(makeBrowseNameUnique(cildNodeCopy->browseName()));

    auto parentNamespaceMap = Utils::instance()->currentNameSpaceMaps().value(
        parentItem->namespaceString());
    if (parentItem->namespaceString() != cildNodeCopy->namespaceString()) {
        cildNodeCopy->changeNamespaceId(parentNamespaceMap.key(cildNodeCopy->namespaceString()));
    }

    std::shared_ptr<TreeItem> childItem = std::make_shared<TreeItem>(cildNodeCopy, parentItem);
    connect(childItem.get(), &TreeItem::forceUpdate, this, &TreeModel::onForceUpdate);

    parentItem->appendChild(childItem);

    addChildNodes(childItem, useUniqueBrowseNames);
}

QString TreeModel::makeBrowseNameUnique(const QString& browseName) const
{
    QString baseBrowseName = browseName;
    QString currentBrowseName = baseBrowseName;
    int browseNameSuffix = 1;

    while (!isBrowseNameUnique(currentBrowseName)) {
        currentBrowseName = baseBrowseName + QStringLiteral("_")
                            + QString::number(browseNameSuffix);
        browseNameSuffix++;
    }

    return currentBrowseName;
}
