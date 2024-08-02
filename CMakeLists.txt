cmake_minimum_required(VERSION 3.16)
project(kwin-zones VERSION 0.1)

set(QT_MIN_VERSION "6.4")
set(KF_MIN_VERSION "6.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)

include(FeatureSummary)
include(GenerateExportHeader)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(ECMQmlModule)
include(ECMSetupQtPluginMacroNames)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    WaylandClient
)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Config)
find_package(WaylandProtocols REQUIRED)
find_package(Wayland 1.3 REQUIRED COMPONENTS Client)

find_package(QtWaylandScanner REQUIRED)
set_package_properties(QtWaylandScanner PROPERTIES
    TYPE REQUIRED
    PURPOSE "Required for building this effect"
)

#add_definitions(-DQT_NO_KEYWORDS)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(KWin 6.0 REQUIRED)

add_subdirectory(src)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
