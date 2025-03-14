// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

// treemodel.h
#ifndef TREEMODEL_H
#define TREEMODEL_H

#include "treeitem.h"
#include "uanodeset.h"
#include <QAbstractItemModel>
#include <QPointer>

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TreeModel(QObject* parent = nullptr);
    ~TreeModel();

    enum Roles {
        NodeIdRole = Qt::UserRole + 1,
        BrowseNameRole,
        DisplayNameRole,
        DescriptionRole,
        ReferencesRole,
        ParentNodeIdRole,
        ParentNode,
        DefinitionNameRole,
        DefinitionFieldsRole,
        DataTypeRole,
        IsAbstractRole,
        ReferenceTypeRole,
        TargetNodeIdRole,
        IsForwardRole,
        ReferenceNodeRole,
        NamespaceStringRole,
        IsOptionalRole,
        TypeNameRole,
        IsRootNodeRole,
        IsSelectedRole,
        ValueRole,
        NodeIdVariableNameRole,
        UserInputMaskRole,
        ValueModelIndexRole,
        ValueModelRole,
        IsParentSelectedRole,
        ItemRole
    };

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    void setupModelData(std::shared_ptr<UANodeSet> nodeSet);
    void addRootNode(
        std::shared_ptr<UANode> node,
        bool resolveSelection = false,
        bool useUniqueBrowseNames = false);
    void addRootNodeToSelection(std::shared_ptr<UANode> node);
    void removeRootNodeFromSelection(const int index);
    void resetModel();

    Q_INVOKABLE std::shared_ptr<TreeItem> getCurrentItem() const;
    Q_INVOKABLE void setCurrentItem(int indexInteger);
    Q_INVOKABLE std::shared_ptr<TreeItem> getItemByName(const QString& name) const;
    Q_INVOKABLE bool isBrowseNameUnique(const QString& name) const;
    Q_INVOKABLE QModelIndex getIndexFromItem(TreeItem* item) const;
    Q_INVOKABLE TreeItem* getItemFromIndex(const QModelIndex& index) const;
    Q_INVOKABLE int getRoleByName(const QString& roleName) const;

    std::shared_ptr<TreeItem> rootItem() const;
    QString makeBrowseNameUnique(const QString& browseName) const;

private slots:
    void onForceUpdate()
    {
        TreeItem* item = qobject_cast<TreeItem*>(sender());
        if (item) {
            QModelIndex index = createIndex(item->row(), 0, item);
            emit dataChanged(index, index, {IsSelectedRole, IsParentSelectedRole});
        }
    }

private:
    std::shared_ptr<TreeItem> m_rootItem;
    std::shared_ptr<TreeItem> m_currentItem;

    void addChildNodes(std::shared_ptr<TreeItem> parent, bool useUniqueBrowseNames = false);
    void collectInheritedNodes(std::shared_ptr<UANode> node, QSet<std::shared_ptr<UANode>>& nodes);
    bool isValidChildNode(const std::shared_ptr<Reference> reference) const;
    void addNodeToTree(
        std::shared_ptr<TreeItem> parentItem,
        std::shared_ptr<UANode> childNode,
        bool useUniqueBrowseNames = false);
    bool shouldAddInheritedNode(const std::shared_ptr<Reference> reference) const;
    std::shared_ptr<TreeItem> findItemByBrowseNameRecursive(
        std::shared_ptr<TreeItem> parent, const QString& name) const;
    QString findBrowseNameRecursive(std::shared_ptr<TreeItem> parent, const QString& name) const;
    void setItemSelected(TreeItem* item, bool selected, bool emitSignal = false);
    void setItemSelectedRecursive(TreeItem* item, bool selected, bool emitSignal);
    void emitDataChangedRecursive(const QModelIndex& parentIndex);
};

#endif // TREEMODEL_H
