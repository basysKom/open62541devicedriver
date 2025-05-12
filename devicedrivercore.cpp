// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

#include "devicedrivercore.h"
#ifdef ENABLE_MODEL_TESTER
#include <QAbstractItemModelTester>
#endif
#include "Util/Utils.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QTimer>
#include <QVariant>

DeviceDriverCore::DeviceDriverCore()
{
    m_deviceTypesModel = new TreeModel();
    m_selectionModel = new TreeModel();
    m_childItemFilterModel = new ChildItemFilterModel();
    m_rootNodeFilterModel = new RootNodeFilterModel();

    m_childItemFilterModel->setSourceModel(m_selectionModel);
    m_rootNodeFilterModel->setSourceModel(m_selectionModel);

    m_projectName = QStringLiteral("Untitled");

#ifdef ENABLE_MODEL_TESTER
    QAbstractItemModelTester* deviceTypesModeTester = new QAbstractItemModelTester(
        m_deviceTypesModel, QAbstractItemModelTester::FailureReportingMode::Fatal, nullptr);
    QAbstractItemModelTester* selectionModelTester = new QAbstractItemModelTester(
        m_selectionModel, QAbstractItemModelTester::FailureReportingMode::Fatal, nullptr);
    QAbstractItemModelTester* childItemFilterModelTester = new QAbstractItemModelTester(
        m_childItemFilterModel, QAbstractItemModelTester::FailureReportingMode::Fatal, nullptr);
    QAbstractItemModelTester* rootNodeFilterModelTester = new QAbstractItemModelTester(
        m_rootNodeFilterModel, QAbstractItemModelTester::FailureReportingMode::Fatal, nullptr);
#endif
}

DeviceDriverCore::~DeviceDriverCore()
{
    delete m_deviceTypesModel;
    delete m_selectionModel;
    delete m_childItemFilterModel;
    delete m_rootNodeFilterModel;
}

void DeviceDriverCore::parseNodeSets(const QString& nodeSetDir)
{
    // Parse the NodeSet2.xml files and store them in a hash map
    qDebug() << "Parsing NodeSet XML files..." << nodeSetDir;
    QStringList requiredModels = findRequiredModels(getNodeSetXmlFile(nodeSetDir));
    QStringList requiredFiles = findRequiredFiles(requiredModels);

    if (requiredModels.size() != requiredFiles.size()) {
        qWarning() << "Required models and files do not match!";
        return;
    }

    for (int i = 0; i < requiredModels.size(); i++) {
        std::shared_ptr<UANodeSet> nodeSet = std::make_shared<UANodeSet>();

        m_parser.parse(requiredFiles.at(i), nodeSet.get());
        m_nodeSets.insert(requiredModels.at(i), nodeSet);
    }

    resolveParentNode();
    resolveReferences();
    resolveDataTypes();
    resolveMethods();
}

TreeModel* DeviceDriverCore::deviceTypesModel() const
{
    return m_deviceTypesModel;
}

void DeviceDriverCore::selectNodeSetXML(const QString& nodeSetDir)
{
    QTimer::singleShot(10, this, [this, nodeSetDir]() {
        qDebug() << "Selected NodeSet XML: " << nodeSetDir;
        m_selectedModelUri.clear();
        m_nodeSets.clear();
        m_selectionModel->resetModel();
        m_currentNodeSetDir = nodeSetDir;
        parseNodeSets(nodeSetDir);

        //clear the current namespaceMap
        Utils::instance()->setcurrentNameSpaceMaps(QMap<QString, QMap<int, QString>>());
        for (const auto& nodeSet : std::as_const(m_nodeSets)) {
            Utils::instance()->addNameSpaceMap(nodeSet->getNameSpaceUri(), nodeSet->namespaceMap());
        }

        if (!m_selectedModelUri.isEmpty())
            m_deviceTypesModel->setupModelData(m_nodeSets[m_selectedModelUri]);

        emit setupFinished();
    });
}

void DeviceDriverCore::addRootNodeToSelectionModel(
    const QString& namespaceString, const QString& nodeId)
{
    qDebug() << "adding: " << namespaceString << " " << nodeId;
    std::shared_ptr<UANode> node = findNodeById(namespaceString, nodeId);
    if (node) {
        std::shared_ptr<UANode> nodeCopy = node->clone();
        m_selectionModel->addRootNodeToSelection(nodeCopy);
    }
}

void DeviceDriverCore::removeRootNodeFromSelection(const int index)
{
    m_selectionModel->removeRootNodeFromSelection(index);
}

void DeviceDriverCore::getVariableAsMustacheArray(
    TreeItem* item, std::unordered_map<std::string, mustache::data>& nodeMap, QJsonObject& jsonObj)
{
    QString nameStr = Utils::instance()->sanitizeName(item->nodeVariableName());

    QString nsName = Utils::instance()->extractNameFromNamespaceString(
        item->dataType().namespaceString());
    QString typesArrayName = QStringLiteral("UA_TYPES")
                             + (nsName.size() > 0 ? QStringLiteral("_") + nsName.toUpper()
                                                  : QStringLiteral(""));

    QString typesArrayIndexAlias
        = QStringLiteral("UA_TYPES")
          + (nsName.size() > 0 ? QStringLiteral("_") + nsName.toUpper() + QStringLiteral("_")
                               : QStringLiteral("_"))
          + Utils::instance()
                ->removeNamespaceIndexFromName(item->dataType().definitionName())
                .toUpper();

    std::string dataTypeValue = Utils::instance()
                                    ->removeNamespaceIndexFromName(item->dataType().definitionName())
                                    .toStdString();
    nodeMap["dataType"] = dataTypeValue.empty() ? mustache::data(false) : dataTypeValue;
    jsonObj[QStringLiteral("dataType")] = QString::fromStdString(dataTypeValue);

    std::string dataTypeVarNameValue
        = Utils::instance()
              ->lowerFirstChar(Utils::instance()->removeNamespaceIndexFromName(item->displayName()))
              .toStdString();
    nodeMap["dataTypeVariableName"] = dataTypeVarNameValue.empty() ? mustache::data(false)
                                                                   : dataTypeVarNameValue;
    jsonObj[QStringLiteral("dataTypeVariableName")] = QString::fromStdString(dataTypeVarNameValue);

    std::string typesArrayNameValue = typesArrayName.toStdString();
    nodeMap["typesArrayName"] = typesArrayNameValue.empty() ? mustache::data(false)
                                                            : typesArrayNameValue;
    jsonObj[QStringLiteral("typesArrayName")] = QString::fromStdString(typesArrayNameValue);

    std::string typesArrayIndexAliasValue = typesArrayIndexAlias.toStdString();
    nodeMap["typesArrayIndexAlias"] = typesArrayIndexAliasValue.empty() ? mustache::data(false)
                                                                        : typesArrayIndexAliasValue;
    jsonObj[QStringLiteral("typesArrayIndexAlias")] = QString::fromStdString(
        typesArrayIndexAliasValue);

    QString filePath = m_existingFilePath;
    ;
    std::string readUserCodeValue = getUserCodeSegment(
                                        filePath,
                                        QStringLiteral("//BEGIN user code read ") + nameStr,
                                        QStringLiteral("//END user code read ") + nameStr)
                                        .toStdString();
    nodeMap["readUserCode"] = readUserCodeValue.empty() ? mustache::data(false) : readUserCodeValue;
    jsonObj[QStringLiteral("readUserCode")] = QString::fromStdString(readUserCodeValue);

    std::string writeUserCodeValue = getUserCodeSegment(
                                         filePath,
                                         QStringLiteral("//BEGIN user code write ") + nameStr,
                                         QStringLiteral("//END user code write ") + nameStr)
                                         .toStdString();
    nodeMap["writeUserCode"] = writeUserCodeValue.empty() ? mustache::data(false)
                                                          : writeUserCodeValue;
    jsonObj[QStringLiteral("writeUserCode")] = QString::fromStdString(writeUserCodeValue);

    std::vector<mustache::data> definitionFieldsArray;
    QJsonArray jsonDefinitionFieldsArray;
    bool hasValue = false;

    QVariantList fields = item->definitionFields();
    for (int i = 0; i < fields.size(); ++i) {
        std::unordered_map<std::string, mustache::data> definitionFields;
        QJsonObject jsonDefinitionFields;

        std::string fieldNameValue
            = Utils::instance()
                  ->lowerFirstChar(Utils::instance()->removeNamespaceIndexFromName(
                      fields.at(i).toMap()[QStringLiteral("key")].toString()))
                  .toStdString();
        definitionFields["fieldName"] = fieldNameValue.empty() ? mustache::data(false)
                                                               : fieldNameValue;
        jsonDefinitionFields[QStringLiteral("fieldName")] = QString::fromStdString(fieldNameValue);

        std::string fieldTypeValue
            = fields.at(i).toMap()[QStringLiteral("value")].toString().toStdString();
        definitionFields["fieldType"] = fieldTypeValue.empty() ? mustache::data(false)
                                                               : fieldTypeValue;
        jsonDefinitionFields[QStringLiteral("fieldType")] = QString::fromStdString(fieldTypeValue);

        std::string fieldValueVal = item->getValue(
                                            fields.at(i).toMap()[QStringLiteral("key")].toString())
                                        .toString()
                                        .toStdString();
        definitionFields["fieldValue"] = fieldValueVal.empty() ? mustache::data(false)
                                                               : fieldValueVal;
        jsonDefinitionFields[QStringLiteral("fieldValue")] = QString::fromStdString(fieldValueVal);

        QString dataType = fields.at(i).toMap()[QStringLiteral("value")].toString();
        bool isString
            = (dataType == QStringLiteral("String") || dataType == QStringLiteral("Locale"));
        definitionFields["isString"] = isString;
        jsonDefinitionFields[QStringLiteral("isString")] = isString;

        hasValue = item->getValue(fields.at(i).toMap()[QStringLiteral("key")].toString()).isValid();

        definitionFieldsArray.emplace_back(definitionFields);
        jsonDefinitionFieldsArray.append(jsonDefinitionFields);
    }

    nodeMap["singleFieldValueFlag"] = (fields.size() == 1);
    jsonObj[QStringLiteral("singleFieldValueFlag")] = (fields.size() == 1);

    nodeMap["fieldsHaveValuesFlag"] = hasValue;
    jsonObj[QStringLiteral("fieldsHaveValuesFlag")] = hasValue;

    nodeMap["definitionFields"] = definitionFieldsArray;
    jsonObj[QStringLiteral("definitionFields")] = jsonDefinitionFieldsArray;
}

void DeviceDriverCore::getMethodAsMustacheArray(
    TreeItem* item, std::unordered_map<std::string, mustache::data>& nodeMap, QJsonObject& jsonObj)
{
    std::shared_ptr<UAMethod> methodNode = std::dynamic_pointer_cast<UAMethod>(item->getNode());

    std::vector<mustache::data> outputArgumentsArray;
    std::vector<mustache::data> inputArgumentsArray;
    QJsonArray jsonOutputArgumentsArray;
    QJsonArray jsonInputArgumentsArray;

    std::shared_ptr<UAVariable> var = methodNode->inputArgument();
    int inputArgSize = var ? static_cast<int>(var->arguments().size()) : 0;

    nodeMap["inputArgumentArrayDimensions"] = QString::number(inputArgSize).toStdString();
    jsonObj[QStringLiteral("inputArgumentArrayDimensions")] = inputArgSize;

    for (int i = 0; i < inputArgSize; ++i) {
        std::unordered_map<std::string, mustache::data> inputArgs;
        QJsonObject jsonInputArgs;

        getArgumentsAsMustacheArray(i, var, inputArgs, jsonInputArgs);
        inputArgs["argumentIndex"] = QString::number(i).toStdString();
        jsonInputArgs[QStringLiteral("argumentIndex")] = i;

        inputArgumentsArray.emplace_back(inputArgs);
        jsonInputArgumentsArray.append(jsonInputArgs);
    }

    var = methodNode->outputArgument();
    int outputArgSize = var ? static_cast<int>(var->arguments().size()) : 0;

    nodeMap["outputArgumentArrayDimensions"] = QString::number(outputArgSize).toStdString();
    jsonObj[QStringLiteral("outputArgumentArrayDimensions")] = outputArgSize;

    for (int i = 0; i < outputArgSize; ++i) {
        std::unordered_map<std::string, mustache::data> outputArgs;
        QJsonObject jsonOutputArgs;

        getArgumentsAsMustacheArray(i, var, outputArgs, jsonOutputArgs);
        outputArgs["argumentIndex"] = QString::number(i).toStdString();
        jsonOutputArgs[QStringLiteral("argumentIndex")] = i;

        outputArgumentsArray.emplace_back(outputArgs);
        jsonOutputArgumentsArray.append(jsonOutputArgs);
    }

    nodeMap["inputArguments"] = inputArgumentsArray;
    jsonObj[QStringLiteral("inputArguments")] = jsonInputArgumentsArray;

    nodeMap["outputArguments"] = outputArgumentsArray;
    jsonObj[QStringLiteral("outputArguments")] = jsonOutputArgumentsArray;

    QString nameStr = Utils::instance()->sanitizeName(item->nodeVariableName());

    std::string userCodeValue = getUserCodeSegment(
                                    m_existingFilePath,
                                    QStringLiteral("//BEGIN user code ") + nameStr,
                                    QStringLiteral("//END user code ") + nameStr)
                                    .toStdString();
    nodeMap["userCode"] = userCodeValue.empty() ? mustache::data(false) : userCodeValue;
    jsonObj[QStringLiteral("userCode")] = QString::fromStdString(userCodeValue);
}

void DeviceDriverCore::getArgumentsAsMustacheArray(
    int index,
    std::shared_ptr<UAVariable> var,
    std::unordered_map<std::string, mustache::data>& argMap,
    QJsonObject& jsonArgMap)
{
    Argument arg = var->arguments().at(index);
    std::string argumentNameValue = Utils::instance()->lowerFirstChar(arg.name).toStdString();
    argMap["argumentName"] = argumentNameValue.empty() ? mustache::data(false) : argumentNameValue;
    jsonArgMap[QStringLiteral("argumentName")] = QString::fromStdString(argumentNameValue);

    int nameSpaceIndex = Utils::instance()->extractNamespaceIndex(arg.dataTypeIdentifier);
    QString namespaceString
        = Utils::instance()->getNamespaceByIndex(var->namespaceString(), nameSpaceIndex);

    std::shared_ptr<UADataType> dataType = std::dynamic_pointer_cast<UADataType>(
        findNodeById(namespaceString, arg.dataTypeIdentifier));

    std::string argumentDataTypeValue
        = Utils::instance()->removeNamespaceIndexFromName(dataType->browseName()).toStdString();
    argMap["argumentDataType"] = argumentDataTypeValue.empty() ? mustache::data(false)
                                                               : argumentDataTypeValue;
    jsonArgMap[QStringLiteral("argumentDataType")] = QString::fromStdString(argumentDataTypeValue);

    QString nsName = Utils::instance()->extractNameFromNamespaceString(dataType->namespaceString());
    QString typesArrayName = QStringLiteral("UA_TYPES")
                             + (nsName.isEmpty() ? QStringLiteral("")
                                                 : QStringLiteral("_") + nsName.toUpper());
    QString typesArrayIndexAlias
        = QStringLiteral("UA_TYPES")
          + (nsName.isEmpty() ? QStringLiteral("_")
                              : QStringLiteral("_") + nsName.toUpper() + QStringLiteral("_"))
          + Utils::instance()->removeNamespaceIndexFromName(dataType->browseName()).toUpper();

    std::string typesArrayNameValue = typesArrayName.toStdString();
    argMap["typesArrayName"] = typesArrayNameValue.empty() ? mustache::data(false)
                                                           : typesArrayNameValue;
    jsonArgMap[QStringLiteral("typesArrayName")] = QString::fromStdString(typesArrayNameValue);

    std::string typesArrayIndexAliasValue = typesArrayIndexAlias.toStdString();
    argMap["typesArrayIndexAlias"] = typesArrayIndexAliasValue.empty() ? mustache::data(false)
                                                                       : typesArrayIndexAliasValue;
    jsonArgMap[QStringLiteral("typesArrayIndexAlias")] = QString::fromStdString(
        typesArrayIndexAliasValue);

    bool isEnum = dataType->isEnum();
    argMap["isEnum"] = isEnum;
    jsonArgMap[QStringLiteral("isEnum")] = isEnum;

    if (isEnum) {
        std::vector<mustache::data> enumValues;
        QJsonArray jsonEnumValues;

        for (const auto& [key, value] : dataType->definitionFields().asKeyValueRange()) {
            std::string enumValue = (key + QStringLiteral(" = ") + value).toStdString();
            enumValues.emplace_back(enumValue.empty() ? mustache::data(false) : enumValue);
            jsonEnumValues.append(QString::fromStdString(enumValue));
        }

        argMap["argumentEnumValues"] = enumValues;
        jsonArgMap[QStringLiteral("argumentEnumValues")] = jsonEnumValues;
    } else {
        std::vector<mustache::data> dataTypeFields;
        QJsonArray jsonDataTypeFields;

        for (const auto& [key, value] : dataType->definitionFields().asKeyValueRange()) {
            std::unordered_map<std::string, mustache::data> field;
            QJsonObject jsonField;

            std::string fieldNameValue = Utils::instance()->lowerFirstChar(key).toStdString();
            field["fieldName"] = fieldNameValue.empty() ? mustache::data(false) : fieldNameValue;
            jsonField[QStringLiteral("fieldName")] = QString::fromStdString(fieldNameValue);

            std::string fieldTypeValue = value.toStdString();
            field["fieldType"] = fieldTypeValue.empty() ? mustache::data(false) : fieldTypeValue;
            jsonField[QStringLiteral("fieldType")] = QString::fromStdString(fieldTypeValue);

            dataTypeFields.emplace_back(field);
            jsonDataTypeFields.append(jsonField);
        }

        argMap["dataTypeFields"] = dataTypeFields;
        jsonArgMap[QStringLiteral("dataTypeFields")] = jsonDataTypeFields;
    }
}

std::unordered_map<std::string, mustache::data> DeviceDriverCore::createNodeMap(
    int index, TreeItem* item, const QMap<int, QString>& namespaceMap, QJsonObject& jsonNodeMap)
{
    std::unordered_map<std::string, mustache::data> nodeMap;

    QString nameStr = Utils::instance()->sanitizeName(item->nodeVariableName());

    std::string nodeIndexValue = std::to_string(index);
    nodeMap["nodeIndex"] = nodeIndexValue.empty() ? mustache::data(false) : nodeIndexValue;
    jsonNodeMap[QStringLiteral("nodeIndex")] = QString::fromStdString(nodeIndexValue);

    std::string nameValue = nameStr.toStdString();
    nodeMap["name"] = nameValue.empty() ? mustache::data(false) : nameValue;
    jsonNodeMap[QStringLiteral("name")] = QString::fromStdString(nameValue);

    std::string nodeIdValue = item->nodeId().toStdString();
    nodeMap["nodeId"] = nodeIdValue.empty() ? mustache::data(false) : nodeIdValue;
    jsonNodeMap[QStringLiteral("nodeId")] = QString::fromStdString(nodeIdValue);

    std::string identifierValue = Utils::instance()->extractIdentifier(item->nodeId()).toStdString();
    nodeMap["identifier"] = identifierValue.empty() ? mustache::data(false) : identifierValue;
    jsonNodeMap[QStringLiteral("identifier")] = QString::fromStdString(identifierValue);

    std::string namespaceIndexValue = std::to_string(namespaceMap.key(item->namespaceString()));
    nodeMap["namespaceIndex"] = namespaceIndexValue.empty() ? mustache::data(false)
                                                            : namespaceIndexValue;
    jsonNodeMap[QStringLiteral("namespaceIndex")] = QString::fromStdString(namespaceIndexValue);

    std::string browseNameValue
        = Utils::instance()->removeNamespaceIndexFromName(item->browseName()).toStdString();
    nodeMap["browseName"] = browseNameValue.empty() ? mustache::data(false) : browseNameValue;
    jsonNodeMap[QStringLiteral("browseName")] = QString::fromStdString(browseNameValue);

    std::string baseBrowseNameValue
        = Utils::instance()->removeNamespaceIndexFromName(item->baseBrowseName()).toStdString();
    nodeMap["baseBrowseName"] = baseBrowseNameValue.empty() ? mustache::data(false)
                                                            : baseBrowseNameValue;
    jsonNodeMap[QStringLiteral("baseBrowseName")] = QString::fromStdString(baseBrowseNameValue);

    std::string displayNameValue = item->displayName().toStdString();
    nodeMap["displayName"] = displayNameValue.empty() ? mustache::data(false) : displayNameValue;
    jsonNodeMap[QStringLiteral("displayName")] = QString::fromStdString(displayNameValue);

    std::string descriptionValue = item->description().toStdString();
    nodeMap["description"] = descriptionValue.empty() ? mustache::data(false) : descriptionValue;
    jsonNodeMap[QStringLiteral("description")] = QString::fromStdString(descriptionValue);

    return nodeMap;
}

std::pair<std::unordered_map<std::string, mustache::data>, QJsonDocument>
DeviceDriverCore::getMustacheData()
{
    std::unordered_map<std::string, mustache::data> data;
    QJsonObject jsonData;

    std::vector<mustache::data> nameSpacesArray;
    std::vector<mustache::data> nodeSetsArray;
    std::vector<mustache::data> rootNodesArray;
    std::vector<mustache::data> objectNodesArray;
    std::vector<mustache::data> variableNodesArray;
    std::vector<mustache::data> methodNodesArray;

    QJsonArray jsonNameSpacesArray;
    QJsonArray jsonNodeSetsArray;
    QJsonArray jsonRootNodesArray;
    QJsonArray jsonObjectNodesArray;
    QJsonArray jsonVariableNodesArray;
    QJsonArray jsonMethodNodesArray;

    // Namespace mapping
    QMap<int, QString> namespaceMap;

    std::string projectName = m_projectName.toStdString();
    data["projectName"] = projectName.empty() ? mustache::data(false) : projectName;
    std::string nsCountValue = std::to_string(m_nodeSets.size());
    data["nsCount"] = nsCountValue.empty() ? mustache::data(false) : nsCountValue;
    jsonData[QStringLiteral("nsCount")] = QString::fromStdString(nsCountValue);

    int i = 0;
    for (auto it = m_nodeSets.begin(); it != m_nodeSets.end(); ++it) {
        std::shared_ptr<UANodeSet> nodeSet = it.value();
        std::unordered_map<std::string, mustache::data> ns;
        QJsonObject jsonNs;

        std::string uriValue = nodeSet->getNameSpaceUri().toStdString();
        ns["uri"] = uriValue.empty() ? mustache::data(false) : uriValue;
        jsonNs[QStringLiteral("uri")] = QString::fromStdString(uriValue);

        qDebug() << "Included Namespace: " << nodeSet->getNameSpaceUri();

        std::string indexValue = std::to_string(i);
        ns["index"] = indexValue.empty() ? mustache::data(false) : indexValue;
        jsonNs[QStringLiteral("index")] = QString::fromStdString(indexValue);

        namespaceMap.insert(i, nodeSet->getNameSpaceUri());
        nameSpacesArray.emplace_back(ns);
        jsonNameSpacesArray.append(jsonNs);
        ++i;

        QString nodeSetName = nodeSet->getNodeSetName();
        if (!nodeSetName.isEmpty()) {
            std::unordered_map<std::string, mustache::data> nodeSetMap;
            QJsonObject jsonNodeSetMap;

            std::string nameValue = nodeSetName.toStdString();
            nodeSetMap["name"] = nameValue.empty() ? mustache::data(false) : nameValue;
            jsonNodeSetMap[QStringLiteral("name")] = QString::fromStdString(nameValue);

            std::string hasCustomTypesValue = nodeSet->getHasCustomTypes() ? "true" : "false";
            nodeSetMap["hasCustomTypes"] = hasCustomTypesValue.empty() ? mustache::data(false)
                                                                       : hasCustomTypesValue;
            jsonNodeSetMap[QStringLiteral("hasCustomTypes")] = nodeSet->getHasCustomTypes();

            nodeSetsArray.insert(nodeSetsArray.begin(), nodeSetMap); // Reverse order
            jsonNodeSetsArray.prepend(jsonNodeSetMap);
        }
    }

    data["nameSpaces"] = nameSpacesArray;
    jsonData[QStringLiteral("nameSpaces")] = jsonNameSpacesArray;

    data["nodeSets"] = nodeSetsArray;
    jsonData[QStringLiteral("nodeSets")] = jsonNodeSetsArray;

    // Get selected nodes
    QList<TreeItem*> allNodes = getSelectedItems();
    std::string nodeCountValue = std::to_string(allNodes.size());
    data["nodeCount"] = nodeCountValue.empty() ? mustache::data(false) : nodeCountValue;
    jsonData[QStringLiteral("nodeCount")] = QString::fromStdString(nodeCountValue);

    qDebug() << allNodes.size() << " Nodes selected.";

    for (int i = 0; i < allNodes.size(); ++i) {
        TreeItem* item = allNodes.at(i);
        QJsonObject jsonNodeMap;
        std::unordered_map<std::string, mustache::data> nodeMap
            = createNodeMap(i, item, namespaceMap, jsonNodeMap);

        if (item->isRootNode()) {
            std::string parentNodeIdValue = "UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER)";
            nodeMap["parentNodeId"] = parentNodeIdValue.empty() ? mustache::data(false)
                                                                : parentNodeIdValue;
            jsonNodeMap[QStringLiteral("parentNodeId")] = QString::fromStdString(parentNodeIdValue);

            std::string referenceTypeNodeIdValue = "UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES)";
            nodeMap["referenceTypeNodeId"] = referenceTypeNodeIdValue.empty()
                                                 ? mustache::data(false)
                                                 : referenceTypeNodeIdValue;
            jsonNodeMap[QStringLiteral("referenceTypeNodeId")] = QString::fromStdString(
                referenceTypeNodeIdValue);

            rootNodesArray.emplace_back(nodeMap);
            jsonRootNodesArray.append(jsonNodeMap);
        } else {
            std::string parentNodeIdStr = Utils::instance()
                                              ->sanitizeName(
                                                  item->parentNode().lock()->nodeVariableName()
                                                  + QStringLiteral("_NodeId"))
                                              .toStdString();
            nodeMap["parentNodeId"] = parentNodeIdStr.empty() ? mustache::data(false)
                                                              : parentNodeIdStr;
            jsonNodeMap[QStringLiteral("parentNodeId")] = QString::fromStdString(parentNodeIdStr);

            std::string referenceTypeNodeIdValue = "UA_NODEID_NUMERIC(0, "
                                                   + Utils::instance()
                                                         ->extractIdentifier(
                                                             parentReferenceNodeId(item->getNode()))
                                                         .toStdString()
                                                   + ")";
            nodeMap["referenceTypeNodeId"] = referenceTypeNodeIdValue.empty()
                                                 ? mustache::data(false)
                                                 : referenceTypeNodeIdValue;
            jsonNodeMap[QStringLiteral("referenceTypeNodeId")] = QString::fromStdString(
                referenceTypeNodeIdValue);
        }

        if (item->typeName() == XmlTags::UAObject) {
            objectNodesArray.emplace_back(nodeMap);
            jsonObjectNodesArray.append(jsonNodeMap);
        } else if (item->typeName() == XmlTags::UAVariable) {
            getVariableAsMustacheArray(item, nodeMap, jsonNodeMap);
            variableNodesArray.emplace_back(nodeMap);
            jsonVariableNodesArray.append(jsonNodeMap);
        } else if (item->typeName() == XmlTags::UAMethod) {
            getMethodAsMustacheArray(item, nodeMap, jsonNodeMap);
            methodNodesArray.emplace_back(nodeMap);
            jsonMethodNodesArray.append(jsonNodeMap);
        }
    }

    if (!methodNodesArray.empty()) {
        std::string methodCountValue = std::to_string(methodNodesArray.size());
        data["methodCount"] = methodCountValue.empty() ? mustache::data(false) : methodCountValue;
        jsonData[QStringLiteral("methodCount")] = QString::fromStdString(methodCountValue);
    }

    data["rootNodes"] = rootNodesArray;
    jsonData[QStringLiteral("rootNodes")] = jsonRootNodesArray;

    data["objectNodes"] = objectNodesArray;
    jsonData[QStringLiteral("objectNodes")] = jsonObjectNodesArray;

    data["variableNodes"] = variableNodesArray;
    jsonData[QStringLiteral("variableNodes")] = jsonVariableNodesArray;

    data["methodNodes"] = methodNodesArray;
    jsonData[QStringLiteral("methodNodes")] = jsonMethodNodesArray;

    QJsonDocument jsonDoc(jsonData);
    return std::make_pair(data, jsonDoc);
}

std::pair<mustache::data, QJsonDocument> DeviceDriverCore::getCMakeMustacheData()
{
    mustache::data data;
    QJsonObject jsonData;

    // Project name is set by the user at start of a new project
    std::string projectName = m_projectName.toStdString();
    data["projectName"] = projectName.empty() ? mustache::data(false) : projectName;
    jsonData[QStringLiteral("projectName")] = QString::fromStdString(projectName);

    data["executableName"] = projectName.empty() ? mustache::data(false) : projectName;
    jsonData[QStringLiteral("executableName")] = QString::fromStdString(projectName);

    QStringList requiredModels = findRequiredModels(getNodeSetXmlFile(m_currentNodeSetDir));

    std::vector<mustache::data> nodeSetsArray;
    QJsonArray jsonNodeSetsArray;

    for (auto it = m_nodeSets.begin(); it != m_nodeSets.end(); ++it) {
        std::shared_ptr<UANodeSet> nodeSet = it.value();
        QString nodeSetName = nodeSet->getNodeSetName();

        if (!nodeSetName.isEmpty()) {
            std::unordered_map<std::string, mustache::data> nodeSetMap;
            QJsonObject jsonNodeSetMap;

            std::string nameValue = nodeSetName.toStdString();
            nodeSetMap["name"] = nameValue.empty() ? mustache::data(false) : nameValue;
            jsonNodeSetMap[QStringLiteral("name")] = QString::fromStdString(nameValue);

            std::string nameUpperValue = nodeSetName.toUpper().toStdString();
            nodeSetMap["nameUpper"] = nameUpperValue.empty() ? mustache::data(false)
                                                             : nameUpperValue;
            jsonNodeSetMap[QStringLiteral("nameUpper")] = QString::fromStdString(nameUpperValue);

            QStringList requiredFiles;
            for (const auto& model : requiredModels) {
                if (model == nodeSet->getNameSpaceUri()) {
                    requiredFiles = findRequiredFiles({model}, false);
                } else {
                    std::string dependsValue
                        = Utils::instance()->extractNameFromNamespaceString(model).toStdString();
                    nodeSetMap["depends"] = dependsValue.empty() ? mustache::data(false)
                                                                 : dependsValue;
                    jsonNodeSetMap[QStringLiteral("depends")] = QString::fromStdString(dependsValue);
                }
            }

            for (auto& file : requiredFiles) {
                file = file.section(QLatin1Char('/'), -1);
                if (file.contains(QStringLiteral("NodeSet2.xml"))) {
                    std::string fileNsValue = file.toStdString();
                    nodeSetMap["file_ns"] = fileNsValue.empty() ? mustache::data(false)
                                                                : fileNsValue;
                    jsonNodeSetMap[QStringLiteral("file_ns")] = QString::fromStdString(fileNsValue);
                }
                if (file.contains(QStringLiteral("NodeIds.csv"))) {
                    std::string fileCsvValue = file.toStdString();
                    nodeSetMap["file_csv"] = fileCsvValue.empty() ? mustache::data(false)
                                                                  : fileCsvValue;
                    jsonNodeSetMap[QStringLiteral("file_csv")] = QString::fromStdString(
                        fileCsvValue);
                }
                if (file.contains(QStringLiteral("Types.bsd"))) {
                    std::string fileBsdValue = file.toStdString();
                    nodeSetMap["file_bsd"] = fileBsdValue.empty() ? mustache::data(false)
                                                                  : fileBsdValue;
                    jsonNodeSetMap[QStringLiteral("file_bsd")] = QString::fromStdString(
                        fileBsdValue);
                }
            }

            QString nodsetDirPrefix
                = nodeSet->getNameSpaceUri().section(QChar::fromLatin1('/'), -2, -2);
            std::string nodsetDirPrefixValue = nodsetDirPrefix.toStdString();
            nodeSetMap["nodsetDirPrefix"] = nodsetDirPrefixValue.empty() ? mustache::data(false)
                                                                         : nodsetDirPrefixValue;
            jsonNodeSetMap[QStringLiteral("nodsetDirPrefix")] = QString::fromStdString(
                nodsetDirPrefixValue);

            std::string hasCustomTypesValue = nodeSet->getHasCustomTypes() ? "true" : "false";
            nodeSetMap["hasCustomTypes"] = hasCustomTypesValue.empty() ? mustache::data(false)
                                                                       : hasCustomTypesValue;
            jsonNodeSetMap[QStringLiteral("hasCustomTypes")] = nodeSet->getHasCustomTypes();

            nodeSetsArray.insert(nodeSetsArray.begin(), nodeSetMap); // Reverse order
            jsonNodeSetsArray.prepend(jsonNodeSetMap);
        }
    }

    data["nodeSets"] = nodeSetsArray;
    jsonData[QStringLiteral("nodeSets")] = jsonNodeSetsArray;

    QJsonDocument jsonDoc(jsonData);
    return std::make_pair(data, jsonDoc);
}

void DeviceDriverCore::printMustacheData(const QJsonDocument& jsonDoc)
{
    QByteArray formattedData = jsonDoc.toJson(QJsonDocument::Indented);
    qDebug() << "Mustache Data as JSON:";
    qDebug().noquote() << formattedData;
}

QString DeviceDriverCore::readMeMustacheTemplatePath() const
{
    return m_readMeMustacheTemplatePath;
}

void DeviceDriverCore::setReadMeMustacheTemplatePath(const QString& newReadMeMustacheTemplatePath)
{
    m_readMeMustacheTemplatePath = newReadMeMustacheTemplatePath;
}

std::string DeviceDriverCore::loadTemplateFile(const QString& filePath)
{
    std::ifstream fileStream(filePath.toStdString());
    if (!fileStream) {
        qWarning() << "Cannot open file for reading:" << filePath;
        return "";
    }

    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    return buffer.str();
}

void DeviceDriverCore::saveToFile(const QString& filePath, const std::string& data)
{
    QString path = filePath;
    if (filePath.startsWith(QStringLiteral("file://")) || filePath.contains(QStringLiteral("://"))) {
        path = QUrl(filePath).toLocalFile();
    }

    QFileInfo fileInfo(path);
    QDir dir = fileInfo.absoluteDir();

    if (!dir.exists()) {
        qDebug() << "Directory doesn't exist, creating:" << dir.absolutePath();
        if (!dir.mkpath(QStringLiteral("."))) {
            qWarning() << "Could not create directory:" << dir.absolutePath();
            return;
        }
    }

    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data.c_str());
        file.close();
        qDebug() << "File saved successfully to:" << path;
    } else {
        qWarning() << "Could not open file for writing:" << path << "Error:" << file.errorString();
    }
}

QString DeviceDriverCore::getUserCodeSegment(
    const QString& fileName, const QString& startMarker, const QString& endMarker)
{
    QString path = QUrl(fileName).toLocalFile();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file:" << path << file.errorString();
        return QString();
    }

    QTextStream in(&file);
    bool capture = false;
    QString userCode;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line == startMarker) {
            capture = true;
        } else if (line == endMarker) {
            break;
        } else if (capture) {
            userCode += line + QStringLiteral("\n");
        }
    }

    return userCode.trimmed();
}

void DeviceDriverCore::generateCode(bool includeCmake, bool includeJson)
{
    qDebug() << "Generating Code...";

    auto codeTemplate = loadTemplateFile(m_mustacheTemplatePath);
    auto cmakeTemplate = loadTemplateFile(m_cmakeMustacheTemplatePath);
    auto readMeTemplate = loadTemplateFile(m_readMeMustacheTemplatePath);

    auto codeContext = getMustacheData();
    auto cmakeContext = getCMakeMustacheData();
    auto readMeContext = getMustacheData();

    mustache::mustache codeTmpl(codeTemplate);
    mustache::mustache cmakeTmpl(cmakeTemplate);
    mustache::mustache readMeTmpl(readMeTemplate);

    // printMustacheData(codeContext.second);

    QString codeFilename = m_projectName + QStringLiteral(".c");
    QString cmakeFilename = QStringLiteral("CMakeLists.txt");
    QString jsonFileName = m_projectName + QStringLiteral(".json");
    QString readMeFilename = QStringLiteral("README.md");

#ifndef WASM_BUILD
    if (includeCmake)
        saveToFile(
            m_outputFilePath + QStringLiteral("/") + cmakeFilename,
            cmakeTmpl.render(cmakeContext.first));
    if (includeJson)
        saveToJson(m_outputFilePath + QStringLiteral("/") + jsonFileName, codeContext.second);

    saveToFile(
        m_outputFilePath + QStringLiteral("/") + codeFilename, codeTmpl.render(codeContext.first));

    saveToFile(
        m_outputFilePath + QStringLiteral("/") + readMeFilename,
        readMeTmpl.render(readMeContext.first));

    emit generateCodeFinished();

    return;
#endif

    QByteArray codeFileContent = QByteArray::fromStdString(codeTmpl.render(codeContext.first));
    QByteArray cmakeFileContent = QByteArray::fromStdString(cmakeTmpl.render(cmakeContext.first));
    QByteArray readMeFileContent = QByteArray::fromStdString(readMeTmpl.render(readMeContext.first));

    downloadFile(codeFilename, codeFileContent);
    if (includeCmake)
        downloadFile(cmakeFilename, cmakeFileContent);
    if (includeJson)
        downloadFile(jsonFileName, codeContext.second.toJson(QJsonDocument::Indented));

    downloadFile(readMeFilename, readMeFileContent);

    emit generateCodeFinished();
}

void DeviceDriverCore::downloadFile(const QString& fileName, const QByteArray& fileContent)
{
#ifdef WASM_BUILD
    QString mimeType = QStringLiteral("text/plain");

    QByteArray base64Content = fileContent.toBase64();
    QString dataUrl = QStringLiteral("data:") + mimeType + QStringLiteral(";base64,")
                      + QString::fromLatin1(base64Content);

    // Create a Blob and trigger download using JavaScript
    emscripten::val document = emscripten::val::global("document");
    emscripten::val a = document.call<emscripten::val>("createElement", std::string("a"));

    a.set("href", dataUrl.toStdString());
    a.set("download", fileName.toStdString());
    a.set("style", "display: none");

    document["body"].call<void>("appendChild", a);
    a.call<void>("click");
    document["body"].call<void>("removeChild", a);
#endif
}

void DeviceDriverCore::saveToJson(const QString& fileName, const QJsonDocument& content)
{
    qDebug() << "Save to Json...";

    QString path = fileName;
    if (fileName.startsWith(QStringLiteral("file://")) || fileName.contains(QStringLiteral("://"))) {
        path = QUrl(fileName).toLocalFile();
    }

    QFileInfo fileInfo(path);

    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        if (!dir.mkpath(QStringLiteral("."))) {
            qWarning() << "Could not create directory:" << dir.absolutePath();
            return;
        }
    }

    QFile file(path);

    if (file.open(QIODevice::WriteOnly)) {
        file.write(content.toJson());
        file.close();
        qDebug() << "File saved to:" << fileName;
    } else {
        qWarning("Could not open file for writing");
    }
}

std::shared_ptr<UANode> DeviceDriverCore::findNodeById(
    const QString& namespaceString, const QString& nodeId) const
{
    std::shared_ptr<UANodeSet> nodeSet = m_nodeSets.value(namespaceString);
    if (nodeSet) {
        return nodeSet->findNodeById(nodeId);
    }
    qWarning() << "NodeSet not found for namespace:" << namespaceString;
    return nullptr;
}

void DeviceDriverCore::setNodeSetPath(const QString& newNodeSetPath)
{
    m_nodeSetPath = newNodeSetPath;
}

void DeviceDriverCore::resolveParentNode()
{
    for (const std::shared_ptr<UANodeSet>& nodeSet : std::as_const(m_nodeSets)) {
        auto nodes = nodeSet->nodes();
        for (auto node : std::as_const(nodes)) {
            QString parentNodeId = node->parentNodeId();
            if (!parentNodeId.isEmpty()) {
                auto parentNode = nodeSet->findNodeById(parentNodeId);
                if (parentNode) {
                    node->setParentNode(parentNode);
                }
            }
        }
    }
}

void DeviceDriverCore::resolveReferences()
{
    // keep track of visited nodes to avoid cycles
    QSet<std::shared_ptr<UANode>> visitedNodes;

    for (const std::shared_ptr<UANodeSet>& nodeSet : std::as_const(m_nodeSets)) {
        auto nodes = nodeSet->nodes();
        for (const auto& node : std::as_const(nodes)) {
            resolveNodeReferences(node, visitedNodes);
        }
    }
}

void DeviceDriverCore::resolveDataTypes()
{
    for (const std::shared_ptr<UANodeSet>& nodeSet : std::as_const(m_nodeSets)) {
        QList<std::shared_ptr<UANode>> nodes = nodeSet->nodes();
        for (const auto& node : std::as_const(nodes)) {
            if (node->typeName() == XmlTags::UAVariable) {
                std::shared_ptr<UAVariable> variable = std::dynamic_pointer_cast<UAVariable>(node);
                QString dataTypeNode = nodeSet->getNodeIdByAlias(
                    variable->dataType().definitionName());
                if (dataTypeNode != QStringLiteral("")) {
                    for (const std::shared_ptr<UANodeSet>& ns : std::as_const(m_nodeSets)) {
                        auto node = ns->findNodeById(dataTypeNode);
                        if (node && node->typeName() == XmlTags::UADataType) {
                            std::shared_ptr<UADataType> dataType
                                = std::dynamic_pointer_cast<UADataType>(node);
                            if (dataType) {
                                variable->setDataType(*dataType);
                            }
                        }
                    }
                }
            }
        }
    }
}

void DeviceDriverCore::resolveMethods()
{
    for (const std::shared_ptr<UANodeSet>& nodeSet : std::as_const(m_nodeSets)) {
        QList<std::shared_ptr<UANode>> nodes = nodeSet->nodes();
        for (const std::shared_ptr<UANode>& node : std::as_const(nodes)) {
            if (node->typeName() == XmlTags::UAMethod) {
                std::shared_ptr<UAMethod> method = std::dynamic_pointer_cast<UAMethod>(node);
                for (const std::shared_ptr<Reference>& reference : method->references()) {
                    if (reference->node()) {
                        if (reference->node()->browseName() == QStringLiteral("InputArguments")) {
                            method->setInputArgument(
                                std::dynamic_pointer_cast<UAVariable>(reference->node()));
                        } else if (reference->node()->browseName() == QStringLiteral("OutputArguments")) {
                            method->setOutputArgument(
                                std::dynamic_pointer_cast<UAVariable>(reference->node()));
                        }
                    }
                }
            }
        }
    }
}

QString DeviceDriverCore::parentReferenceNodeId(std::shared_ptr<UANode> node)
{
    {
        for (const std::shared_ptr<Reference>& reference : node->parentNode().lock()->references()) {
            if (reference->targetNodeId() == node->nodeId()) {
                QString type = reference->referenceType();
                QString refNodeId = m_nodeSets.value(m_selectedModelUri)->getNodeIdByAlias(type);
                if (!refNodeId.isEmpty()) {
                    return refNodeId;
                }
            }
        }

        qWarning() << "Access to non existing parentReferenceNodeId member from "
                   << node->typeName();
        return QStringLiteral("");
    }
}

void DeviceDriverCore::resolveNodeReferences(
    std::shared_ptr<UANode> node, QSet<std::shared_ptr<UANode>>& visitedNodes)
{
    if (visitedNodes.contains(node)) {
        return;
    }

    visitedNodes.insert(node);

    QList<std::shared_ptr<Reference>> references = node->references();
    for (std::shared_ptr<Reference> reference : std::as_const(references)) {
        std::shared_ptr<UANode> referencedNode
            = findNodeById(reference->namespaceString(), reference->targetNodeId());
        if (referencedNode) {
            reference->setNode(referencedNode);

            // set the optional flag for the node. This includes OptionalPlaceholders
            if (reference->referenceType() == XmlTags::HasModellingRule) {
                if (referencedNode->browseName().contains(XmlTags::Optional)) {
                    node->setIsOptional(true);
                }
            }
            // We want to get the inherited definition fields from the referenced node
            if (reference->referenceType() == XmlTags::HasSubtype) {
                if (std::shared_ptr<UADataType> dataTypeNode
                    = std::dynamic_pointer_cast<UADataType>(node)) {
                    if (const std::shared_ptr<UADataType> referenceDataType
                        = std::dynamic_pointer_cast<UADataType>(reference->node())) {
                        for (const auto [key, value] :
                             referenceDataType->definitionFields().asKeyValueRange()) {
                            dataTypeNode->addDefinitionField(key, value);
                        }
                    }
                }
            }

            // Recursively resolve references for the referenced node
            resolveNodeReferences(referencedNode, visitedNodes);
        }
    }
}

QStringList DeviceDriverCore::findRequiredModels(const QString& fileName)
{
    // finds the requiredModels (NodeSet2.xml files) from the selected Nodeset
    QStringList requiredModels;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file for reading:" << file.errorString();
        return requiredModels;
    }

    QXmlStreamReader xml(&file);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name().toString() == XmlTags::Models) {
                while (!xml.atEnd()) {
                    xml.readNext();
                    if (xml.isStartElement()) {
                        if (xml.name().toString() == XmlTags::RequiredModel) {
                            QXmlStreamAttributes attributes = xml.attributes();
                            if (attributes.hasAttribute(XmlTags::ModelUri)) {
                                QString modelUri = attributes.value(XmlTags::ModelUri).toString();
                                requiredModels.append(modelUri);
                            }
                            // NOTE the Model is the URI of the selected Companion Specification.
                        } else if (xml.name().toString() == XmlTags::Model) {
                            QXmlStreamAttributes attributes = xml.attributes();
                            if (attributes.hasAttribute(XmlTags::ModelUri)) {
                                QString modelUri = attributes.value(XmlTags::ModelUri).toString();
                                // Save the selected model to fill the model with the correct namespace
                                m_selectedModelUri = modelUri;
                                requiredModels.append(modelUri);
                            }
                        }
                    }
                }
            }
        }
    }
    file.close();
    return requiredModels;
}

QStringList DeviceDriverCore::findRequiredFiles(const QStringList& models, bool xmlOnly)
{
    // based on the requiredModels, find the corresponding NodeSet2.xml files
    QStringList requiredFiles;
    for (const QString& model : std::as_const(models)) {
        QString folderName = QStringLiteral("/") + model.section(QChar::fromLatin1('/'), -2, -2);
        // The UA NodeSet is always in the Schema folder
        QString path = m_nodeSetPath
                       + (folderName == QStringLiteral("/UA") ? QStringLiteral("/Schema")
                                                              : folderName);
        if (xmlOnly) {
            requiredFiles.append(getNodeSetXmlFile(path));
        } else {
            requiredFiles.append((getAllNodesetFiles(path)));
        }
    }
    return requiredFiles;
}

QStringList DeviceDriverCore::getAllNodesetFiles(const QString& dir)
{
    qDebug() << "Getting all Nodeset files from directory:" << dir;

    QStringList files;
    QDir directory(dir);

    if (!directory.exists()) {
        qDebug() << "Directory does not exist!" << dir;
    } else {
        QFileInfoList fileInfoList = directory.entryInfoList(QDir::Files);
        for (const QFileInfo& fileInfo : std::as_const(fileInfoList)) {
            // We only want the NodeSet2.xml file
            // TODO Sometimes there are multiple NodeSet2.xml files. Which one to choose? For now, just take the first one.
            if (fileInfo.fileName().contains(QStringLiteral("NodeSet2.xml"))
                || fileInfo.fileName().contains(QStringLiteral("NodeIds.csv"))
                || fileInfo.fileName().contains(QStringLiteral("Types.bsd"))
                       && !fileInfo.fileName().contains(QStringLiteral("Example"))) {
                files.append(fileInfo.absoluteFilePath());
            }
        }
    }

    return files;
}

QString DeviceDriverCore::getNodeSetXmlFile(const QString& dir)
{
    qDebug() << "Getting NodeSet2.xml file from directory:" << dir;
    QDir directory(dir);
    if (!directory.exists()) {
        qDebug() << "Directory does not exist!" << dir;
    } else {
        QFileInfoList fileInfoList = directory.entryInfoList(QDir::Files);

        for (const QFileInfo& fileInfo : std::as_const(fileInfoList)) {
            // We only want the NodeSet2.xml file
            // TODO Sometimes there are multiple NodeSet2.xml files. Which one to choose? For now, just take the first one.
            if (fileInfo.fileName().contains(QStringLiteral("NodeSet2.xml"))
                && !fileInfo.fileName().contains(QStringLiteral("Example"))) {
                return fileInfo.absoluteFilePath();
            }
        }
    }
    return QString();
}

void DeviceDriverCore::loadState(const QString& filePath)
{
#ifdef WASM_BUILD
    qDebug() << "Loading not supported for WebAssembly right now";
    return;
#endif
    QUrl url(filePath);
    QFile file;
    file.setFileName(url.toLocalFile());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open file for reading:" << filePath;
        emit openProjectReturned(false);
        return;
    }
    QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (jsonDoc.isNull()) {
        qWarning() << "Failed to parse JSON from file:" << filePath;
        emit openProjectReturned(false);
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    printMustacheData(jsonDoc);
    m_projectName = jsonObj[QStringLiteral("projectName")].toString();
    QJsonArray rootNodes = jsonObj[QStringLiteral("rootNodes")].toArray();
    QJsonArray selectedNodes = jsonObj[QStringLiteral("selectedNodes")].toArray();

    connect(this, &DeviceDriverCore::setupFinished, this, [this, rootNodes, selectedNodes]() {
        for (const QJsonValue& rootNodeVal : rootNodes) {
            if (!rootNodeVal.isObject()) {
                qWarning() << "Invalid rootNode format!";
                continue;
            }
            QJsonObject rootNode = rootNodeVal.toObject();

            addRootNodeToSelectionModel(
                rootNode[QStringLiteral("uri")].toString(),
                rootNode[QStringLiteral("nodeId")].toString());
            auto node = m_selectionModel->getItemByName(
                rootNode[QStringLiteral("originalUniqueBrowseName")].toString());
            node->setDisplayName(rootNode[QStringLiteral("displayName")].toString());
            node->setBrowseName(rootNode[QStringLiteral("browseName")].toString());
            node->setDescription(rootNode[QStringLiteral("description")].toString());
        }

        for (const QJsonValue& selectedNodeVal : selectedNodes) {
            if (!selectedNodeVal.isObject()) {
                qWarning() << "Invalid selectedNode format!";
                continue;
            }
            QJsonObject selectedNode = selectedNodeVal.toObject();
            auto node = m_selectionModel->getItemByName(
                selectedNode[QStringLiteral("originalUniqueBrowseName")].toString());
            node->setSelected(true);
            node->setDisplayName(selectedNode[QStringLiteral("displayName")].toString());
            node->setBrowseName(selectedNode[QStringLiteral("browseName")].toString());
            node->setDescription(selectedNode[QStringLiteral("description")].toString());
        }
    });

    selectNodeSetXML(jsonObj[QStringLiteral("selectedNodeSetXML")].toString());

    emit openProjectReturned(true);
}

void DeviceDriverCore::saveProject()
{
    QJsonObject data;
    QJsonArray rootNodes;
    QJsonArray selectedNodes;
    QList<TreeItem*> allNodes = getSelectedItems();
    data[QStringLiteral("projectName")] = m_projectName;
    data[QStringLiteral("selectedNodeSetXML")] = m_currentNodeSetDir;

    for (auto node : allNodes) {
        if (node->isRootNode()) {
            QJsonObject rootNodeData;
            rootNodeData[QStringLiteral("uri")] = node->namespaceString();
            rootNodeData[QStringLiteral("nodeId")] = node->nodeId();
            rootNodeData[QStringLiteral("displayName")] = node->displayName();
            rootNodeData[QStringLiteral("description")] = node->description();
            rootNodeData[QStringLiteral("browseName")] = node->browseName();
            rootNodeData[QStringLiteral("originalUniqueBrowseName")] = node->uniqueBaseBrowseName();
            rootNodes.append(rootNodeData);
        } else {
            QJsonObject selectedNodeData;
            selectedNodeData[QStringLiteral("nodeId")] = node->nodeId();
            selectedNodeData[QStringLiteral("displayName")] = node->displayName();
            selectedNodeData[QStringLiteral("description")] = node->description();
            selectedNodeData[QStringLiteral("browseName")] = node->browseName();
            selectedNodeData[QStringLiteral("originalUniqueBrowseName")]
                = node->uniqueBaseBrowseName();
            selectedNodes.append(selectedNodeData);
        }
    }

    data[QStringLiteral("rootNodes")] = rootNodes;
    data[QStringLiteral("selectedNodes")] = selectedNodes;
    QString jsonFileName = m_projectName + QStringLiteral("_project.json");

#ifndef WASM_BUILD
    saveToJson(m_outputFilePath + QStringLiteral("/") + jsonFileName, QJsonDocument(data));
    return;
#endif
    downloadFile(jsonFileName, QJsonDocument(data).toJson(QJsonDocument::Indented));
}

TreeModel* DeviceDriverCore::selectionModel() const
{
    return m_selectionModel;
}

ChildItemFilterModel* DeviceDriverCore::childItemFilterModel() const
{
    return m_childItemFilterModel;
}

void DeviceDriverCore::setExistingFilePath(const QString& newExistingFilePath)
{
    m_existingFilePath = newExistingFilePath;
}

void DeviceDriverCore::setProjectFilePath(const QString& newProjectFilePath)
{
    m_projectFilePath = newProjectFilePath;
}

void DeviceDriverCore::setOutputFilePath(const QString& newOutputFilePath)
{
    m_outputFilePath = newOutputFilePath;
}

void DeviceDriverCore::setMustacheTemplatePath(const QString& newMustacheTemplatePath)
{
    m_mustacheTemplatePath = newMustacheTemplatePath;
}

void DeviceDriverCore::setCmakeMustacheTemplatePath(const QString& newCmakeMustacheTemplatePath)
{
    m_cmakeMustacheTemplatePath = newCmakeMustacheTemplatePath;
}

QList<TreeItem*> DeviceDriverCore::getSelectedItems()
{
    QList<TreeItem*> selectedItems;
    travereseTreeModel(m_selectionModel, selectedItems);
    return selectedItems;
}

void DeviceDriverCore::travereseTreeModel(
    const QAbstractItemModel* model, QList<TreeItem*>& items, const QModelIndex& parent)
{
    int rowCount = model->rowCount(parent);
    for (int row = 0; row < rowCount; ++row) {
        QModelIndex index = model->index(row, 0, parent);
        if (!index.isValid()) {
            continue;
        }

        TreeItem* item = static_cast<TreeItem*>(index.internalPointer());

        if (item && (item->isSelected() || item->isRootNode())) {
            items.append(item);
        }

        // Recursively process children of the current node
        if (model->hasChildren(index)) {
            travereseTreeModel(model, items, index);
        }
    }
}

QString DeviceDriverCore::nodeSetPath() const
{
    return m_nodeSetPath;
}

RootNodeFilterModel* DeviceDriverCore::rootNodeFilterModel() const
{
    return m_rootNodeFilterModel;
}

QString DeviceDriverCore::projectName() const
{
    return m_projectName;
}

void DeviceDriverCore::setProjectName(const QString& newProjectName)
{
    if (m_projectName == newProjectName)
        return;
    m_projectName = newProjectName;
    emit projectNameChanged();
}

QString DeviceDriverCore::projectFilePath() const
{
    return m_projectFilePath;
}

QString DeviceDriverCore::outputFilePath() const
{
    return m_outputFilePath;
}

QString DeviceDriverCore::existingFilePath() const
{
    return m_existingFilePath;
}

QString DeviceDriverCore::appendProjectNameToPath(const QString& basePath)
{
    if (basePath.isEmpty() || m_projectName.isEmpty()) {
        return basePath;
    }

    // Combine base path and project name
    QString projectPath = QDir(basePath).filePath(m_projectName);

    // Ensure the path is unique
    return ensureUniqueDirectory(projectPath);
}

QString DeviceDriverCore::ensureUniqueDirectory(const QString& path)
{
    QString filePath = QUrl(path).toLocalFile();
    QFileInfo fileInfo(filePath);
    int counter = 1;

    QString originalPath = filePath;

    while (fileInfo.exists()) {
        filePath = originalPath + QStringLiteral("(%1)").arg(counter);
        fileInfo.setFile(filePath);
        counter++;

        if (counter > 100) {
            break;
        }
    }
    qDebug() << "Returning:" << filePath << "input:" << path;
    return filePath;
}
