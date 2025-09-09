/*
    SPDX-FileCopyrightText: 2024 Aleix Pol Gonzalez <aleix.pol_gonzalez@mercedes-benz.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QRect>
#include <QSet>

#include <plugin.h>
#include "qwayland-server-ext-zones-v1.h"

namespace KWin
{
class ExtZoneManagerV1Interface;
class ExtZoneItemV1Interface;

class Zones : public Plugin
{
    Q_OBJECT
public:
    explicit Zones();

private:
    ExtZoneManagerV1Interface *const m_extZones;
};

class ExtZoneV1Interface : public QObject, public QtWaylandServer::ext_zone_v1
{
    Q_OBJECT

public:
    ExtZoneV1Interface(const QRect& area, const QString& handle)
        : m_area(area)
          , m_handle(handle)
    {
        Q_ASSERT(!m_handle.isEmpty());
        setObjectName(handle);
    }

    void ext_zone_v1_bind_resource(Resource* resource) override
    {
        const QSizeF size = m_area.size();
        send_size(resource->handle, size.width(), size.height());
        send_handle(resource->handle, m_handle);
        send_done(resource->handle);
    }

    void ext_zone_v1_destroy(Resource* resource) override
    {
        wl_resource_destroy(resource->handle);
    }

    void ext_zone_v1_add_item(Resource*/*resource*/, struct ::wl_resource* item) override
    {
        setThisZone(item);
    }

    void ext_zone_v1_remove_item(Resource* resource, struct ::wl_resource* item) override;

    void setArea(const QRect& area);

private:
    void setThisZone(wl_resource* item);

    friend class ExtZoneItemV1Interface;
    QSet<ExtZoneItemV1Interface*> m_items;
    QRect m_area;
    const QString m_handle;
};

} // namespace KWin
