import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

TextField {
    id: control

    property string hintText: ""

    Layout.preferredHeight: theme.touchTarget
    font.pixelSize: theme.bodySize
    color: theme.bodyText
    placeholderText: activeFocus || text.length > 0 ? "" : hintText
    placeholderTextColor: theme.mutedText
    leftPadding: 14
    rightPadding: 14

    Theme {
        id: theme
    }

    background: Rectangle {
        radius: theme.radiusControl
        color: theme.surface
        border.color: control.activeFocus ? theme.primary : theme.border
        border.width: control.activeFocus ? 2 : 1
    }
}
