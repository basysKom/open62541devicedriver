// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs
import Utils 1.0
import "../Components"

Page {
    id: root

    property int rowHeight: isWasm ? Utils.defaultRowHeight * 1.5 : Utils.defaultRowHeight

    property var deviceTypesTreemodel: core.deviceTypesModel
    property var childItemModel: core.childItemFilterModel
    property var selectedTreeModel: core.selectionModel
    property var rootNodeModel: core.rootNodeFilterModel

    signal newProject

    Connections {
        target: core.selectionModel
        function onModelReset() {
            listView.selectedIndex = -1
        }
    }

    Dialog {
        id: generateCodedialog

        title: "Generate Code"
        modal: true

        anchors.centerIn: parent

        width: root.width * 0.5
        height: root.height * 0.5

        function setPath() {
            let finalPath = core.appendProjectNameToPath(folderDialog.selectedFolder);

            core.outputFilePath = finalPath;
            projectJsonPathField.text = finalPath;
        }

        onOpened: {
            setPath();
        }

        Connections {
            target: core
            function onGenerateCodeFinished() {
                finishedNote.visible = true;
                closeTimer.start();
            }
        }

        Timer {
            id: closeTimer
            repeat: false
            interval: 2000
            onTriggered: {
                finishedNote.visible = false;
                generateCodedialog.close();
            }
        }

        FlatButton {
            visible: !closeTimer.running
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            text: "Ok"
            width: generateCodedialog.font === 1 ? generateCodedialog.availableWidth / 2 : undefined
            enabled:  projectJsonPathField.text !== "" || isWasm
            onClicked: {
                core.generateCode(includeCmake.checked, includeJson.checked);
                if(saveProject.checked)
                    core.saveProject();
            }
        }

        Overlay.modal: Rectangle {
            color: palette.window
            opacity: 0.9
        }

        FolderDialog {
            id: folderDialog
            title: "Select Output Folder"

            onAccepted: {
                generateCodedialog.setPath()
            }
        }


        ColumnLayout {
            LabeledCheckbox {
                id: includeCmake

                labelText: "Include CMakeLists"
            }
            LabeledCheckbox {
                id: includeJson
                labelText: "Include JSON dump"
            }

            LabeledCheckbox {
                id: saveProject
                labelText: "Include Project File"
            }

            Text {
                text: "Output path"
                color: palette.windowText
            }

            Row {
                spacing: Utils.defaultSpacing

                visible: !isWasm

                TextField {
                    id: projectJsonPathField
                    placeholderText: "Set Project Output Folder"
                    implicitWidth: Utils.preferredLabelWidth * 2
                    color: palette.windowText
                    readOnly: true
                }

                FlatButton {
                    text: "Select Folder..."
                    backgroundHeight: 20
                    onClicked: {
                        folderDialog.open();
                    }
                }
            }
        }
        Text {
            id: finishedNote

            anchors.centerIn: parent
            anchors.topMargin: Utils.normalSpacing
            text: "Code generated!"
            font.pointSize: 22
            visible: false
            color: "red"
        }
    }

    Dialog {
        id: helpDialog

        anchors.centerIn: parent

        title: "Help"
        modal: true
        standardButtons: Dialog.Close
        footer: DialogButtonBox {
            visible: count > 0
            delegate: FlatButton {
                width: helpDialog.count === 1 ? helpDialog.availableWidth / 2 : undefined
            }
        }

        width: root.width * 0.5
        height: root.height * 0.5

        Text {
            anchors.centerIn: parent
            // TODO Add more detailed help text and do not put it in the qml file
            text: "1. Choose a Companion Specification from the dropdown in the headerbar \n
 2. Select nodes in the tree on the left with right click \n
 3. Inspect the selected nodes in the middle \n
 4. Select Methods, Variables and Objects to generate code for with the checkbox \n
 5. Once selected, you can set the value of the nodes in the popup \n
 4. Generate code with the button in the footer \n"
            color: root.palette.text
        }
    }

    header: Rectangle {
        width: parent.width
        height: 50
        color: root.palette.window

        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: "black"
        }

        Text {
            text: core.projectName
            anchors.centerIn: parent
            color: root.palette.text
        }

        IconButton  {
            id: helpButton

            anchors.left: parent.left
            anchors.leftMargin: 10
            width: isWasm ? Utils.defaultIconSize * 2 : Utils.defaultIconSize
            height: isWasm ? Utils.defaultIconSize * 2 : Utils.defaultIconSize
            anchors.verticalCenter: parent.verticalCenter
            iconSource: "../Icons/help.svg"
            visible: true
            onClicked: {
                helpDialog.visible = !helpDialog.visible
            }
        }

        FlatButton {
            id: newProjectButton

            anchors.left: helpButton.right
            anchors.leftMargin: 10

            text: "New Project..."
            implicitWidth: 150

            buttonColor: root.palette.highlight

            onClicked: {
                newProject();
            }
        }
    }

    footer: Rectangle {
        width: parent.width
        height: 50
        color: root.palette.window
        anchors.bottom: parent.bottom

        Rectangle {
            anchors.top: parent.top
            width: parent.width
            height: 1
            color: "black"
        }

        RowLayout {
            anchors.centerIn: parent
            anchors.margins: 10

            FlatButton {
                id: generateButton

                text: "Generate Code"
                onClicked: {
                    generateCodedialog.open()
                }
            }
        }
    }

    NodeSetupDialog {
        id: setupDialog

        visible: false

        anchors.centerIn: parent

        width: root.width * 0.5
        height: root.height * 0.6

        headlineTextColor: root.palette.text
        nodeIdColor: root.palette.dark

        Overlay.modal: Rectangle {
            color: palette.window
            opacity: 0.9
        }
    }

    GridLayout {
        id: viewContainer

        anchors.fill: parent

        anchors.margins: Utils.defaultPadding

        rows: 2
        columns: 3

        columnSpacing:  Utils.defaultPadding
        rowSpacing:  Utils.defaultPadding

        property var elementRaduis: 5

        Rectangle {
            id: nodeTree

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.row: 0
            Layout.column: 0
            Layout.rowSpan: 2

            radius: viewContainer.elementRaduis

            color: "transparent"

            function hide()
            {
                scrollView.visible = false;
                Layout.maximumWidth = 36;
            }
            function show()
            {
                scrollView.visible = true;
                Layout.maximumWidth = -1;
            }

            IconButton {
                id: hideButton

                width: isWasm ? Utils.defaultIconSize * 1.5 : Utils.defaultIconSize
                height: isWasm ? Utils.defaultIconSize * 1.5 : Utils.defaultIconSize

                text: scrollView.visible
                iconSource: "../Icons/arrow.svg"
                color: root.palette.text
                onClicked: {
                    if (scrollView.visible)
                    {
                        hideButton.iconRotation = 180;
                        nodeTree.hide();
                    } else {
                        hideButton.iconRotation = 0;
                        nodeTree.show();
                    }
                }
            }
            ScrollView {
                id: scrollView

                visible: true
                anchors.topMargin: hideButton.height
                anchors.fill: parent

                property int selectedIndex: -1

                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                TreeView {
                    id: deviceTypesTreeView

                    anchors.fill: parent
                    anchors.leftMargin: Utils.defaultPadding
                    anchors.rightMargin: Utils.normalSpacing
                    anchors.topMargin: Utils.defaultPadding
                    anchors.bottomMargin: Utils.defaultPadding

                    flickableDirection: Flickable.VerticalFlick

                    model: deviceTypesTreemodel
                    selectionModel: ItemSelectionModel {
                        model: deviceTypesTreemodel
                    }

                    selectionMode: TableView.SingleSelection

                    rowSpacing: Utils.smallSpacing

                    clip: true

                    delegate: DeviceNodesTreeDeletage{
                        implicitWidth: deviceTypesTreeView.width
                    }
                }
            }
        }

        Rectangle {
            id: currentSelectionContainer

            color: "transparent"
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.row: 0
            Layout.column: 1

            border.color: palette.text
            border.width: 1

            radius: viewContainer.elementRaduis

            Label {
                id: emptySelectionLabel

                anchors.centerIn: parent

                text: "No Nodes Selected"
                color: root.palette.text
                visible: listView.count === 0
                font.bold: true
                font.pixelSize: 20
            }
            ScrollView {
                visible: true
                anchors.fill: parent
                anchors.margins: 10

                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                ListView {
                    id: listView

                    anchors.fill: parent
                    anchors.margins: 10

                    model: rootNodeModel

                    property int selectedIndex: -1

                    onSelectedIndexChanged: {
                        let index = selectedTreeModel.index(selectedIndex, 0);
                        childItemModel.rootNodeIndex = index;
                    }

                    flickableDirection: Flickable.VerticalFlick
                    spacing: 5

                    clip: true

                    delegate: RootObjectListDelegate {
                        id: rootObjectDelegate

                        implicitHeight: root.rowHeight
                        implicitWidth: listView.width

                        highlightColor: root.palette.highlight
                        hoveredColor: Qt.lighter(palette.window)

                        selected: listView.selectedIndex === index

                        onSelect: (index) => {
                                      if (detailLoader.item) {
                                          detailLoader.item.beforeSelectionChanging();
                                      }
                                      listView.selectedIndex = index;
                                      detailLoader.sourceItem = model
                                  }
                    }
                }
            }
        }

        Rectangle {
            color: "transparent"
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.row: 0
            Layout.column: 2
            radius: viewContainer.elementRaduis
            border.color: palette.text
            border.width: 1

            Label {
                id: emptyDetailsLabel

                anchors.centerIn: parent

                text: "No Nodes Selected"
                color: root.palette.text
                visible: detailLoader.sourceItem === null
                font.bold: true
                font.pixelSize: 20
            }

            Loader {
                id: detailLoader

                anchors.fill: parent
                anchors.topMargin: isWasm ? Utils.defaultPadding : 0

                property var sourceItem: null

                onSourceItemChanged: {
                    if (sourceItem)
                    {
                        currentChildrenTreeView.expand(0);
                        sourceComponent = rootDelegate;
                    }
                }
            }

            Component {
                id: rootDelegate

                RootObjectDelegate {
                    anchors.fill: parent
                    sourceItem: detailLoader.sourceItem
                    headlineTextColor: root.palette.text
                    nodeIdColor: root.palette.dark
                }
            }
        }

        Rectangle {
            id: referencesContainer

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.row: 1
            Layout.column: 1
            Layout.columnSpan: 2
            
            color: "transparent"
            radius: viewContainer.elementRaduis

            border.color: palette.text
            border.width: 1

            Label {
                id: emptyReferencesLabel

                anchors.centerIn: parent

                text: "No References"
                color: root.palette.text
                visible: detailLoader.sourceItem === null
                font.bold: true
                font.pixelSize: 20
            }

            Rectangle {
                id: headerContainer
                color: root.palette.window
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right

                RowLayout {
                    id: headerLayout

                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: Utils.defaultPadding
                    anchors.rightMargin: Utils.defaultPadding

                    height: root.rowHeight

                    visible: detailLoader.sourceItem !== null
                    property var headerModel: ["Settings", "DisplayName", "NodeId"]

                    spacing: 0

                    Repeater {
                        model: headerLayout.headerModel
                        delegate: Item {
                            Text {
                                id: headerText
                                anchors.verticalCenter: parent.verticalCenter

                                Layout.preferredWidth: headerLayout.width / headerLayout.headerModel.length
                                Layout.fillWidth: true

                                text: modelData
                                color: palette.text
                                font.bold: true
                            }
                            Rectangle {
                                visible: index !== 0

                                anchors.right: headerText.left
                                anchors.top: headerText.top
                                anchors.bottom: headerText.bottom
                                anchors.rightMargin: Utils.smallSpacing

                                width: 1
                                height: referencesContainer.height + 5
                                color: root.palette.text
                            }
                        }
                    }
                }

                Rectangle {
                    id: separator

                    anchors.top: headerLayout.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right

                    height: 1
                    color: palette.text
                }
            }
            ScrollView {
                anchors.top: headerContainer.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                anchors.leftMargin: Utils.defaultPadding
                anchors.rightMargin: Utils.defaultPadding
                anchors.topMargin: root.rowHeight + 1
                anchors.bottomMargin: 1

                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                TreeView {
                    id: currentChildrenTreeView

                    flickableDirection: Flickable.VerticalFlick
                    model: childItemModel

                    rowSpacing: isWasm? Utils.defaultSpacing : Utils.smallSpacing

                    reuseItems: false

                    clip: true

                    delegate: ChildNodesTreeDelegate {
                        implicitWidth: currentChildrenTreeView.width
                        // Hide the parent
                        implicitHeight: index !== 0 ? root.rowHeight : 1
                        headerCells: headerLayout.headerModel.length

                        onSetup: (item) => {
                                     setupDialog.sourceItem = item
                                     setupDialog.open()
                                 }
                    }
                }
            }
        }
    }
}
