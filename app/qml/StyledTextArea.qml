import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

TextArea {
    id: control

    property string hintText: ""

    font.pixelSize: theme.bodySize
    color: theme.bodyText
    placeholderText: activeFocus || text.length > 0 ? "" : hintText
    placeholderTextColor: theme.mutedText
    wrapMode: TextEdit.Wrap
    clip: true
    leftPadding: 14
    rightPadding: 14
    topPadding: 12
    bottomPadding: 12

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
