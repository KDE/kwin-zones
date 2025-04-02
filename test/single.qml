// SPDX-FileCopyrightText: 2024 Aleix Pol Gonzalez <aleix.pol_gonzalez@mercedes-benz.com>
// SPDX-License-Identifier: MIT

// Useful to test z-ordering
// To use, run as: qml single.qml -- layerIndex labelText
// e.g.: qml single.qml -- 2 UI3

import QtQuick
import QtQuick.Controls
import org.kde.zones

ApplicationWindow {
    id: main
    visible: true
    width: 200
    height: 200
    title: Qt.application.arguments[Qt.application.arguments.length - 1]
    ZoneItemAttached.item.layerIndex: parseInt(Qt.application.arguments[Qt.application.arguments.length - 2])


    Rectangle {
        anchors.fill: parent
        color: mainArea.pressed ? "blue" : "yellow"

        Text {
            anchors.fill: parent
            text: main.title + "\n" + main.ZoneItemAttached.zone.handle
        }

        MouseArea {
            id: mainArea
            anchors.fill: parent
        }
    }
}
