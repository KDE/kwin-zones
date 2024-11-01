add_subdirectory(client)

kcoreaddons_add_plugin(KWinZones INSTALL_NAMESPACE "kwin/plugins")
target_sources(KWinZones PRIVATE main.cpp zones.cpp)


kconfig_add_kcfg_files(KWinZones zonessettings.kcfgc)
ecm_add_qtwayland_server_protocol(KWinZones
    PROTOCOL ext-zones-v1.xml
    BASENAME ext-zones-v1
)

target_link_libraries(KWinZones KWin::kwin KF6::ConfigGui)
