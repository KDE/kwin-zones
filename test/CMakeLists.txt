# SPDX-FileCopyrightText: 2024 Aleix Pol Gonzalez <aleix.pol_gonzalez@mercedes-benz.com>
# SPDX-License-Identifier: BSD-3-Clause

ecm_add_qml_module(QtZonesQuick
                  GENERATE_PLUGIN_SOURCE
                  URI "org.kde.zones"
                  VERSION 1.0
                  SOURCES zonewindow.cpp zonemanager.cpp)
target_link_libraries(QtZonesQuick PRIVATE Qt::Qml Qt::WaylandClient Qt::GuiPrivate Wayland::Client)

qt6_generate_wayland_protocol_client_sources(QtZonesQuick FILES
    ${WaylandProtocols_DATADIR}/stable/xdg-shell/xdg-shell.xml
    ${CMAKE_SOURCE_DIR}/src/ext-zones-v1.xml
)

ecm_finalize_qml_module(QtZonesQuick DESTINATION ${KDE_INSTALL_QMLDIR})
