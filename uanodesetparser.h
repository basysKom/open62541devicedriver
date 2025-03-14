// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef UANODESETPARSER_H
#define UANODESETPARSER_H

#include "uanode.h"
#include "uanodeset.h"
#include <QDebug>
#include <QFile>
#include <QXmlStreamReader>

class UaNodeSetParser
{
public:
    UaNodeSetParser();
    bool parse(const QString& filePath, UANodeSet* nodeSet);

private:
    QFile m_file;

    void parseNodeSet(QXmlStreamReader& xml, UANodeSet* nodeSet);
    void parseUAObject(QXmlStreamReader& xml, std::shared_ptr<UAObject> object, UANodeSet* nodeSet);
    void parseUADataType(
        QXmlStreamReader& xml, std::shared_ptr<UADataType> dataType, UANodeSet* nodeSet);
    void parseUAVariable(
        QXmlStreamReader& xml, std::shared_ptr<UAVariable> variable, UANodeSet* nodeSet);
    void parseUAMethod(QXmlStreamReader& xml, std::shared_ptr<UAMethod> method, UANodeSet* nodeSet);
    void parseUAVariableType(
        QXmlStreamReader& xml, std::shared_ptr<UAVariableType> variableType, UANodeSet* nodeSet);
    void parseUAObjectType(
        QXmlStreamReader& xml, std::shared_ptr<UAObjectType> objectType, UANodeSet* nodeSet);
    void parseReferences(QXmlStreamReader& xml, std::shared_ptr<UANode> node, UANodeSet* nodeSet);
    void parseNamespaceMappping(QXmlStreamReader& xml, UANodeSet* nodeSet);
    void parseAliases(QXmlStreamReader& xml, UANodeSet* nodeSet);
    void parseDisplayName(QXmlStreamReader& xml, std::shared_ptr<UANode> node);

    bool findElement(QXmlStreamReader& xml, const QString& elementName);
    void parseListOfExtensionObject(QXmlStreamReader& xml, QVector<Argument>& arguments);
    Argument parseExtensionObject(QXmlStreamReader& xml);
    void parseArgument(QXmlStreamReader& xml, Argument& arg);
    void parseDataType(QXmlStreamReader& xml, Argument& arg);
    void parseTypeId(QXmlStreamReader& xml, Argument& arg);
};

#endif // UANODESETPARSER_H
