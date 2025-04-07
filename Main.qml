// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick
import QtQuick.Window 2.12
import Qt.labs.folderlistmodel 2.6
import QtQuick.Controls 2.15
import QtQuick.Dialogs
import QtQuick.Layouts 1.15
import Utils 1.0
import "./Screens"
import "./Components"

Window {
    id: root
    visible: true
    width: Utils.mainWindowWidth
    height: Utils.mainWindowHeight
    title: qsTr("Device Driver")

    palette.window: "#2A2A2A"
    palette.windowText: "white"
    palette.base: "#4A4A4A"
    palette.text: "white"
    palette.highlight: "#BB86FC"
    palette.highlightedText: "black"
    palette.button:  "#5A5A5A"
    palette.placeholderText: Qt.darker("#FFFFFF", 1.4)
    palette.buttonText: "white"

    property bool newProject
    
    Component.onCompleted: {
        initDialog.open()
    }
    Connections {
        target: core
        function onSetupFinished() {
            globalLoadingSpinner.hideSpinner()
        }

        function onOpenProjectReturned(success) {
            if (success) {
                globalLoadingSpinner.hideSpinner();
                initDialog.close();
            } else {
                globalLoadingSpinner.hideSpinner();
            }
        }
    }

    LoadingSpinner {
        id: globalLoadingSpinner
        z:3
    }

    Dialog {
        id: initDialog
        title: "Initialization"

        modal: true

        anchors.centerIn: parent

        width: root.width * 0.6
        height: root.height * 0.6

        closePolicy: Popup.NoAutoClose

        Overlay.modal: Rectangle {
            color: palette.window
            opacity: 0.9
        }

        FlatButton {
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            text: "Ok"
            visible: !projectTypeRow.visible
            width: initDialog.font === 1 ? initDialog.availableWidth / 2 : undefined
            enabled: root.newProject ? companionDropdown.currentIndex !== -1 ? true : false : projectJsonPathField.text !== "" ? true : false
            onClicked: initDialog.accept();
        }

        function accept() {
            if(root.newProject) {
                if (projectNameField.text.trim() !== "") {
                    core.projectName = projectNameField.text.trim();
                }
                globalLoadingSpinner.showSpinner();
                initDialog.close();
                Qt.callLater(function() {
                    core.selectNodeSetXML(folderModel.get(companionDropdown.currentIndex, "filePath"));
                });
            } else {
                globalLoadingSpinner.showSpinner();
                core.loadState(core.projectFilePath);
            }
        }

        IconButton {
            id: backButton

            anchors.top: initDialog.top
            anchors.left: initDialog.left

            text: "Back"
            iconSource: "../Icons/arrow.svg"
            width: 40
            height: 40

            visible: !projectTypeRow.visible

            onClicked: {
                projectTypeRow.visible = true;
                newProjectContent.visible = false;
                loadProjectContent.visible = false;
            }
        }

        Column {
            id: projectTypeRow

            anchors.centerIn: parent
            anchors.topMargin: Utils.defaultSpacing * 2

            spacing: Utils.defaultSpacing * 2

            FlatButton {
                id: newProjectButton

                text: "Create Project..."
                implicitWidth: 150

                buttonColor: root.palette.highlight

                onClicked: {
                    root.newProject = true;
                    projectTypeRow.visible = false;
                    newProjectContent.visible = true;
                    loadProjectContent.visible = false;
                }
            }

            Column {
                FlatButton {
                    id: loadProjectButton
                    text: "Open Project..."
                    implicitWidth: 150
                    enabled: !isWasm
                    palette.buttonText: isWasm ? "grey" : root.palette.buttonText
                    onClicked: {
                        root.newProject = false;
                        projectTypeRow.visible = false;
                        newProjectContent.visible = false;
                        loadProjectContent.visible = true;
                    }
                }

                Text {
                    text: "Not yet supported for WebAssembly"
                    color: "red"
                    visible: isWasm
                }
            }
        }

        Column {
            id: newProjectContent

            anchors.centerIn: parent
            visible: false
            spacing: Utils.defaultSpacing
            width: parent.width * 0.45

            LabeledDropdown {
                id: companionDropdown

                anchors.left: parent.left
                anchors.right: parent.right

                model: folderModel
                textRole: "fileName"
                labelText: "Companion Spec:"
            }

            TextField {
                id: projectNameField

                anchors.left: parent.left
                anchors.right: parent.right

                placeholderText: "Project Name"
                text: core.projectName
            }

            FolderListModel {
                id: folderModel

                property url folderUrl: "file://" + core.nodeSetPath

                folder: folderUrl
                showDirs: true
            }
        }

        Column {
            id: loadProjectContent

            anchors.centerIn: parent
            visible: false

            spacing: Utils.defaultSpacing
            width: parent.width * 0.45

            Text {
                text: "Load Project JSON"
                anchors.left: parent.left
                color: palette.windowText
            }

            Row {
                spacing: Utils.defaultSpacing
                anchors.left: parent.left
                anchors.right: parent.right

                TextField {
                    id: projectJsonPathField
                    placeholderText: "Selected Project JSON Path"
                    implicitWidth: Utils.preferredLabelWidth * 2
                    color: palette.windowText
                    readOnly: true
                    onTextChanged: {
                        core.projectFilePath = text;

                    }
                }

                FlatButton {
                    text: "Select File..."
                    backgroundHeight: 20
                    onClicked: {
                        fileDialog.title = "Select Project JSON File";
                        fileDialog.open();
                    }
                }
            }

            Text {
                text: "Load Generated C File (Optional)"
                anchors.left: parent.left
                color: palette.windowText
            }

            Row {
                spacing: Utils.defaultSpacing
                anchors.left: parent.left
                anchors.right: parent.right

                TextField {
                    id: generatedCFilePathField
                    placeholderText: "Selected Generated C File Path"
                    implicitWidth: Utils.preferredLabelWidth * 2
                    readOnly: true
                    onTextChanged: {
                        core.existingFilePath = text;
                    }
                }

                FlatButton {
                    text: "Select File..."
                    backgroundHeight: 20
                    onClicked: {
                        fileDialog.title = "Select Generated C File";
                        fileDialog.selectExisting = true;
                        fileDialog.open();
                    }
                }
            }
        }
    }


    FileDialog {
        id: fileDialog
        nameFilters: selectExisting ? ["C Files (*.c)"] : ["JSON Files (*.json)"]
        property var selectExisting: false
        onAccepted: {
            if(selectExisting) {
                core.existingFilePath = fileDialog.selectedFile;
                generatedCFilePathField.text = core.existingFilePath

            } else {
                core.projectFilePath = fileDialog.selectedFile;
                projectJsonPathField.text = core.projectFilePath
            }
        }
    }

    Dialog {
        id: saveDialog

        modal: true

        anchors.centerIn: parent

        width: root.width * 0.6
        height: root.height * 0.6

        closePolicy: Popup.NoAutoClose

        function setPath() {
            let finalPath = core.appendProjectNameToPath(folderDialog.selectedFolder);

            core.outputFilePath = finalPath;
            savePath.text = finalPath;
        }

        FolderDialog {
            id: folderDialog
            title: "Select Output Folder"

            onAccepted: {
                saveDialog.setPath()
            }
        }

        ColumnLayout {
            anchors.centerIn: parent
            LabeledCheckbox {
                id: saveProject
                labelText: "Save Current Project File"
            }

            Row {
                spacing: Utils.defaultSpacing

                visible: !isWasm

                TextField {
                    id: savePath
                    placeholderText: "Set Project Output Folder"
                    implicitWidth: Utils.preferredLabelWidth * 2
                    color: palette.windowText
                    readOnly: true
                }

                FlatButton {
                    text: "Select Folder..."
                    backgroundHeight: 20
                    enabled: saveProject.checked
                    onClicked: {
                        folderDialog.open();
                    }
                }
            }
        }

        FlatButton {
            anchors.bottom: parent.bottom
            anchors.right: closeButton.left
            anchors.rightMargin: Utils.smallSpacing
            text: "Ok"
            enabled: saveProject.checked && ( savePath.text !== "" || isWasm) || !saveProject.checked
            onClicked: {
                if(saveProject.checked && ( savePath.text !== "" || isWasm)) {
                    core.saveProject();
                }

                saveDialog.close()
                projectTypeRow.visible = true;
                newProjectContent.visible = false;
                loadProjectContent.visible = false;
                initDialog.open()
            }
        }

        FlatButton {
            id: closeButton

            anchors.bottom: parent.bottom
            anchors.right: parent.right
            text: "Cancel"
            onClicked: {
                saveDialog.close()
            }
        }

        Overlay.modal: Rectangle {
            color: palette.window
            opacity: 0.9
        }


    }

    NodeSelectionGridScreen {
        id: nodeSelectionGrid

        anchors.fill: parent
        anchors.margins: isWasm ? 80 : 0
        onNewProject: {
            saveDialog.open()
        }

        background: Rectangle {
            color: palette.window
            radius: 20
        }
    }
}

