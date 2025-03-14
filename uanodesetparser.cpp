// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

#include "uanodesetparser.h"
#include "Util/Utils.h"

UaNodeSetParser::UaNodeSetParser() {}

bool UaNodeSetParser::parse(const QString& filePath, UANodeSet* nodeSet)
{
    m_file.setFileName(filePath);
    if (!m_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file for reading:" << m_file.errorString() << filePath;
        return false;
    }

    QXmlStreamReader xml(&m_file);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name().toString() == XmlTags::UANodeSet) {
                parseNodeSet(xml, nodeSet);
            }
        }
    }

    m_file.close();

    if (xml.hasError()) {
        qWarning() << "XML error:" << xml.errorString();
        return false;
    }

    return true;
}

void UaNodeSetParser::parseNodeSet(QXmlStreamReader& xml, UANodeSet* nodeSet)
{
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            // TODO Verify that the Models Emelement is present in all cases. if not use NamespaceUri from the UANodeSet Element
            if (xml.name().toString() == XmlTags::Models) {
                parseNamespaceMappping(xml, nodeSet);
            } else if (xml.name().toString() == XmlTags::UAObject) {
                std::shared_ptr<UAObject> object = std::make_shared<UAObject>();
                parseUAObject(xml, object, nodeSet);
                nodeSet->addNode(object);
            } else if (xml.name().toString() == XmlTags::UADataType) {
                nodeSet->setHasCustomTypes(true);
                std::shared_ptr<UADataType> dataType = std::make_shared<UADataType>();
                parseUADataType(xml, dataType, nodeSet);
                nodeSet->addNode(dataType);
            } else if (xml.name().toString() == XmlTags::UAVariable) {
                std::shared_ptr<UAVariable> variable = std::make_shared<UAVariable>();
                parseUAVariable(xml, variable, nodeSet);
                nodeSet->addNode(variable);
            } else if (xml.name().toString() == XmlTags::UAMethod) {
                std::shared_ptr<UAMethod> method = std::make_shared<UAMethod>();
                parseUAMethod(xml, method, nodeSet);
                nodeSet->addNode(method);
            } else if (xml.name().toString() == XmlTags::UAVariableType) {
                std::shared_ptr<UAVariableType> variableType = std::make_shared<UAVariableType>();
                parseUAVariableType(xml, variableType, nodeSet);
                nodeSet->addNode(variableType);
            } else if (xml.name().toString() == XmlTags::UAObjectType) {
                std::shared_ptr<UAObjectType> objectType = std::make_shared<UAObjectType>();
                parseUAObjectType(xml, objectType, nodeSet);
                nodeSet->addNode(objectType);
            } else if (xml.name().toString() == XmlTags::Aliases) {
                parseAliases(xml, nodeSet);
            }
        } else if (xml.isEndElement() && xml.name().toString() == XmlTags::UANodeSet) {
            break;
        }
    }
}

void UaNodeSetParser::parseUAObject(
    QXmlStreamReader& xml, std::shared_ptr<UAObject> object, UANodeSet* nodeSet)
{
    object->setNodeId(xml.attributes().value(XmlTags::NodeId).toString());
    object->setBrowseName(xml.attributes().value(XmlTags::BrowseName).toString());
    object->setBaseBrowseName(xml.attributes().value(XmlTags::BrowseName).toString());
    object->setDescription(xml.attributes().value(XmlTags::Description).toString());
    object->setParentNodeId(xml.attributes().value(XmlTags::ParentNodeId).toString());
    object->setNamespaceString(nodeSet->getNameSpaceUri());

    parseDisplayName(xml, object);
    parseReferences(xml, object, nodeSet);
}

void UaNodeSetParser::parseUADataType(
    QXmlStreamReader& xml, std::shared_ptr<UADataType> dataType, UANodeSet* nodeSet)
{
    dataType->setNodeId(xml.attributes().value(XmlTags::NodeId).toString());
    // setting the DefinitionName to BrowseName since base datatypes have no Definition Tag.
    dataType->setDefinitionName(xml.attributes().value(XmlTags::BrowseName).toString());
    dataType->setBrowseName(xml.attributes().value(XmlTags::BrowseName).toString());
    dataType->setBaseBrowseName(xml.attributes().value(XmlTags::BrowseName).toString());
    dataType->setDescription(xml.attributes().value(XmlTags::Description).toString());
    dataType->setParentNodeId(xml.attributes().value(XmlTags::ParentNodeId).toString());
    dataType->setNamespaceString(nodeSet->getNameSpaceUri());
    parseDisplayName(xml, dataType);
    parseReferences(xml, dataType, nodeSet);

    if (dataType->definitionName() == QStringLiteral("LocalizedText")) {
        dataType->addDefinitionField(QStringLiteral("Locale"), QStringLiteral("Locale"));
        dataType->addDefinitionField(QStringLiteral("Text"), QStringLiteral("String"));
    }

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name().toString() == XmlTags::Definition) {
                // overwrite definition name if the datatype has a Definiton Tag
                dataType->setDefinitionName(xml.attributes().value(XmlTags::Name).toString());
                while (!xml.atEnd()) {
                    xml.readNext();
                    if (xml.isStartElement() && xml.name().toString() == XmlTags::Field) {
                        // To distinguish between enums and other types, we check if the value is set.
                        // a value represents the enum index. If there is no value, we have a normal datatype.
                        QString value = xml.attributes().value(XmlTags::Value).toString();
                        if (value != QStringLiteral("")) {
                            dataType->setIsEnum(true);
                            dataType->addDefinitionField(
                                xml.attributes().value(XmlTags::Name).toString(), value);
                        } else {
                            dataType->addDefinitionField(
                                xml.attributes().value(XmlTags::Name).toString(),
                                xml.attributes().value(XmlTags::DataType).toString());
                        }

                    } else if (xml.isEndElement() && xml.name().toString() == XmlTags::Definition) {
                        break;
                    }
                }
            }
        } else if (xml.isEndElement() && xml.name().toString() == XmlTags::UADataType) {
            break;
        }
    }
}

void UaNodeSetParser::parseUAVariable(
    QXmlStreamReader& xml, std::shared_ptr<UAVariable> variable, UANodeSet* nodeSet)
{
    variable->setNodeId(xml.attributes().value(XmlTags::NodeId).toString());
    variable->setBrowseName(xml.attributes().value(XmlTags::BrowseName).toString());
    variable->setBaseBrowseName(xml.attributes().value(XmlTags::BrowseName).toString());
    variable->setDescription(xml.attributes().value(XmlTags::Description).toString());
    UADataType dataType;
    dataType.setDefinitionName(xml.attributes().value(XmlTags::DataType).toString());
    variable->setDataType(dataType);
    variable->setParentNodeId(xml.attributes().value(XmlTags::ParentNodeId).toString());
    variable->setValueRank(xml.attributes().value(XmlTags::ValueRank).toInt());
    variable->setArrayDimensions(xml.attributes().value(XmlTags::ArrayDimensions).toInt());
    variable->setNamespaceString(nodeSet->getNameSpaceUri());

    parseDisplayName(xml, variable);
    parseReferences(xml, variable, nodeSet);

    QVector<Argument> arguments;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name().toString() == QStringLiteral("Value")) {
                parseListOfExtensionObject(xml, arguments);
            }
        } else if (xml.isEndElement() && xml.name().toString() == XmlTags::UAVariable) {
            break;
        }
    }

    variable->setArguments(arguments);
}

void UaNodeSetParser::parseUAMethod(
    QXmlStreamReader& xml, std::shared_ptr<UAMethod> method, UANodeSet* nodeSet)
{
    method->setNodeId(xml.attributes().value(XmlTags::NodeId).toString());
    method->setBrowseName(xml.attributes().value(XmlTags::BrowseName).toString());
    method->setBaseBrowseName(xml.attributes().value(XmlTags::BrowseName).toString());
    method->setDescription(xml.attributes().value(XmlTags::Description).toString());
    method->setParentNodeId(xml.attributes().value(XmlTags::ParentNodeId).toString());
    method->setNamespaceString(nodeSet->getNameSpaceUri());

    parseDisplayName(xml, method);
    parseReferences(xml, method, nodeSet);
}

void UaNodeSetParser::parseUAVariableType(
    QXmlStreamReader& xml, std::shared_ptr<UAVariableType> variableType, UANodeSet* nodeSet)
{
    variableType->setNodeId(xml.attributes().value(XmlTags::NodeId).toString());
    variableType->setBrowseName(xml.attributes().value(XmlTags::BrowseName).toString());
    variableType->setBaseBrowseName(xml.attributes().value(XmlTags::BrowseName).toString());
    variableType->setDescription(xml.attributes().value(XmlTags::Description).toString());
    variableType->setParentNodeId(xml.attributes().value(XmlTags::ParentNodeId).toString());
    variableType->setIsAbstract(
        xml.attributes().value(XmlTags::IsAbstract).toString() == QStringLiteral("true"));
    variableType->setNamespaceString(nodeSet->getNameSpaceUri());

    parseDisplayName(xml, variableType);
    parseReferences(xml, variableType, nodeSet);
}

void UaNodeSetParser::parseUAObjectType(
    QXmlStreamReader& xml, std::shared_ptr<UAObjectType> objectType, UANodeSet* nodeSet)
{
    objectType->setNodeId(xml.attributes().value(XmlTags::NodeId).toString());
    objectType->setBrowseName(xml.attributes().value(XmlTags::BrowseName).toString());
    objectType->setBaseBrowseName(xml.attributes().value(XmlTags::BrowseName).toString());
    objectType->setDescription(xml.attributes().value(XmlTags::Description).toString());
    objectType->setParentNodeId(xml.attributes().value(XmlTags::ParentNodeId).toString());
    objectType->setIsAbstract(
        xml.attributes().value(XmlTags::IsAbstract).toString() == QStringLiteral("true"));
    objectType->setNamespaceString(nodeSet->getNameSpaceUri());

    parseDisplayName(xml, objectType);
    parseReferences(xml, objectType, nodeSet);
}

void UaNodeSetParser::parseReferences(
    QXmlStreamReader& xml, std::shared_ptr<UANode> node, UANodeSet* nodeSet)
{
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name().toString() == XmlTags::Reference) {
                QString referenceType = xml.attributes().value(XmlTags::ReferenceType).toString();
                bool isForward = xml.attributes().value(XmlTags::IsForward).toString().length() > 0
                                     ? false
                                     : true;
                QString targetId = xml.readElementText();
                QString nameSpaceString = nodeSet->getNamespaceUriByIndex(
                    Utils::instance()->extractNamespaceIndex(targetId));

                if (nameSpaceString.isEmpty()) {
                    qWarning() << "NamespaceString not found for targetId" << targetId;
                }
                node->addReference(
                    std::make_shared<Reference>(referenceType, targetId, isForward, nameSpaceString));
            }
        } else if (xml.isEndElement() && xml.name().toString() == XmlTags::References) {
            break;
        }
    }
}

void UaNodeSetParser::parseNamespaceMappping(QXmlStreamReader& xml, UANodeSet* nodeSet)
{
    // we start from the second namespace index, because the first two namespaces are already mapped for the own namespace and the UA namespace
    int namespaceIndex = 2;
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            //RequiredModel is the URI of the required Companion Specification
            if (xml.name().toString() == XmlTags::RequiredModel) {
                QXmlStreamAttributes attributes = xml.attributes();
                if (attributes.hasAttribute(XmlTags::ModelUri)) {
                    QString modelUri = attributes.value(XmlTags::ModelUri).toString();
                    if (modelUri == QStringLiteral("http://opcfoundation.org/UA/")) {
                        nodeSet->addNamespaceMapEntry(0, modelUri);
                    } else {
                        nodeSet->addNamespaceMapEntry(namespaceIndex, modelUri);
                        namespaceIndex++;
                    }
                }
                // "Model" is the URI of the selected Companion Specification
            } else if (xml.name().toString() == XmlTags::Model) {
                QXmlStreamAttributes attributes = xml.attributes();
                if (attributes.hasAttribute(XmlTags::ModelUri)) {
                    QString modelUri = attributes.value(XmlTags::ModelUri).toString();
                    // special case for the map to work. The UA namespace is always the first namespace
                    if (modelUri == QStringLiteral("http://opcfoundation.org/UA/")) {
                        nodeSet->addNamespaceMapEntry(0, modelUri);
                        nodeSet->setNamespaceUri(modelUri);
                    } else {
                        nodeSet->addNamespaceMapEntry(1, modelUri);
                        nodeSet->setNamespaceUri(modelUri);
                    }
                }
            }
        } else if (xml.isEndElement() && xml.name().toString() == XmlTags::Models) {
            break;
        }
    }
}

void UaNodeSetParser::parseAliases(QXmlStreamReader& xml, UANodeSet* nodeSet)
{
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name().toString() == XmlTags::Alias) {
                QString alias = xml.attributes().value(XmlTags::Alias).toString();
                QString target = xml.readElementText();
                nodeSet->addAlias(alias, target);
            }
        } else if (xml.isEndElement() && xml.name().toString() == XmlTags::Aliases) {
            break;
        }
    }
}

void UaNodeSetParser::parseDisplayName(QXmlStreamReader& xml, std::shared_ptr<UANode> node)
{
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name().toString() == XmlTags::DisplayName) {
                node->setDisplayName(xml.readElementText());
                break;
            }
        } else if (xml.isEndElement() && xml.name().toString() == XmlTags::DisplayName) {
            break;
        }
    }
}

void UaNodeSetParser::parseListOfExtensionObject(QXmlStreamReader& xml, QVector<Argument>& arguments)
{
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name().toString() == QStringLiteral("ListOfExtensionObject")) {
                while (!xml.atEnd()) {
                    xml.readNext();
                    if (xml.isStartElement()) {
                        if (xml.name().toString() == QStringLiteral("ExtensionObject")) {
                            arguments.append(parseExtensionObject(xml));
                        }
                    } else if (
                        xml.isEndElement()
                        && xml.name().toString() == QStringLiteral("ListOfExtensionObject")) {
                        break;
                    }
                }
            } else if (
                xml.isEndElement()
                && xml.name().toString() == QStringLiteral("ListOfExtensionObject")) {
                break;
            }
        } else if (xml.isEndElement() && xml.name().toString() == QStringLiteral("Value")) {
            break;
        }
    }
}

Argument UaNodeSetParser::parseExtensionObject(QXmlStreamReader& xml)
{
    Argument arg;
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name().toString() == QStringLiteral("TypeId")) {
                parseTypeId(xml, arg);
            }
            if (xml.name().toString() == QStringLiteral("Body")) {
                parseArgument(xml, arg);
            }
        } else if (xml.isEndElement() && xml.name().toString() == QStringLiteral("ExtensionObject")) {
            return arg;
        }
    }
    return arg;
}

void UaNodeSetParser::parseArgument(QXmlStreamReader& xml, Argument& arg)
{
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name().toString() == QStringLiteral("Name")) {
                arg.name = xml.readElementText();
            } else if (xml.name().toString() == QStringLiteral("DataType")) {
                parseDataType(xml, arg);
            } else if (xml.name().toString() == QStringLiteral("ValueRank")) {
                arg.valueRank = xml.readElementText().toInt();
            }
        } else if (xml.isEndElement() && xml.name().toString() == QStringLiteral("Body")) {
            break;
        }
    }
}

void UaNodeSetParser::parseDataType(QXmlStreamReader& xml, Argument& arg)
{
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name().toString() == QStringLiteral("Identifier")) {
                arg.dataTypeIdentifier = xml.readElementText();
            }
        } else if (xml.isEndElement() && xml.name().toString() == QStringLiteral("DataType")) {
            break;
        }
    }
}

void UaNodeSetParser::parseTypeId(QXmlStreamReader& xml, Argument& arg)
{
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name().toString() == QStringLiteral("Identifier")) {
                arg.dataTypeIdentifier = xml.readElementText();
            }
        } else if (xml.isEndElement() && xml.name().toString() == QStringLiteral("TypeId")) {
            break;
        }
    }
}
