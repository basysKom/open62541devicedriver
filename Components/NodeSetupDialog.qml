// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

// NodeSetupDialog.qml
import QtQuick 2.15
import QtQuick.Controls 2.15

Dialog {
    id: dialog
    
    title: "Title"
    modal: true
    standardButtons: Dialog.Close
    footer: DialogButtonBox {
        visible: count > 0
        delegate: FlatButton {
            width: dialog.count === 1 ? dialog.availableWidth / 2 : undefined
        }
    }

    property alias sourceItem: detailLoader.sourceItem

    property string headlineTextColor: "black"
    property string nodeIdColor: "black"

    Loader {
        id: detailLoader

        anchors.fill: parent

        property var sourceItem: null

        onSourceItemChanged: {
            if (sourceItem)
            {
                switch (dialog.sourceItem.typeName) {
                case "UAObject":
                    sourceComponent = objectDelegate;
                    return;
                case "UAVariable":
                    sourceComponent = variableDelegate;
                    return;
                case "UAMethod":
                    sourceComponent = methodDelegate;
                    return;
                default:
                    sourceComponent = rootObjectDelegate;
                    return;
                }
            }
        }
    }

    Component {
        id: rootObjectDelegate

        RootObjectDelegate {
            sourceItem: dialog.sourceItem
            headlineTextColor: dialog.headlineTextColor
            nodeIdColor: dialog.nodeIdColor
        }
    }

    Component {
        id: variableDelegate

        VariableDelegate {
            sourceItem: dialog.sourceItem
            headlineTextColor: dialog.headlineTextColor
            nodeIdColor: dialog.nodeIdColor
        }
    }

    Component {
        id: methodDelegate

        MethodDelegate {
            sourceItem: dialog.sourceItem
            headlineTextColor: dialog.headlineTextColor
            nodeIdColor: dialog.nodeIdColor
        }
    }

    Component {
        id: objectDelegate

        ObjectDelegate {
            sourceItem: dialog.sourceItem
            headlineTextColor: dialog.headlineTextColor
            nodeIdColor: dialog.nodeIdColor
        }
    }
}
