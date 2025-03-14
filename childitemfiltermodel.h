// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CHILDITEMFILTERMODEL_H
#define CHILDITEMFILTERMODEL_H

#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QVariant>

class ChildItemFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QModelIndex rootNodeIndex READ rootNodeIndex WRITE setRootNodeIndex NOTIFY
                   rootNodeIndexChanged)

public:
    explicit ChildItemFilterModel(QObject* parent = nullptr)
        : QSortFilterProxyModel(parent)
    {}

    void setRootNodeIndex(const QModelIndex& index);

    QModelIndex rootNodeIndex() const;

signals:
    void rootNodeIndexChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    QModelIndex m_rootNodeIndex;
};

#endif // CHILDITEMFILTERMODEL_H
