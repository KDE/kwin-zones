# SPDX-FileCopyrightText: 2024 Aleix Pol Gonzalez <aleix.pol_gonzalez@mercedes-benz.com>
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(client)

if (NOT ONLY_CLIENT_BUILD)
    kcoreaddons_add_plugin(KWinZones INSTALL_NAMESPACE "kwin/plugins")
    target_sources(KWinZones PRIVATE main.cpp zones.cpp)

    kconfig_add_kcfg_files(KWinZones zonessettings.kcfgc)
    ecm_add_qtwayland_server_protocol(KWinZones
        PROTOCOL ext-zones-v1.xml
        BASENAME ext-zones-v1
    )
    ecm_add_qtwayland_server_protocol(KWinZones
        PROTOCOL ${WaylandProtocols_DATADIR}/stable/xdg-shell/xdg-shell.xml
        BASENAME xdg-shell
    )
    ecm_qt_declare_logging_category(KWinZones
        HEADER kwinzonescompositorlogging.h
        IDENTIFIER KWINZONES
        CATEGORY_NAME kwinzones.compositor
        DEFAULT_SEVERITY Info
    )

    target_link_libraries(KWinZones KWin::kwin KF6::ConfigGui)
endif()