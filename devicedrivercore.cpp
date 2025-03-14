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
#include <mstch/mstch.hpp>
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

void DeviceDriverCore::getVariableAsMustacheArray(TreeItem* item, mstch::map& nodeMap)
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

    nodeMap["dataType"] = mstch::node(
        Utils::instance()
            ->removeNamespaceIndexFromName(item->dataType().definitionName())
            .toStdString());
    nodeMap["dataTypeVariableName"] = mstch::node(
        Utils::instance()
            ->lowerFirstChar(Utils::instance()->removeNamespaceIndexFromName(item->displayName()))
            .toStdString());

    nodeMap["typesArrayName"] = mstch::node(typesArrayName.toStdString());
    nodeMap["typesArrayIndexAlias"] = mstch::node(typesArrayIndexAlias.toStdString());
    nodeMap["readUserCode"] = mstch::node(getUserCodeSegment(
                                              m_outputFilePath,
                                              QStringLiteral("//BEGIN user code read ") + nameStr,
                                              QStringLiteral("//END user code read ") + nameStr)
                                              .toStdString());
    nodeMap["writeUserCode"] = mstch::node(getUserCodeSegment(
                                               m_outputFilePath,
                                               QStringLiteral("//BEGIN user code write ") + nameStr,
                                               QStringLiteral("//END user code write ") + nameStr)
                                               .toStdString());

    mstch::array definitionFieldsArray;
    bool hasValue;

    QVariantList fields = item->definitionFields();
    for (int i = 0; i < fields.size(); ++i) {
        mstch::map definitionFields;
        definitionFields["fieldName"] = mstch::node(
            Utils::instance()
                ->lowerFirstChar(Utils::instance()->removeNamespaceIndexFromName(
                    fields.at(i).toMap()[QStringLiteral("key")].toString()))
                .toStdString());
        definitionFields["fieldType"] = mstch::node(
            fields.at(i).toMap()[QStringLiteral("value")].toString().toStdString());
        definitionFields["fieldValue"] = mstch::node(
            item->getValue(fields.at(i).toMap()[QStringLiteral("key")].toString())
                .toString()
                .toStdString());
        QString dataType = fields.at(i).toMap()[QStringLiteral("value")].toString();
        definitionFields["isString"] = mstch::node(
            dataType == QStringLiteral("String") || dataType == QStringLiteral("Locale"));
        hasValue = item->getValue(fields.at(i).toMap()[QStringLiteral("key")].toString()).isValid();

        definitionFieldsArray.push_back(definitionFields);
    }
    nodeMap["singleFieldValueFlag"] = mstch::node(fields.size() == 1);
    nodeMap["fieldsHaveValuesFlag"] = mstch::node(hasValue);
    nodeMap["definitionFields"] = mstch::node(definitionFieldsArray);
}

void DeviceDriverCore::getMethodAsMustacheArray(TreeItem* item, mstch::map& nodeMap)
{
    std::shared_ptr<UAMethod> methodNode = std::dynamic_pointer_cast<UAMethod>(item->getNode());

    mstch::array outputArgumentsArray;
    mstch::array inputArgumentsArray;

    std::shared_ptr<UAVariable> var = methodNode->inputArgument();
    int inputArgSize = var == nullptr ? 0 : (int) var->arguments().size();

    nodeMap["inputArgumentArrayDimensions"] = inputArgSize;
    for (int i = 0; i < inputArgSize; ++i) {
        mstch::map inputArgs;
        getArgumentsAsMustacheArray(i, var, inputArgs);
        inputArgs["argumentIndex"] = mstch::node(i);
        inputArgumentsArray.push_back(inputArgs);
    }

    var = methodNode->outputArgument();
    int outputArgSize = var == nullptr ? 0 : (int) var->arguments().size();

    nodeMap["outputArgumentArrayDimensions"] = outputArgSize;
    for (int i = 0; i < outputArgSize; ++i) {
        mstch::map outputArgs;
        getArgumentsAsMustacheArray(i, var, outputArgs);
        outputArgs["argumentIndex"] = mstch::node(i);
        outputArgumentsArray.push_back(outputArgs);
    }

    nodeMap["inputArguments"] = mstch::node(inputArgumentsArray);
    nodeMap["outputArguments"] = mstch::node(outputArgumentsArray);
    // FIXME use the user input here for variable name
    QString nameStr = Utils::instance()->sanitizeName(item->nodeVariableName());

    nodeMap["userCode"] = mstch::node(getUserCodeSegment(
                                          m_outputFilePath,
                                          QStringLiteral("//BEGIN user code ") + nameStr,
                                          QStringLiteral("//END user code ") + nameStr)
                                          .toStdString());
}

void DeviceDriverCore::getArgumentsAsMustacheArray(
    int index, std::shared_ptr<UAVariable> var, mstch::map& argMap)
{
    Argument arg = var->arguments().at(index);
    argMap["argumentName"] = mstch::node(Utils::instance()->lowerFirstChar(arg.name).toStdString());

    int nameSpaceIndex = Utils::instance()->extractNamespaceIndex(arg.dataTypeIdentifier);
    QString namespaceString
        = Utils::instance()->getNamespaceByIndex(var->namespaceString(), nameSpaceIndex);
    std::shared_ptr<UADataType> dataType = std::dynamic_pointer_cast<UADataType>(
        findNodeById(namespaceString, arg.dataTypeIdentifier));
    argMap["argumentDataType"] = mstch::node(
        Utils::instance()->removeNamespaceIndexFromName(dataType->browseName()).toStdString());

    QString nsName = Utils::instance()->extractNameFromNamespaceString(dataType->namespaceString());
    QString typesArrayName = QStringLiteral("UA_TYPES")
                             + (nsName.size() > 0 ? QStringLiteral("_") + nsName.toUpper()
                                                  : QStringLiteral(""));
    QString typesArrayIndexAlias
        = QStringLiteral("UA_TYPES")
          + (nsName.size() > 0 ? QStringLiteral("_") + nsName.toUpper() + QStringLiteral("_")
                               : QStringLiteral("_"))
          + Utils::instance()->removeNamespaceIndexFromName(dataType->browseName()).toUpper();
    argMap["typesArrayName"] = mstch::node(typesArrayName.toStdString());
    argMap["typesArrayIndexAlias"] = mstch::node(typesArrayIndexAlias.toStdString());
    argMap["isEnum"] = mstch::node(dataType->isEnum());

    if (dataType->isEnum()) {
        mstch::array enumValues;
        for (const auto [key, value] : dataType->definitionFields().asKeyValueRange()) {
            enumValues.push_back(mstch::node((key + QStringLiteral(" = ") + value).toStdString()));
        }
        argMap["argumentEnumValues"] = enumValues;
    } else {
        mstch::array dataTypeFields;
        for (const auto [key, value] : dataType->definitionFields().asKeyValueRange()) {
            mstch::map field;
            field["fieldName"] = mstch::node(Utils::instance()->lowerFirstChar(key).toStdString());
            // TODO those are just the field names from the NodeSet2.xml.
            // Maybe use a "findDataTypeByName" function to get the correct field type. (E.g. Duration is just a double)
            field["fieldType"] = mstch::node(value.toStdString());
            dataTypeFields.push_back(field);
        }
        argMap["dataTypeFields"] = dataTypeFields;
    }
}

mstch::map DeviceDriverCore::createNodeMap(
    int index, TreeItem* item, const QMap<int, QString>& namespaceMap)
{
    mstch::map nodeMap;

    QString nameStr = Utils::instance()->sanitizeName(item->nodeVariableName());
    nodeMap["nodeIndex"] = mstch::node(index);
    nodeMap["name"] = mstch::node(nameStr.toStdString());
    nodeMap["nodeId"] = mstch::node(item->nodeId().toStdString());
    nodeMap["identifier"] = mstch::node(
        Utils::instance()->extractIdentifier(item->nodeId()).toStdString());
    nodeMap["namespaceIndex"] = mstch::node(namespaceMap.key(item->namespaceString()));
    nodeMap["browseName"] = mstch::node(
        Utils::instance()->removeNamespaceIndexFromName(item->browseName()).toStdString());
    nodeMap["baseBrowseName"] = mstch::node(
        Utils::instance()->removeNamespaceIndexFromName(item->baseBrowseName()).toStdString());
    nodeMap["displayName"] = mstch::node(item->displayName().toStdString());
    nodeMap["description"] = mstch::node(item->description().toStdString());

    return nodeMap;
}

mstch::map DeviceDriverCore::getMustacheData()
{
    // TODO use defines for the nodeMap keys and the hardcoded strings?
    mstch::map data;
    mstch::array nameSpacesArray;
    mstch::array nodeSetsArray;
    mstch::array rootNodesArray;
    mstch::array objectNodesArray;
    mstch::array variableNodesArray;
    mstch::array methodNodesArray;

    // map for namespace mapping to match the correct namespace index.
    QMap<int, QString> namespaceMap;

    // Some top level infos. The number of namespaces and the nodesets.
    data["nsCount"] = mstch::node{(int) m_nodeSets.size()};
    int i = 0;
    for (auto it = m_nodeSets.begin(); it != m_nodeSets.end(); ++it) {
        std::shared_ptr<UANodeSet> nodeSet = it.value();
        mstch::map ns;
        ns["uri"] = mstch::node(nodeSet->getNameSpaceUri().toStdString());
        qDebug() << "Included Namespace: " << nodeSet->getNameSpaceUri();
        ns["index"] = mstch::node(i);
        namespaceMap.insert(i, nodeSet->getNameSpaceUri());
        nameSpacesArray.push_back(ns);
        ++i;

        QString nodeSetName = nodeSet->getNodeSetName();
        if (nodeSetName != QStringLiteral("")) {
            mstch::map nodeSetMap;
            nodeSetMap["name"] = mstch::node(nodeSetName.toStdString());
            nodeSetMap["hasCustomTypes"] = mstch::node(nodeSet->getHasCustomTypes());
            // insert nodesets at the beginning because we need the reverse order for the nodeset compiler
            nodeSetsArray.insert(nodeSetsArray.cbegin(), nodeSetMap);
        }
    }
    data["nameSpaces"] = mstch::node(nameSpacesArray);
    data["nodeSets"] = mstch::node(nodeSetsArray);

    // get all elements in the selection model
    QList<TreeItem*> allNodes = getSelectedItems();
    data["nodeCount"] = mstch::node{(int) allNodes.size()};
    qDebug() << allNodes.size() << " Nodes selected.";
    // Loop over all the selected nodes, create a map for each node and add it to the corresponding array
    for (int i = 0; i < allNodes.size(); ++i) {
        TreeItem* item = allNodes.at(i);
        mstch::map nodeMap = createNodeMap(i, item, namespaceMap);

        // TODO the root node will be placed in the UA_NS0ID_OBJECTSFOLDER with the reference UA_NS0ID_ORGANIZES for now.
        if (item->isRootNode()) {
            nodeMap["parentNodeId"] = mstch::node(
                QStringLiteral("UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER)").toStdString());
            nodeMap["referenceTypeNodeId"] = mstch::node(
                QStringLiteral("UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES)").toStdString());

            rootNodesArray.push_back(nodeMap);
        } else {
            std::string parentNodeIdStr = Utils::instance()
                                              ->sanitizeName(
                                                  item->parentNode().lock()->nodeVariableName()
                                                  + QStringLiteral("_NodeId"))
                                              .toStdString();
            nodeMap["parentNodeId"] = mstch::node(parentNodeIdStr);
            // TODO this assumes that all the ref type nodeids are in ns=0
            nodeMap["referenceTypeNodeId"] = mstch::node(
                (QStringLiteral("UA_NODEID_NUMERIC(0, ")
                 + Utils::instance()->extractIdentifier(parentReferenceNodeId(item->getNode()))
                 + QStringLiteral(")"))
                    .toStdString());
        }

        // add specific values for the different node types
        if (item->typeName() == XmlTags::UAObject) {
            objectNodesArray.push_back(nodeMap);
        } else if (item->typeName() == XmlTags::UAVariable) {
            getVariableAsMustacheArray(item, nodeMap);
            variableNodesArray.push_back(nodeMap);
        } else if (item->typeName() == XmlTags::UAMethod) {
            getMethodAsMustacheArray(item, nodeMap);
            methodNodesArray.push_back(nodeMap);
        }
    }

    if (methodNodesArray.size() > 0) {
        data["methodCount"] = mstch::node{(int) methodNodesArray.size()};
    }

    data["rootNodes"] = mstch::node(rootNodesArray);
    data["objectNodes"] = mstch::node(objectNodesArray);
    data["variableNodes"] = mstch::node(variableNodesArray);
    data["methodNodes"] = mstch::node(methodNodesArray);

    return data;
}

mstch::map DeviceDriverCore::getCMakeMustacheData()
{
    // TODO let the user set the project and executable name
    mstch::map data;
    data["projectName"] = mstch::node(
        Utils::instance()->extractNameFromNamespaceString(m_selectedModelUri).toStdString());
    data["executableName"] = mstch::node(
        Utils::instance()->extractNameFromNamespaceString(m_selectedModelUri).toStdString());
    QStringList requiredModels = findRequiredModels(getNodeSetXmlFile(m_currentNodeSetDir));

    mstch::array nodeSetsArray;
    int i = 0;
    for (auto it = m_nodeSets.begin(); it != m_nodeSets.end(); ++it) {
        std::shared_ptr<UANodeSet> nodeSet = it.value();
        mstch::map nodeSetMap;
        QString nodeSetName = nodeSet->getNodeSetName();
        if (nodeSetName != QStringLiteral("")) {
            QStringList requiredFiles;
            for (const auto& model : requiredModels) {
                if (model == nodeSet->getNameSpaceUri()) {
                    requiredFiles = findRequiredFiles({model}, false);
                } else {
                    nodeSetMap["depends"] = mstch::node(
                        Utils::instance()->extractNameFromNamespaceString(model).toStdString());
                }
            }

            for (auto& file : requiredFiles) {
                file = file.section(QLatin1Char('/'), -1);
                if (file.contains(QStringLiteral("NodeSet2.xml")))
                    nodeSetMap["file_ns"] = mstch::node(file.toStdString());
                if (file.contains(QStringLiteral("NodeIds.csv")))
                    nodeSetMap["file_csv"] = mstch::node(file.toStdString());
                if (file.contains(QStringLiteral("Types.bsd")))
                    nodeSetMap["file_bsd"] = mstch::node(file.toStdString());
            }

            nodeSetMap["name"] = mstch::node(nodeSetName.toStdString());
            nodeSetMap["nameUpper"] = mstch::node(nodeSetName.toUpper().toStdString());
            QString nodsetDirPrefix
                = nodeSet->getNameSpaceUri().section(QChar::fromLatin1('/'), -2, -2);

            nodeSetMap["nodsetDirPrefix"] = mstch::node(nodsetDirPrefix.toStdString());
            nodeSetMap["hasCustomTypes"] = mstch::node(nodeSet->getHasCustomTypes());
            // insert nodesets at the beginning because we need the reverse order for the nodeset compiler
            nodeSetsArray.insert(nodeSetsArray.cbegin(), nodeSetMap);
        }
    }
    data["nodeSets"] = mstch::node(nodeSetsArray);
    return data;
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
    std::ofstream outFile(filePath.toStdString());
    if (!outFile) {
        qWarning() << "Cannot open file for writing:" << filePath;
        return;
    }
    outFile << data;
    qDebug() << "File saved to:" << filePath;
}

QString DeviceDriverCore::getUserCodeSegment(
    const QString& fileName, const QString& startMarker, const QString& endMarker)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();

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

QJsonValue DeviceDriverCore::mstchNodeToJsonValue(const mstch::node& node)
{
    const std::type_info& type = node.type();

    if (type == typeid(std::string)) {
        return QString::fromUtf8(boost::get<std::string>(node).c_str());
    } else if (type == typeid(int)) {
        return boost::get<int>(node);
    } else if (type == typeid(double)) {
        return boost::get<double>(node);
    } else if (type == typeid(bool)) {
        return boost::get<bool>(node);
    } else if (type == typeid(mstch::array)) {
        QJsonArray jsonArray;
        const mstch::array& arr = boost::get<mstch::array>(node);
        for (const auto& elem : arr) {
            jsonArray.append(mstchNodeToJsonValue(elem));
        }
        return jsonArray;
    } else if (type == typeid(mstch::map)) {
        QJsonObject jsonObj;
        const mstch::map& map = boost::get<mstch::map>(node);
        for (const auto& pair : map) {
            jsonObj[QString::fromStdString(pair.first)] = mstchNodeToJsonValue(pair.second);
        }
        return jsonObj;
    }

    return QJsonValue();
}

QJsonObject DeviceDriverCore::mstchMapToJsonObject(const mstch::map& map)
{
    QJsonObject jsonObj;
    for (const auto& pair : map) {
        jsonObj[QString::fromStdString(pair.first)] = mstchNodeToJsonValue(pair.second);
    }
    return jsonObj;
}

void DeviceDriverCore::generateCode()
{
    qDebug() << "Generating Code...";
    auto codeContext = getMustacheData();
    auto cmakeContext = getCMakeMustacheData();
    auto codeTemplate = loadTemplateFile(m_mustacheTemplatePath);
    auto cmakeTemplate = loadTemplateFile(m_cmakeMustacheTemplatePath);
    saveToFile(m_outputFilePath, mstch::render(codeTemplate, codeContext));
    saveToFile(m_cmakeOutputFilePath, mstch::render(cmakeTemplate, cmakeContext));
}

void DeviceDriverCore::saveToJson()
{
    qDebug() << "Save to Json...";
    QJsonDocument jsonDoc = QJsonDocument(mstchMapToJsonObject(getMustacheData()));

    QFile file(m_jsonOutputFilePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(jsonDoc.toJson());
        file.close();
        qDebug() << "File saved to:" << m_jsonOutputFilePath;
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

TreeModel* DeviceDriverCore::selectionModel() const
{
    return m_selectionModel;
}

ChildItemFilterModel* DeviceDriverCore::childItemFilterModel() const
{
    return m_childItemFilterModel;
}

void DeviceDriverCore::setOutputFilePath(const QString& newOutputFilePath)
{
    m_outputFilePath = newOutputFilePath;
}

void DeviceDriverCore::setJsonOutputFilePath(const QString& newJsonOutputFilePath)
{
    m_jsonOutputFilePath = newJsonOutputFilePath;
}

void DeviceDriverCore::setCmakeOutputFilePath(const QString& newCmakeOutputFilePath)
{
    m_cmakeOutputFilePath = newCmakeOutputFilePath;
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
