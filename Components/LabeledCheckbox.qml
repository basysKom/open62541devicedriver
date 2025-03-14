// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

// LabeledCheckbox.qml
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import Utils 1.0

RowLayout {
    id: root

    property alias labelText: label.text
    property alias checked: checkBox.checked
    property alias labelColor: label.color
    property alias enabled: checkBox.enabled

    spacing: Utils.defaultSpacing

    signal toggled(bool checked)

    Label {
        id: label
        text: root.labelText
        Layout.preferredWidth: Utils.preferredLabelWidth
        Layout.alignment: Qt.AlignLeft
    }

    CheckBox {
        id: checkBox
        checked: false
        Layout.preferredWidth: Utils.preferredLabelWidth
        Layout.alignment: Qt.AlignRight

        onToggled: root.toggled(checked)
    }
}
