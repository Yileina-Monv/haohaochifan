import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Button {
    id: control

    property bool primary: false
    property bool selected: false
    property bool compact: false

    Layout.preferredHeight: compact ? 40 : theme.touchTarget
    flat: true
    padding: 0

    Theme {
        id: theme
    }

    background: Rectangle {
        radius: theme.radiusControl
        color: !control.enabled
               ? theme.surfaceDisabled
               : (control.primary || control.selected)
                 ? (control.down ? theme.primaryPressed
                    : (control.hovered ? theme.primaryHover : theme.primary))
                 : (control.down ? theme.surfacePressed
                    : (control.hovered ? theme.surfaceHover : theme.surface))
        border.color: (control.primary || control.selected) ? theme.primaryPressed : theme.border
        border.width: 1
    }

    contentItem: Label {
        text: control.text
        color: !control.enabled
               ? theme.mutedText
               : (control.primary || control.selected) ? theme.surface : "#2f372c"
        font.pixelSize: control.compact ? 14 : theme.bodySize
        font.bold: control.primary || control.selected
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        maximumLineCount: 1
    }
}
