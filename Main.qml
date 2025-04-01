// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick
import QtQuick.Window 2.12
import Utils 1.0
import "./Screens"

Window {
    id: root
    visible: true
    width: Utils.mainWindowWidth
    height: Utils.mainWindowHeight
    title: qsTr("Device Driver")

    palette.window: "#2A2A2A"
    palette.windowText: "white"
    palette.base: "#8A8A8A"
    palette.text: "white"
    palette.highlight: "#BB86FC"
    palette.highlightedText: "black"

    NodeSelectionGridScreen {
        id: nodeSelectionGrid

        anchors.fill: parent
        anchors.margins: isWasm ? 80 : 0

        background: Rectangle {
            color: palette.window
            radius: 20
        }
    }
}

