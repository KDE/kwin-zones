// SPDX-FileCopyrightText: 2024 Aleix Pol Gonzalez <aleix.pol_gonzalez@mercedes-benz.com>
// SPDX-License-Identifier: MIT

import QtQuick
import QtQuick.Controls
import org.kde.zones

Window {
   title: "titaniumi"
   visible: true
   width: 1920
   height: 720
   transientParent: null
   ZoneItemAttached.zone: _loader.item ? _loader.item.ZoneItemAttached.zone : null
   ZoneItemAttached.item.layerIndex: 120
   ZoneItemAttached.item.requestedPosition: Qt.point(0, 0)

   Rectangle {
       color: "green"
       anchors.fill: parent
   }


    Loader {
        id: _loader
        active: false
        sourceComponent: Window {
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
        Component.onCompleted: {
            Qt.callLater(function() { _loader.active = true })
            // _loader.active = true;
        }
    }
}
