import QtQuick
import QtQuick.Layouts

Rectangle {
    id: card

    property color fill: theme.surface
    property color stroke: theme.border
    property int padding: 16
    default property alias content: contentHost.data

    Layout.fillWidth: true
    Layout.preferredHeight: implicitHeight
    implicitHeight: contentHost.childrenRect.height > 0
                    ? contentHost.childrenRect.y + contentHost.childrenRect.height + padding * 2
                    : 0
    radius: theme.radiusCard
    color: fill
    border.color: stroke
    border.width: 1

    Theme {
        id: theme
    }

    Item {
        id: contentHost
        x: card.padding
        y: card.padding
        width: Math.max(card.width - card.padding * 2, 0)
        height: childrenRect.height
    }
}
