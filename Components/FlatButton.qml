import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.impl

Button {
    id: control

    property color buttonColor: palette.button
    property var backgroundWidth: 100
    property var backgroundHeight: 40

    palette.button:  "#5A5A5A"

    palette.buttonText: control.enabled ? "white" : Qt.darker("white", 1.4)

    background: Rectangle {
        implicitWidth: backgroundWidth
        implicitHeight: backgroundHeight

        color: Color.blend(control.checked || control.highlighted ? palette.dark : buttonColor,
                           palette.mid, control.down ? 0.5 : 0.0)
        border.color: control.palette.highlight
        border.width: control.visualFocus ? 2 : 0
    }
}
