// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

#include "childitemfiltermodel.h"
#include "treeitem.h"

bool ChildItemFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    TreeItem* item = static_cast<TreeItem*>(sourceIndex.internalPointer());
    TreeItem* rootNodeItem = m_rootNodeIndex.isValid()
                                 ? static_cast<TreeItem*>(m_rootNodeIndex.internalPointer())
                                 : nullptr;

    if (item != nullptr && item->getNode() != nullptr && item == rootNodeItem) {
        return true;
    }

    TreeItem* parentItem = item->parentItem().get();

    if (parentItem != nullptr && parentItem->getNode() != nullptr && parentItem == rootNodeItem) {
        return true;
    }

    while (parentItem->parentItem() != nullptr) {
        parentItem = parentItem->parentItem().get();
        if (parentItem == rootNodeItem) {
            return true;
        }
    }

    return false;
}

void ChildItemFilterModel::setRootNodeIndex(const QModelIndex& index)
{
    if (m_rootNodeIndex != index) {
        m_rootNodeIndex = index;

        invalidateFilter();
        emit rootNodeIndexChanged();
    }
}

QModelIndex ChildItemFilterModel::rootNodeIndex() const
{
    return m_rootNodeIndex;
}
