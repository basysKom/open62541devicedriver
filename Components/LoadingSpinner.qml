// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick
import QtQuick.Controls

Item {
    id: globalLoadingSpinner

    anchors.fill: parent
    visible: false

    signal showSpinner()
    signal hideSpinner()

    onShowSpinner: { visible = true }
    onHideSpinner: { visible = false }

    Rectangle {
        anchors.fill: parent
        color: palette.window
        opacity: 0.9
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: globalLoadingSpinner.visible

        width: 80
        height: 80
    }
}
