// SPDX-FileCopyrightText: 2025 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 Marius Dege <marius.dege@basyskom.com>
// SPDX-FileCopyrightText: 2024 basysKom GmbH

// SPDX-License-Identifier: LGPL-3.0-or-later

// LabeledInput.qml
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import Utils 1.0

RowLayout { 
    id: root

    property alias labelText: label.text
    property alias textFieldText: textField.text
    property alias textColor: textField.color
    property alias labelColor: label.color
    property alias placeHolderTextColor: textField.placeholderTextColor
    property alias placeHolderText: textField.placeholderText
    property alias validator: textField.validator
    property bool enabled: true
    property bool booleanValue: false
    property bool dateValue: false

    spacing: Utils.defaultSpacing

    signal editingFinished()
    signal editingStarted()

    function setDataType(dataType: string) {
        switch (dataType) {
        case "Boolean":
            booleanValue = true
            dateValue = false
            break
        case "String":
            textField.validator = stringValidator
            break
        case "Locale":
            textField.validator = localeValidator
            break
        case "Byte":
            textField.validator = byteValidator
            break
        case "SByte":
            textField.validator = sbyteValidator
            break
        case "Int16":
            textField.validator = int16Validator
            break
        case "UInt16":
            textField.validator = uint16Validator
            break
        case "Int32":
            textField.validator = int32Validator
            break
        case "UInt32":
            textField.validator = uint32Validator
            break
        case "UInt64":
            textField.validator = uint64Validator
            break
        case "Int64":
            textField.validator = int64Validator
            break
        case "Float":
            textField.validator = floatValidator
            break
        case "Double":
            textField.validator = doubleValidator
            break
        case "DateTime":
            textField.validator = dateTimeValidator
            dateValue = true
            booleanValue = false
            break
        case "Guid":
            textField.validator = guidValidator
            break
        default:
            console.log("No validator found for dataType", dataType)
            textField.validator = null
        }
    }

    property var localeValidator: RegularExpressionValidator { regularExpression: /^([a-z]{2}-[A-Z]{2})$/ }
    property var stringValidator: RegularExpressionValidator { regularExpression: /[a-zA-Z0-9]+/ }
    property var byteValidator: RegularExpressionValidator { regularExpression: /^(?:[0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])$/ }
    property var sbyteValidator: RegularExpressionValidator { regularExpression: /^(-?[0-9]|-1[0-2][0-7]|[1-9][0-9]?)$/ }
    property var int16Validator: RegularExpressionValidator { regularExpression: /^-?(?:[0-9]{1,4}|[1-2][0-9]{4}|3[0-2][0-9]{3}|3276[0-7])$/ }
    property var uint16Validator: RegularExpressionValidator { regularExpression: /^(?:[0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|6553[0-5])$/ }
    property var int32Validator: RegularExpressionValidator { regularExpression: /^-?(?:[0-9]{1,9}|1[0-9]{9}|2[0-1][0-9]{8}|214748364[0-7])$/ }
    property var uint32Validator: RegularExpressionValidator { regularExpression: /^(?:[0-9]{1,9}|[1-3][0-9]{9}|4[0-2][0-9]{8}|429496729[0-5])$/ }
    property var uint64Validator: RegularExpressionValidator { regularExpression: /^(?:[0-9]{1,19}|1[0-7][0-9]{18}|18[0-3][0-9]{17}|184[0-3][0-9]{16}|18446[0-6][0-9]{14}|184467[0-3][0-9]{12}|184467440[0-6][0-9]{10}|18446744073[0-6][0-9]{8}|184467440737[0-8][0-9]{6}|18446744073709[0-4][0-9]{4}|1844674407370955[0-1][0-9]{2}|184467440737095516[0-5])$/}
    property var int64Validator: RegularExpressionValidator { regularExpression: /^-?(?:[0-9]{1,18}|9[0-1][0-9]{17}|922[0-2][0-9]{15}|922337[0-1][0-9]{13}|922337203[0-5][0-9]{10}|92233720368[0-4][0-9]{8}|922337203685[0-3][0-9]{6}|92233720368547[0-6][0-9]{4}|922337203685477[0-5][0-9]{2}|922337203685477580[0-7])$/}
    property var floatValidator: RegularExpressionValidator { regularExpression: /^-?[0-9]*\.?[0-9]+$/ }
    property var doubleValidator: RegularExpressionValidator { regularExpression: /^-?[0-9]*\.?[0-9]+$/ }
    property var dateTimeValidator: RegularExpressionValidator { regularExpression: /^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(?:\.\d+)?Z?$/ }
    property var guidValidator: RegularExpressionValidator { regularExpression: /^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$/ }
    
    Label {
        id: label

        text: root.labelText

        Layout.preferredWidth: Utils.preferredLabelWidth
        Layout.alignment: Qt.AlignLeft
    }

    TextField {
        id: textField

        Layout.preferredWidth: Utils.preferredLabelWidth
        visible: root.enabled && !root.booleanValue

        text: root.textFieldText

        Rectangle {
            color: "transparent"

            border.color: !textField.acceptableInput && textField.activeFocus ? "red" : "transparent"
            border.width: 1

            anchors.fill: parent
        }

        // Input mask for ISO 8601-1 DateTime
        inputMask: root.dateValue ? "9999-99-99T99:99:99Z" : validator === localeValidator ? "AA-AA" : null

        Component.onCompleted: {
            textField.cursorPosition = 0
        }
        onActiveFocusChanged: {
            if (!activeFocus) {
                cursorPosition = 0
            } else {
                root.editingStarted()
            }
        }
        onEditingFinished: root.editingFinished()
    }

    Label {
        id: disabledText

        text: textFieldText

        visible: !root.enabled && !root.booleanValue && !root.dateValue

        Layout.preferredWidth: Utils.preferredLabelWidth
        Layout.alignment: Qt.AlignLeft
    }
}
