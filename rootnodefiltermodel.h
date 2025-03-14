// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ROOTNODEFILTERMODEL_H
#define ROOTNODEFILTERMODEL_H

#include <QObject>
#include <QSortFilterProxyModel>

class RootNodeFilterModel : public QSortFilterProxyModel
{
public:
    explicit RootNodeFilterModel(QObject* parent = nullptr);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
};

#endif // ROOTNODEFILTERMODEL_H
