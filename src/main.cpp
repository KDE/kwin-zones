/*
    SPDX-FileCopyrightText: 2024 Aleix Pol Gonzalez <aleix.pol_gonzalez@mercedes-benz.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <plugin.h>
#include <main.h>
#include "zones.h"

using namespace KWin;

class KWIN_EXPORT KWinZonesFactory : public PluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginFactory_iid FILE "metadata.json")
    Q_INTERFACES(KWin::PluginFactory)

public:
    explicit KWinZonesFactory() = default;

    std::unique_ptr<KWin::Plugin> create() const override {
#ifdef KWIN_ZONES_SUPPORT_OPERATION_MODES
        switch (kwinApp()->operationMode()) {
        case Application::OperationModeX11:
            return nullptr;
        case Application::OperationModeXwayland:
        case Application::OperationModeWaylandOnly:
            return std::make_unique<Zones>();
        default:
            return nullptr;
        }
#else
        return std::make_unique<Zones>();
#endif
    }
};

#include "main.moc"
