// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DEVICEDRIVERCORE_H
#define DEVICEDRIVERCORE_H

#include "childitemfiltermodel.h"
#include "mustache.hpp"
#include "rootnodefiltermodel.h"
#include "treemodel.h"
#include "uanodesetparser.h"

#include <QDir>
#include <QFileInfoList>
#include <QStringList>
#ifdef WASM_BUILD
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#endif

#include <unordered_map>
#include <vector>

using namespace kainjow;

class DeviceDriverCore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(TreeModel* deviceTypesModel READ deviceTypesModel CONSTANT)
    Q_PROPERTY(TreeModel* selectionModel READ selectionModel NOTIFY selectionModelChanged)
    Q_PROPERTY(ChildItemFilterModel* childItemFilterModel READ childItemFilterModel NOTIFY
                   childItemFilterModelChanged)
    Q_PROPERTY(RootNodeFilterModel* rootNodeFilterModel READ rootNodeFilterModel NOTIFY
                   rootNodeFilterModelChanged)
    Q_PROPERTY(QString nodeSetPath READ nodeSetPath WRITE setNodeSetPath NOTIFY selectionModelChanged)

public:
    DeviceDriverCore();
    ~DeviceDriverCore();

    Q_INVOKABLE void selectNodeSetXML(const QString& nodeSetDir);
    Q_INVOKABLE void addRootNodeToSelectionModel(
        const QString& namespaceString, const QString& nodeId);
    Q_INVOKABLE void removeRootNodeFromSelection(const int index);
    Q_INVOKABLE void generateCode(bool includeCmake, bool includeJson);
    Q_INVOKABLE void saveToJson(const QString& fileName, const QJsonDocument& content);
    Q_INVOKABLE void loadState(const QString& filePath);
    Q_INVOKABLE void saveProject();

    TreeModel* deviceTypesModel() const;
    std::shared_ptr<UANode> findNodeById(const QString& namespaceString, const QString& nodeId) const;

    void setNodeSetPath(const QString& newNodeSetPath);
    void setMustacheTemplatePath(const QString& newMustacheTemplatePath);
    void setCmakeMustacheTemplatePath(const QString& newCmakeMustacheTemplatePath);

    TreeModel* selectionModel() const;

    ChildItemFilterModel* childItemFilterModel() const;

    void setOutputFilePath(const QString& newOutputFilePath);
    void setJsonOutputFilePath(const QString& newJsonOutputFilePath);

    QString nodeSetPath() const;

    RootNodeFilterModel* rootNodeFilterModel() const;

    void setCmakeOutputFilePath(const QString& newCmakeOutputFilePath);

    void printMustacheData(const QJsonDocument& jsonDoc);
signals:
    void selectionModelChanged();
    void childItemFilterModelChanged();
    void rootNodeFilterModelChanged();
    void setupFinished();

private:
    TreeModel* m_deviceTypesModel = nullptr;
    TreeModel* m_selectionModel = nullptr;
    ChildItemFilterModel* m_childItemFilterModel = nullptr;

    UaNodeSetParser m_parser;
    QMap<QString, std::shared_ptr<UANodeSet>> m_nodeSets;

    QString m_nodeSetPath;
    QString m_mustacheTemplatePath;
    QString m_cmakeMustacheTemplatePath;
    QString m_outputFilePath;
    QString m_jsonOutputFilePath;
    QString m_cmakeOutputFilePath;
    QString m_selectedModelUri;
    QString m_currentNodeSetDir;

    QStringList findRequiredModels(const QString& fileName);
    QStringList findRequiredFiles(const QStringList& models, bool xmlOnly = true);
    QStringList getAllNodesetFiles(const QString& dir);
    QString getNodeSetXmlFile(const QString& dir);

    void parseNodeSets(const QString& nodeSetDir);
    void resolveNodeReferences(
        std::shared_ptr<UANode> node, QSet<std::shared_ptr<UANode>>& visitedNodes);
    void resolveParentNode();
    void resolveReferences();
    void resolveDataTypes();
    void resolveMethods();

    QString parentReferenceNodeId(std::shared_ptr<UANode> node);

    QList<TreeItem*> getSelectedItems();
    void travereseTreeModel(
        const QAbstractItemModel* model,
        QList<TreeItem*>& items,
        const QModelIndex& parent = QModelIndex());

    std::pair<std::unordered_map<std::string, mustache::data>, QJsonDocument> getMustacheData();
    std::pair<mustache::data, QJsonDocument> getCMakeMustacheData();

    std::unordered_map<std::string, mustache::data> createNodeMap(
        int index, TreeItem* item, const QMap<int, QString>& namespaceMap, QJsonObject& jsonNodeMap);
    void getVariableAsMustacheArray(
        TreeItem* item,
        std::unordered_map<std::string, mustache::data>& nodeMap,
        QJsonObject& jsonObj);
    void getMethodAsMustacheArray(
        TreeItem* item,
        std::unordered_map<std::string, mustache::data>& nodeMap,
        QJsonObject& jsonObj);
    void getArgumentsAsMustacheArray(
        int index,
        std::shared_ptr<UAVariable> var,
        std::unordered_map<std::string, mustache::data>& argMap,
        QJsonObject& jsonArgMap);
    std::string loadTemplateFile(const QString& filePath);
    void saveToFile(const QString& filePath, const std::string& data);
    void downloadFile(const QString& fileName, const QByteArray& fileContent);

    QString getUserCodeSegment(
        const QString& fileName, const QString& startMarker, const QString& endMarker);

    RootNodeFilterModel* m_rootNodeFilterModel = nullptr;
};

#endif // DEVICEDRIVERCORE_H
