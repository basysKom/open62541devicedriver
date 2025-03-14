// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

#include "uanodeset.h"
#include "Util/Utils.h"

UANodeSet::UANodeSet() {}

UANodeSet::~UANodeSet()
{
    m_nodes.clear();
}

void UANodeSet::addNode(std::shared_ptr<UANode> node)
{
    m_nodes.insert(Utils::instance()->extractIdentifier(node->nodeId()).toInt(), node);
}

QList<std::shared_ptr<UANode>> UANodeSet::nodes() const
{
    return m_nodes.values();
}

QString UANodeSet::getNameSpaceUri() const
{
    return m_uri;
}

void UANodeSet::addNamespaceMapEntry(int index, const QString& namespaceUri)
{
    m_namespaceMap.insert(index, namespaceUri);
}

QString UANodeSet::getNamespaceUriByIndex(int index) const
{
    // NOTE if we are in the UA Nodeset, it will always be index 0
    if (m_namespaceMap.size() == 1 && index != 0) {
        return QStringLiteral("0");
    }
    return m_namespaceMap.value(index);
}

std::shared_ptr<UANode> UANodeSet::findNodeById(const QString& nodeId) const
{
    // only check for the identifier not the namespace since the nodeId might be from another Nodeset
    // this will only return the node if it is in this nodeset
    int identifier = Utils::instance()->extractIdentifier(nodeId).toInt();
    if (m_nodes.contains(identifier)) {
        return m_nodes.value(identifier);
    }
    return nullptr;
}

QMap<int, QString> UANodeSet::namespaceMap() const
{
    return m_namespaceMap;
}

QString UANodeSet::getNodeSetName()
{
    return Utils::instance()->extractNameFromNamespaceString(m_uri);
}

void UANodeSet::addAlias(const QString& alias, const QString& uri)
{
    m_aliasMap.insert(alias, uri);
}

QString UANodeSet::getNodeIdByAlias(const QString& alias) const
{
    return m_aliasMap.value(alias, QStringLiteral(""));
}

bool UANodeSet::getHasCustomTypes() const
{
    return m_hasCustomTypes;
}

void UANodeSet::setHasCustomTypes(bool newHasCustomTypes)
{
    if (m_hasCustomTypes == newHasCustomTypes)
        return;
    m_hasCustomTypes = newHasCustomTypes;
}

void UANodeSet::setNamespaceUri(const QString& newUri)
{
    m_uri = newUri;
}
