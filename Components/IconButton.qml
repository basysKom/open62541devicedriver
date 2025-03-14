// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.impl as QQCimpl

Button {
    id: root

    required property string iconSource
    property alias color: icon.color
    property alias iconRotation: icon.rotation

    flat: true

    contentItem: QQCimpl.ColorImage {
        id: icon
        source: root.iconSource
        color: palette.text
        scale: 0.8

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: root.clicked()
        }
    }

    background: Rectangle {
        color: "transparent"
    }

    // fancy hover animation to make the button more interactive
    states: [
        State {
            name: "hovered"
            when: root.hovered
            PropertyChanges {
                root.scale: 1.1
            }
        }
    ]

    transitions: [
        Transition {
            from: "*"; to: "*"
            NumberAnimation {
                properties: "scale"
                duration: 100
                easing.type: Easing.InOutQuad
            }
        }
    ]
}
