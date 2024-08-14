# SPDX-FileCopyrightText: 2024 Aleix Pol Gonzalez <aleix.pol_gonzalez@mercedes-benz.com>
# SPDX-License-Identifier: BSD-3-Clause

qt6_add_qml_module(QtZonesQuick
                  URI "org.kde.zones"
                  OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/org/kde/zones
                  SOURCES zoneitemattached.cpp zonemanager.cpp)
set_target_properties(QtZonesQuickplugin PROPERTIES PREFIX "lib")
target_link_libraries(QtZonesQuick PRIVATE Qt::Qml Qt::WaylandClient Qt::GuiPrivate Wayland::Client Qt::WaylandClientPrivate)

qt6_generate_wayland_protocol_client_sources(QtZonesQuick FILES
    ${Wayland_DATADIR}/wayland.xml
    ${WaylandProtocols_DATADIR}/stable/xdg-shell/xdg-shell.xml
    ${CMAKE_SOURCE_DIR}/src/ext-zones-v1.xml
)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/org DESTINATION ${QT6_INSTALL_QML})
