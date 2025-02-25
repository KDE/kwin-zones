// SPDX-FileCopyrightText: 2024 Aleix Pol Gonzalez <aleix.pol_gonzalez@mercedes-benz.com>
// SPDX-License-Identifier: MIT

import QtQuick
import QtQuick.Controls
import org.kde.zones

Item {
    visible: false
    ApplicationWindow {
        id: main
        visible: true
        transientParent: null
        width: 7680
        height: 1260
        title: "Background"
        ZoneItemAttached.item.layerIndex: 1000
        ZoneItemAttached.item.requestedPosition: Qt.point(0, 0)

        Rectangle {
            anchors.fill: parent
            color: "purple"
        }
    }

    Window {
        title: "titaniumi"
        visible: true
        width: 1920
        height: 720
        transientParent: null
        ZoneItemAttached.zone: main.ZoneItemAttached.zone
        ZoneItemAttached.item.layerIndex: 123
        ZoneItemAttached.item.requestedPosition: Qt.point(0, 0)

        Rectangle {
            color: "green"
            anchors.fill: parent
        }
    }

    Window {
        title: "ui3"
        visible: true
        width: 3000
        height: 500
        transientParent: null
        ZoneItemAttached.zone: main.ZoneItemAttached.zone
        ZoneItemAttached.item.layerIndex: 100
        ZoneItemAttached.item.requestedPosition: Qt.point(1920, 0)

        Rectangle {
            color: "red"
            anchors.fill: parent
        }
    }

    Window {
        title: "ui3-codriver"
        visible: true
        width: 2000
        height: 500
        transientParent: null
        ZoneItemAttached.zone: main.ZoneItemAttached.zone
        ZoneItemAttached.item.layerIndex: 100
        ZoneItemAttached.item.requestedPosition: Qt.point(1920 + 3000, 0)

        Rectangle {
            color: "yellow"
            anchors.fill: parent
        }
    }
}
