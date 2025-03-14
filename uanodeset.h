// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef UANODESET_H
#define UANODESET_H

#include "treeitem.h"
#include "uanode.h"
#include <QHash>
#include <QRegularExpression>

class UANodeSet
{
public:
    UANodeSet();
    ~UANodeSet();

    void addNode(std::shared_ptr<UANode> node);
    QList<std::shared_ptr<UANode>> nodes() const;

    QString getNameSpaceUri() const;
    void setNamespaceUri(const QString& newUri);

    void addNamespaceMapEntry(int index, const QString& namespaceUri);
    QString getNamespaceUriByIndex(int index) const;

    std::shared_ptr<UANode> findNodeById(const QString& nodeId) const;

    QMap<int, QString> namespaceMap() const;

    QString getNodeSetName();

    void addAlias(const QString& alias, const QString& uri);
    QString getNodeIdByAlias(const QString& alias) const;

    bool getHasCustomTypes() const;
    void setHasCustomTypes(bool newHasCustomTypes);

private:
    // QMap with the identifier as key without the namespace
    QMap<int, std::shared_ptr<UANode>> m_nodes;

    // mappping of namespace index to namespace uri for the nodeset
    QMap<int, QString> m_namespaceMap;
    QMap<QString, QString> m_aliasMap;

    bool m_hasCustomTypes = false;

    QString m_uri;

    void resolveNamespaceMapping(TreeItem* item);
};

#endif // UANODESET_H
