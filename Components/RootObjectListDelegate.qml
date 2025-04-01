// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

// RootObjectListDelegate.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.impl as QQCimpl
import Utils 1.0

Rectangle {
    id: treeDelegate

    radius: 5

    readonly property real padding: Utils.defaultPadding

    property color highlightColor: "lightblue"
    property color hoveredColor: "lightgrey"

    property bool hovered: false
    property bool selected: false

    signal toggleExpanded(int index)
    signal expand(int index)
    signal select(int index)

    color: selected ? highlightColor : hovered ? hoveredColor : "transparent"

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        hoverEnabled: true

        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onClicked: (mouse) => {
                       if (mouse.button === Qt.RightButton)
                       {
                           contextMenu.popup();
                       }
                       if (mouse.button === Qt.LeftButton)
                       {
                           select(index);
                       }
                   }
        onContainsMouseChanged: treeDelegate.hovered = containsMouse

        Menu {
            id: contextMenu

            MenuItem {
                // FIXME implement this
                text: "Remove from selection"
                onTriggered: {
                    select(-1)
                    core.removeRootNodeFromSelection(index)
                }

                palette.windowText: "black"
                palette.light: "white"
                palette.midlight: "#8A8A8A"
            }
        }
    }

    QQCimpl.ColorImage {
        id: icon

        anchors.left: parent.left
        anchors.verticalCenter: treeDelegate.verticalCenter

        width: Utils.defaultFontSize
        height: Utils.defaultFontSize

        color: palette.text
        source: {
            switch (model.typeName) {
            case "RootObject":
                return "../Icons/root.svg";
            case "UAObjectType":
                return "../Icons/root.svg";
            case "UAVariable":
                return "../Icons/variable.svg";
            case "UAMethod":
                return "../Icons/method.svg";
            default:
                return "../Icons/method.svg";
            }
        }

    }

    Text {
        id: label

        anchors.verticalCenter: treeDelegate.verticalCenter

        width: treeDelegate.width - treeDelegate.padding - x
        x: treeDelegate.padding + icon.width
        clip: true

        color: palette.text
        text: model.browseName + " -> " + model.nodeId
        elide: Text.ElideRight
    }
}
