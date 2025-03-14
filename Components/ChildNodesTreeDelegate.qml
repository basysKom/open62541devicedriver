// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Layouts 1.15
import Utils 1.0
import QtQuick.Controls.impl as QQCimpl

TreeViewDelegate {
    id: treeDelegate

    signal setup(var item)

    property bool showCheckBox: true
    property int headerCells: 1
    property int cellWidth: width / headerCells

    visible: implicitHeight > 1

    background: Rectangle {
        radius: 5
        color: treeDelegate.selected
               ? treeDelegate.palette.highlight : treeDelegate.hovered ? Qt.lighter(palette.window) : "transparent"
    }

    IconButton {
        id: setupPopupButton

        anchors.verticalCenter: treeDelegate.verticalCenter

        opacity: enabled
        enabled: checkBox.checked

        iconSource: "../Icons/setup.svg"

        onClicked: {
            setup(model)
        }
    }

    contentItem: RowLayout  {
        anchors.left: indicator.right
        spacing: 0

        Item {
            id: selectItem

            Layout.fillWidth: true
            Layout.preferredWidth: treeDelegate.cellWidth

            CheckBox {
                id: checkBox

                anchors.verticalCenter: selectItem.verticalCenter

                visible: treeDelegate.showCheckBox

                checked: treeDelegate.model.isSelected
                enabled: !model.isOptional ? false : model.isParentSelected ? true : false
                onClicked: {
                    model.isSelected = checked;
                    if (checked  === true){
                        setup(model)
                    }
                }
            }

            QQCimpl.ColorImage {
                id: icon

                anchors.verticalCenter: selectItem.verticalCenter
                anchors.left: checkBox.right

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
                height: Utils.smallIconSize
                width: Utils.smallIconSize
            }
        }

        Text {
            id: displayNameLabel

            Layout.preferredWidth: treeDelegate.cellWidth

            color: palette.text
            text: model.browseName
            elide: Text.ElideRight
        }

        Text {
            id: idLabel

            Layout.preferredWidth: treeDelegate.cellWidth

            color: palette.text
            text: model.nodeId
            elide: Text.ElideRight
        }
    }
}
