import QtQuick
import QtQuick.Controls
import org.kde.zones

ApplicationWindow {
    id: main
    visible: true
    width: 200
    height: 200

    Repeater {
        id: rep
        model: 10
        delegate: Item {
            Window {
                id: movingWindow
                title: "Test Window " + modelData
                visible: true
                width: 100
                height: 100
                ZoneItemAttached.zone: main.ZoneItemAttached.zone
                Timer {
                    property real angle: 0
                    property int centerX: main.ZoneItemAttached.item.position.x + main.width / 2
                    property int centerY: main.ZoneItemAttached.item.position.y + main.height / 2
                    property int radius: (centerX/2 * modelData) / rep.model

                    interval: 1000/60 // 60fps
                    repeat: true
                    running: movingWindow.ZoneItemAttached.zone !== undefined
                    onTriggered: {
                        angle += 0.01
                        if (angle > Math.PI * 2) {
                            angle -= Math.PI * 2
                        }
                        const point = Qt.point(centerX + radius * Math.cos(angle) - movingWindow.width / 2,
                                            centerY + radius * Math.sin(angle) - movingWindow.height / 2);
                        movingWindow.ZoneItemAttached.requestPosition(point)
                        movingWindow.ZoneItemAttached.item.layerIndex = Math.sin((angle * modelData) / rep.model) * rep.model
                    }
                }
            }
        }
    }
}
