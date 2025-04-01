// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls
import Utils 1.0
import QtQuick.Controls.impl as QQCimpl

TreeViewDelegate {
    id: treeDelegate

    background: Rectangle {
        radius: 5
        color: treeDelegate.highlighted
               ? treeDelegate.palette.highlight : treeDelegate.hovered ? Qt.lighter(palette.window) : "transparent"
    }

    contentItem: Rectangle {
        anchors.fill: parent
        color: "transparent"
        anchors.leftMargin: indicator.x  + Utils.normalSpacing * 3
        QQCimpl.ColorImage {
            id: icon

            anchors.right: parent.left
            anchors.bottom: parent.bottom
            anchors.bottomMargin: isWasm ? Utils.smallIconSize - 4 : 0
            anchors.rightMargin: Utils.smallSpacing

            source: {
                switch (model.typeName) {
                case "UAObjectType":
                    return "../Icons/root.svg";
                case "UAObject":
                    return "../Icons/object.svg";
                case "UAVariable":
                    return "../Icons/variable.svg";
                case "UAMethod":
                    return "../Icons/method.svg";
                default:
                    return "../Icons/method.svg";
                }
            }
            color: palette.text
            width: Utils.smallIconSize
            height: Utils.smallIconSize
        }
        Label {
            id: label

            text:  treeDelegate.model.browseName + " -> " + model.nodeId
            anchors.verticalCenter: parent.verticalCenter
            elide: Text.ElideRight
            color: palette.text

        }
        MouseArea {
            id: mouseArea

            anchors.fill: parent

            acceptedButtons: Qt.RightButton

            onClicked: (mouse) => {
                           if (mouse.button === Qt.RightButton)
                           {
                               contextMenu.popup();
                           }
                       }
            Menu {
                id: contextMenu

                enabled: model.isRootNode

                MenuItem {
                    text: "Add to selection"
                    onTriggered: {
                        console.log(model.nodeId, "added to selection.")
                        core.addRootNodeToSelectionModel(model.namespaceString, model.nodeId);
                    }

                    palette.windowText: "black"
                    palette.light: "white"
                    palette.midlight: "#8A8A8A"
                }
            }
        }
    }
}
