// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

// treeitem.h
#ifndef TREEITEM_H
#define TREEITEM_H

#include "uanode.h"
#include <QObject>
#include <QVariant>

class TreeItem : public QObject, public std::enable_shared_from_this<TreeItem>
{
    Q_OBJECT

    // UANode API properties
    Q_PROPERTY(QString nodeId READ nodeId WRITE setNodeId NOTIFY nodeIdChanged)
    Q_PROPERTY(QString browseName READ browseName WRITE setBrowseName NOTIFY browseNameChanged)
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)
    Q_PROPERTY(QString typeName READ typeName NOTIFY typeNameChanged)
    Q_PROPERTY(QList<std::shared_ptr<Reference>> references READ references NOTIFY referencesChanged)
    Q_PROPERTY(
        QString parentNodeId READ parentNodeId WRITE setParentNodeId NOTIFY parentNodeIdChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(std::weak_ptr<UANode> parentNode READ parentNode WRITE setParentNode NOTIFY
                   parentNodeChanged)
    Q_PROPERTY(QString namespaceString READ namespaceString WRITE setNamespaceString NOTIFY
                   namespaceStringChanged)
    Q_PROPERTY(bool isOptional READ isOptional WRITE setIsOptional NOTIFY isOptionalChanged)
    Q_PROPERTY(QStringList userInputMask READ userInputMask NOTIFY userInputMaskChanged)
    Q_PROPERTY(bool isRootNode READ isRootNode WRITE setIsRootNode NOTIFY isRootNodeChanged)
    Q_PROPERTY(bool isSelected READ isSelected WRITE setSelected NOTIFY isSelectedChanged)
    Q_PROPERTY(QString nodeVariableName READ nodeVariableName NOTIFY nodeVariableNameChanged)
    Q_PROPERTY(bool isParentSelected READ isParentSelected WRITE setIsParentSelected NOTIFY
                   isParentSelectedChanged)

    // UADataType extra properties
    Q_PROPERTY(QString definitionName READ definitionName WRITE setDefinitionName NOTIFY
                   definitionNameChanged)
    Q_PROPERTY(QVariantList definitionFields READ definitionFields NOTIFY definitionFieldsChanged)

    // UAVariable extra properties
    Q_PROPERTY(UADataType dataType READ dataType WRITE setDataType NOTIFY dataTypeChanged)

    // UAVariableType and UAObjectType extra properties
    Q_PROPERTY(bool isAbstract READ isAbstract WRITE setIsAbstract NOTIFY isAbstractChanged)

    // Reference extra properties
    Q_PROPERTY(
        QString referenceType READ referenceType WRITE setReferenceType NOTIFY referenceTypeChanged)
    Q_PROPERTY(
        QString targetNodeId READ targetNodeId WRITE setTargetNodeId NOTIFY targetNodeIdChanged)
    Q_PROPERTY(bool isForward READ isForward WRITE setIsForward NOTIFY isForwardChanged)
    Q_PROPERTY(
        UANode* referenceNode READ referenceNode WRITE setReferenceNode NOTIFY referenceNodeChanged)

public:
    explicit TreeItem(std::shared_ptr<UANode> node, std::shared_ptr<TreeItem> parentItem = nullptr);
    TreeItem() {}
    ~TreeItem();

    void appendChild(std::shared_ptr<TreeItem> child);
    void removeChild(int index);
    std::shared_ptr<TreeItem> child(int row);
    int childCount() const;
    int row() const;

    Q_INVOKABLE QVariant data(const QString role);
    Q_INVOKABLE void setData(const QString role, const QVariant& value);
    Q_INVOKABLE void setValue(const QString valueRole, const QVariant& value);
    Q_INVOKABLE QVariant getValue(const QString valueRole);

    std::shared_ptr<TreeItem> parentItem();
    std::shared_ptr<UANode> getNode() const;

    QString nodeId() const;
    void setNodeId(const QString& newNodeId);

    QString parentNodeId() const;
    void setParentNodeId(const QString& newParentNodeId);

    QString browseName() const;
    void setBrowseName(const QString& newBrowseName);

    QString baseBrowseName() const;

    QString displayName() const;
    void setDisplayName(const QString& newDisplayName);

    QList<std::shared_ptr<Reference>> references() const;

    QString description() const;
    void setDescription(const QString& newDescription);

    std::weak_ptr<UANode> parentNode() const;
    void setParentNode(std::weak_ptr<UANode> newParentNode);

    QString namespaceString() const;
    void setNamespaceString(const QString& newNamespaceString);

    QString definitionName() const;
    void setDefinitionName(const QString& newDefinitionName);

    QVariantList definitionFields() const;

    UADataType dataType() const;
    void setDataType(const UADataType& newDataType);

    bool isAbstract() const;
    void setIsAbstract(bool newIsAbstract);

    QString referenceType() const;
    void setReferenceType(const QString& newReferenceType);

    bool isForward() const;
    bool setIsForward(const bool& newIsForward);

    QString targetNodeId() const;
    void setTargetNodeId(const QString& newTargetNodeId);

    UANode* referenceNode() const;
    void setReferenceNode(UANode* newNode);

    bool isOptional() const;
    void setIsOptional(bool newIsOptional);

    QString typeName() const;

    QStringList userInputMask() const;

    bool isRootNode() const;
    void setIsRootNode(bool newIsRootNode);

    bool isSelected() const;
    void setSelected(bool newIsSelected);

    QVariant value() const;
    void setValue(const QVariant& newValue);

    QStringList valueModel() const;

    int valueModelIndex() const;
    void setValueModelIndex(int newValueModelIndex);

    QString nodeVariableName() const;

    bool isParentSelected() const;
    void setIsParentSelected(bool newIsParentSelected);

signals:
    void nodeIdChanged();
    void parentNodeIdChanged();
    void browseNameChanged();
    void displayNameChanged();
    void referencesChanged();
    void descriptionChanged();
    void parentNodeChanged();
    void namespaceStringChanged();
    void definitionNameChanged();
    void definitionFieldsChanged();
    void dataTypeChanged();
    void isAbstractChanged();
    void referenceTypeChanged();
    void isForwardChanged();
    void targetNodeIdChanged();
    void referenceNodeChanged();
    void isOptionalChanged();
    void typeNameChanged();
    void isRootNodeChanged();
    void isSelectedChanged();
    void valueChanged();
    void valueModelChanged();
    void valueModelIndexChanged();
    void userInputMaskChanged();
    void nodeVariableNameChanged();
    void isParentSelectedChanged();
    void forceUpdate();

private:
    QList<std::shared_ptr<TreeItem>> m_childItems;
    std::shared_ptr<UANode> m_node;
    std::weak_ptr<TreeItem> m_parentItem;
    QStringList m_userInputMask;
    QMap<QString, QVariant> m_valueMap;

    bool m_isSelected = false;
    bool m_isParentSelected = false;
    void updateChildrenSelected(bool selected);
};

Q_DECLARE_METATYPE(TreeItem)

#endif // TREEITEM_H
