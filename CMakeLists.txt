# SPDX-FileCopyrightText: 2024 Aleix Pol Gonzalez <aleix.pol_gonzalez@mercedes-benz.com>
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(kwin-zones VERSION 0.1)

set(QT_MIN_VERSION "6.4")
set(KF_MIN_VERSION "6.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

option(ONLY_CLIENT_BUILD "Build only KWinZones QML Client Plugin" OFF)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)

include(FeatureSummary)
include(GenerateExportHeader)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEClangFormat)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(ECMQmlModule)
include(ECMSetupQtPluginMacroNames)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Gui
    GuiPrivate
    Widgets
    WaylandClient
    WaylandClientPrivate
)

if (NOT ONLY_CLIENT_BUILD)
    find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Config)
    find_package(KWin 6.0 REQUIRED)
else()
    find_package(WaylandScanner REQUIRED)
endif()

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

add_subdirectory(src)

if (NOT ONLY_CLIENT_BUILD)
    # add clang-format target for all our real source files
    file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
    kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
