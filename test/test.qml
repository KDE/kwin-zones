import QtQuick
import QtQuick.Controls
import org.kde.zones

ApplicationWindow {
    id: main
    visible: true
    width: 200
    height: 200

    Window {
        id: movingWindow
        visible: true
        width: 100
        height: 100
        onVisibleChanged: if (visible) {
            ZoneWindowAttached.zone = main.ZoneWindowAttached.zone
            console.log("zone!!", ZoneWindowAttached.zone.size, ttt.centerX)
            ttt.centerX = Qt.binding(() => ZoneWindowAttached.zone.size.width / 2)
            ttt.centerY = Qt.binding(() => ZoneWindowAttached.zone.size.height / 2)
        }
        Timer {
            id: ttt
            property real angle: 0
            property int centerX: 0//main.ZoneWindowAttached.window.position.x + main.width / 2
            property int centerY: 0//main.ZoneWindowAttached.window.position.y + main.height / 2
            property int radius: centerX/2

            interval: 1000/60
            repeat: true
            running: true
            onTriggered: {
                angle += 0.01
                if (angle > Math.PI * 2) {
                    angle -= Math.PI * 2
                }
                const point = Qt.point(centerX + radius * Math.cos(angle) - movingWindow.width / 2,
                                       centerY + radius * Math.sin(angle) - movingWindow.height / 2);
                movingWindow.ZoneWindowAttached.requestPosition(point)
            }
        }
    }
}
