// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

// LabeledDropdown.qml
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import Utils 1.0

RowLayout { 
    id: root

    property alias labelText: label.text
    property alias labelColor: label.color
    property alias model: comboBox.model
    property alias currentIndex: comboBox.currentIndex
    property alias textRole: comboBox.textRole

    signal selected(var text)
    
    Label {
        id: label

        Layout.preferredWidth: Utils.preferredLabelWidth + Utils.normalSpacing
        Layout.alignment: Qt.AlignLeft
    }

    ComboBox {
        id: comboBox

        Layout.preferredWidth: Utils.preferredLabelWidth
        onActivated: (index) =>  {
                         selected(textAt(index))
                     }
    }
}
