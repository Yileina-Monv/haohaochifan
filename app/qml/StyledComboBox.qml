import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ComboBox {
    id: control

    Layout.preferredHeight: theme.touchTarget
    font.pixelSize: theme.bodySize
    leftPadding: width > 0 && width < 96 ? 10 : 14
    rightPadding: width > 0 && width < 96 ? 28 : 38

    Theme {
        id: theme
    }

    contentItem: Text {
        leftPadding: control.leftPadding
        rightPadding: control.rightPadding
        text: control.displayText
        font: control.font
        color: control.enabled ? theme.bodyText : theme.mutedText
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: Item {
        x: control.width - width - (control.width > 0 && control.width < 96 ? 6 : 10)
        y: 0
        width: control.width > 0 && control.width < 96 ? 22 : 28
        height: control.height

        Rectangle {
            width: 8
            height: 2
            radius: 1
            color: theme.primary
            anchors.centerIn: parent
            anchors.horizontalCenterOffset: -3
            rotation: 45
        }

        Rectangle {
            width: 8
            height: 2
            radius: 1
            color: theme.primary
            anchors.centerIn: parent
            anchors.horizontalCenterOffset: 3
            rotation: -45
        }
    }

    background: Rectangle {
        radius: theme.radiusControl
        color: control.enabled ? theme.surface : theme.surfaceDisabled
        border.color: control.activeFocus || control.pressed ? theme.primary : theme.border
        border.width: control.activeFocus || control.pressed ? 2 : 1
    }

    delegate: ItemDelegate {
        id: delegateRoot

        width: control.width
        height: theme.touchTarget
        highlighted: control.highlightedIndex === index

        contentItem: Text {
            text: control.textAt(index)
            color: delegateRoot.highlighted ? theme.surface : theme.bodyText
            font.pixelSize: theme.bodySize
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        background: Rectangle {
            radius: 8
            color: delegateRoot.highlighted ? theme.primary : "transparent"
        }
    }

    popup: Popup {
        y: control.height + 4
        width: control.width
        implicitHeight: Math.min(contentItem.implicitHeight + 8, 280)
        padding: 4

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
        }

        background: Rectangle {
            radius: theme.radiusSmall
            color: theme.surface
            border.color: theme.border
            border.width: 1
        }
    }
}
