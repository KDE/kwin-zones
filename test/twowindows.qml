import QtQuick
import QtQuick.Controls
import org.kde.zones

ApplicationWindow {
    id: main
    visible: true
    width: 200
    height: 200
    title: "Bottom"
    ZoneItemAttached.item.layerIndex: 1


    Rectangle {
        anchors.fill: parent
        color: mainArea.pressed ? "blue" : "yellow"

        MouseArea {
            id: mainArea
            anchors.fill: parent
        }
    }

    Window {
        id: movingWindow
        title: "Top"
        visible: true
        width: 500
        height: 500
        flags: Qt.WA_TranslucentBackground
        color: "transparent"
        ZoneItemAttached.zone: main.ZoneItemAttached.zone
        ZoneItemAttached.item.layerIndex: 100

        Rectangle {
            color: topArea.pressed ? "red" : "green"
            height: parent.height / 3
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
        }

        Rectangle {
            color: topArea.pressed ? "red" : "green"
            height: parent.height / 3
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
        }

        MouseArea {
            id: topArea
            anchors.fill: parent
        }
    }
}
