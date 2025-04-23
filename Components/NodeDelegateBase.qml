// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later
// NodeDelegateBase.qml
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.impl as QQCimpl
import Utils 1.0

Item {
    id: root

    property var sourceItem: null
    property var setValue: null
    property alias headlineTextColor: headlineText.color
    property alias nodeIdColor: nodeIdText.color
    property alias iconSource: icon.source
    property alias content: content.data
    property var disabledFields: ["nodeId"]

    signal beforeSelectionChanging

    clip: true

    Row {
        id: headlineRow

        anchors.horizontalCenter: parent.horizontalCenter
        spacing: Utils.smallSpacing

        QQCimpl.ColorImage {
            id: icon

            width: Utils.defaultIconSize
            height: Utils.defaultIconSize

            fillMode: Image.PreserveAspectFit
            color: headlineTextColor
        }

        Text {
            id: headlineText

            text: sourceItem ? sourceItem.browseName : ""
            elide: Text.ElideRight
            font.bold: true
            font.pixelSize: Utils.defaultFontSize
        }

        Text {
            id: nodeIdText

            anchors.bottom: headlineText.bottom

            text: sourceItem ? sourceItem.nodeId : ""
            font.pixelSize: Utils.smallFontSize
            elide: Text.ElideRight
        }
    }

    ColumnLayout {
        id: detailLoader

        anchors.top: headlineRow.bottom
        anchors.topMargin: Utils.defaultFontSize
        anchors.horizontalCenter: parent.horizontalCenter
        Layout.fillHeight: false

        Repeater {
            model: sourceItem ? sourceItem.userInputMask : null

            delegate:
                Column {
                id: detailsDelegate

                property bool isInvald: false

                LabeledInput {
                    id: labeledInput

                    enabled: disabledFields.indexOf(modelData) === -1
                    labelText: modelData + ":"
                    placeHolderText: "Enter " + modelData
                    textFieldText: root.sourceItem[modelData] ? root.sourceItem[modelData] : ""
                    onEditingFinished: {

                        if (modelData === "browseName" && textFieldText !== root.sourceItem[modelData]) {
                            var isUnique = core.selectionModel.isBrowseNameUnique(textFieldText)
                            if (!isUnique) {
                                console.log("Name already exists")
                                detailsDelegate.isInvald = true;
                                return
                            }
                        }
                        root.sourceItem[modelData] = textFieldText
                    }

                    Connections {
                        target: root
                        function onBeforeSelectionChanging() {
                            labeledInput.editingFinished()
                            detailsDelegate.isInvald = false;
                        }
                    }
                }

                Text {
                    id: warningText

                    anchors.horizontalCenter: parent.horizontalCenter
                    color: "red"
                    text: "Name already exists!"

                    visible: detailsDelegate.isInvald
                }
            }
        }

        Item {
            id: content
        }
    }
}
