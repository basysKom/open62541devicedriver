// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

#include "rootnodefiltermodel.h"
#include "treeitem.h"

RootNodeFilterModel::RootNodeFilterModel(QObject* parent)
    : QSortFilterProxyModel{parent}
{}

bool RootNodeFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    return index.isValid() && static_cast<TreeItem*>(index.internalPointer())->isRootNode();
}
