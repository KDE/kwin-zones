add_subdirectory(client)

kcoreaddons_add_plugin(KWinZones INSTALL_NAMESPACE "kwin/plugins")
target_sources(KWinZones PRIVATE main.cpp zones.cpp)


kconfig_add_kcfg_files(KWinZones zonessettings.kcfgc)
ecm_add_qtwayland_server_protocol(KWinZones
    PROTOCOL ext-zones-v1.xml
    BASENAME ext-zones-v1
)
ecm_qt_declare_logging_category(KWinZones
    HEADER kwinzonescompositorlogging.h
    IDENTIFIER KWINZONES
    CATEGORY_NAME kwinzones.compositor
    DEFAULT_SEVERITY Info
)

target_link_libraries(KWinZones KWin::kwin KF6::ConfigGui)
