// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

// variableDelegate.qml
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import Utils 1.0

NodeDelegateBase {
    id: delegate

    iconSource: "../Icons/variable.svg"

    Dialog {
        id: valueDialog

        modal: true
        standardButtons: Dialog.Close

        anchors.centerIn: parent

        width: Utils.mainWindowWidth * 0.3
        height: Utils.mainWindowHeight * 0.3

        property var fields: sourceItem && sourceItem.definitionFields ? sourceItem.definitionFields : null

        ColumnLayout {
            id: detailLoader

            anchors.centerIn: parent

            Repeater {
                model: sourceItem ? sourceItem.definitionFields : null

                delegate:
                    LabeledInput {
                    labelText: modelData.key + " (" + modelData.value + "):"
                    placeHolderText: "Enter " + modelData.key
                    textFieldText: sourceItem.item.getValue(modelData.key) ? sourceItem.item.getValue(modelData.key) : ""
                    onEditingFinished: {
                        sourceItem.item.setValue(modelData.key, textFieldText)
                    }

                    Component.onCompleted: setDataType(modelData.value)
                }
            }

            LabeledCheckbox {
                id: isAbstract

                labelText: "Static Value: "
                checked: sourceItem && sourceItem.isAbstract ? sourceItem.isAbstract : false
                // INFO hidden for now until the functionality is implemented
                visible: false
                onCheckedChanged: {
                    // TODO use a write call to set the value in the server once.
                    console.log("TODO static value")
                }
            }
        }
    }

    content: ColumnLayout {
        id: contentItem

        visible: true

        LabeledInput {
            id: dataType

            Layout.fillWidth: true
            enabled: false

            labelText: "DataType:"
            placeHolderText: "Enter DataType"
            textFieldText: sourceItem && sourceItem.definitionName ? sourceItem.definitionName : ""
            labelColor: delegate.headlineTextColor

            onEditingFinished: {
                sourceItem.definitionName = textFieldText
            }

            onEditingStarted: valueDialog.open()
        }
        Button {
            id: setValueButton

            Layout.alignment: Qt.AlignHCenter

            text: "Set Value"
            onClicked: {
                valueDialog.open()
            }
        }
    }
}



