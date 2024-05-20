/*
    SPDX-FileCopyrightText: 2024 Aleix Pol Gonzalez <aleix.pol_gonzalez@mercedes-benz.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "zones.h"
#include "qwayland-server-ext-zones-v1.h"

#include <wayland/clientconnection.h>
#include <wayland/display.h>
#include <wayland/output.h>
#include <wayland/seat.h>
#include <wayland/surface.h>
#include <wayland/xdgshell.h>
#include <utils/resource.h>
#include "workspace.h"
#include "wayland_server.h"
#include "window.h"

namespace KWin
{
static const int s_version = 1;
class ExtZoneV1Interface;

class ExtZoneWindowV1Interface : public QObject, public QtWaylandServer::ext_zone_window_v1
{
public:
    ExtZoneWindowV1Interface(XdgToplevelInterface *toplevel)
        : m_toplevel(toplevel)
    {}

    static ExtZoneWindowV1Interface *get(::wl_resource *resource)
    {
        return resource_cast<ExtZoneWindowV1Interface *>(resource);
    }

    XdgToplevelInterface *const m_toplevel;
    ExtZoneV1Interface* m_zone = nullptr;
};

class ExtZoneV1Interface : public QObject, public QtWaylandServer::ext_zone_v1
{
public:
    ExtZoneV1Interface(const QRect &area, const QString &handle)
        : m_area(area)
        , m_handle(handle)
    {
        Q_ASSERT(!m_handle.isEmpty());
    }

    void ext_zone_v1_bind_resource(Resource * resource) override
    {
        const QSizeF size = m_area.size();
        send_size(resource->handle, size.width(), size.height());
        send_handle(resource->handle, m_handle);
        send_done(resource->handle);
    }

    void ext_zone_v1_get_position(Resource * resource, struct ::wl_resource *window) override
    {
        ExtZoneWindowV1Interface *zoneWindow = ExtZoneWindowV1Interface::get(window);
        if (zoneWindow->m_zone != this || !zoneWindow->m_zone) {
            qDebug() << "e" << zoneWindow->m_zone;
            send_position_failed(resource->handle, window);
            return;
        }

        auto w = effects->findWindow(zoneWindow->m_toplevel->surface());
        if (!w) {
            qDebug() << "Could not find window" << zoneWindow->m_toplevel << zoneWindow << zoneWindow->m_toplevel->surface();
            send_position_failed(resource->handle, window);
            return;
        }
        const QPointF pos = w->frameGeometry().topLeft() - m_area.topLeft();
        send_position(resource->handle, pos.x(), pos.y());
    }
    void ext_zone_v1_set_position(Resource *resource, struct ::wl_resource *window, int32_t x, int32_t y) override
    {
        ExtZoneWindowV1Interface *zoneWindow = ExtZoneWindowV1Interface::get(window);
        if (!zoneWindow) {
            // Can happen when shutting down
            return;
        }
        if (zoneWindow->m_zone != this || !zoneWindow->m_zone) {
            qDebug() << "different zone" << zoneWindow->m_zone << this;
            send_position_failed(resource->handle, window);
            return;
        }

        auto w = waylandServer()->findWindow(zoneWindow->m_toplevel->surface());
        if (!w) {
            qDebug() << "Could nt find surface" << zoneWindow->m_toplevel;
            send_position_failed(resource->handle, window);
            return;
        }
        const QPoint pos = QPoint(x, y) + m_area.topLeft();
        if (!m_area.contains(pos)) {
            qDebug() << "could not position toplevel" << m_area << pos << m_area;
            send_position_failed(resource->handle, window);
            return;
        }
        w->move(pos);
    }
    void ext_zone_v1_set_layer(Resource *resource, struct ::wl_resource *window, int32_t layer_index) override {
        ExtZoneWindowV1Interface *zoneWindow = ExtZoneWindowV1Interface::get(window);
        if (zoneWindow->m_zone != this || !zoneWindow->m_zone) {
            qDebug() << "c" << zoneWindow->m_zone;
            send_position_failed(resource->handle, window);
            return;
        }

        auto w = waylandServer()->findWindow(zoneWindow->m_toplevel->surface());
        if (!w) {
            qDebug() << "d" << zoneWindow;
            send_position_failed(resource->handle, window);
            return;
        }
        // TODO: Make it per-zone?
        w->setStackingOrder(layer_index);
    }

    void ext_zone_v1_destroy(Resource *resource) override {
        wl_resource_destroy(resource->handle);
    }
    void ext_zone_v1_add_window(Resource *resource, struct ::wl_resource *window) override {
        auto w = ExtZoneWindowV1Interface::get(window);
        if (w->m_zone && w->m_zone != this) {
            w->m_zone->send_window_left(resource->handle, window);
        }
        w->m_zone = this;
        send_window_entered(resource->handle, window);
    }
    void ext_zone_v1_remove_window(Resource *resource, struct ::wl_resource *window) override {
        auto w = ExtZoneWindowV1Interface::get(window);
        if (w) {
            w->m_zone = nullptr;
        }
        send_window_left(resource->handle, window);
    }

    void setArea(const QRect &area) {
        if (m_area == area) {
            return;
        }

        const bool sizeChange = m_area.size() != area.size();
        m_area = area;
        if (sizeChange) {
            const auto clientResources = resourceMap();
            for (auto r : clientResources) {
                send_size(r->handle, m_area.width(), m_area.height());
            }
        }

    }

private:
    QRect m_area;
    const QString m_handle;
};

class ExtZoneManagerV1Interface : public QObject, public QtWaylandServer::ext_zone_manager_v1
{
public:
    ExtZoneManagerV1Interface(Display *display, QObject *parent)
        : QObject(parent)
        , ext_zone_manager_v1(*display, s_version)
    {
    }

    void ext_zone_manager_v1_destroy(Resource *resource) override {
        wl_resource_destroy(resource->handle);
    }

    void ext_zone_manager_v1_get_zone_window(Resource *resource, uint32_t id, struct ::wl_resource *toplevelResource) override
    {
        XdgToplevelInterface *toplevel = XdgToplevelInterface::get(toplevelResource);
        if (!toplevel) {
            wl_resource_post_error(resource->handle, QtWaylandServer::ext_zone_v1::error_invalid, "xdg-toplevel object not found");
            return;
        }

        auto it = m_zoneWindows.constFind(toplevel);
        if (it == m_zoneWindows.constEnd()) {
            auto zoneWindow = new ExtZoneWindowV1Interface(toplevel);
            it = m_zoneWindows.insert(toplevel,  zoneWindow);
            connect(toplevel, &XdgToplevelInterface::aboutToBeDestroyed, zoneWindow, [this, toplevel] {
                auto zoneWindow = m_zoneWindows.take(toplevel);
                delete zoneWindow;
            });
        }
        (*it)->add(resource->client(), id, s_version);
    }

    void ext_zone_manager_v1_get_zone(Resource *resource, uint32_t id, struct ::wl_resource *outputResource) override
    {
        OutputInterface *outputIface = OutputInterface::get(outputResource);
        if (!outputIface) {
            wl_resource_post_error(resource->handle, QtWaylandServer::ext_zone_v1::error_invalid, "output object not found");
            return;
        }

        auto output = outputIface->handle();
        const auto handle = output->name();
        auto it = m_zones.constFind(handle);
        if (it == m_zones.constEnd()) {
            auto zone = new ExtZoneV1Interface(output->geometry(), handle);
            connect(output, &Output::geometryChanged, zone, [zone, output] {
                zone->setArea(output->geometry());
            });
            it = m_zones.insert(handle, zone);
        }
        (*it)->add(resource->client(), id, s_version);
    }

    void ext_zone_manager_v1_get_zone_from_handle(QtWaylandServer::ext_zone_manager_v1::Resource * resource, uint32_t id, const QString & handle) override
    {
        auto it = m_zones.constFind(handle);
        if (it == m_zones.constEnd()) {
            wl_resource_post_error(resource->handle, QtWaylandServer::ext_zone_v1::error_invalid, "zone handle not found");
        }
        (*it)->add(resource->client(), id, s_version);
    }

    QHash<QString, ExtZoneV1Interface *> m_zones;
    QHash<XdgToplevelInterface *, ExtZoneWindowV1Interface *> m_zoneWindows;
};

Zones::Zones()
    : m_extZones(new ExtZoneManagerV1Interface(effects->waylandDisplay(), this))
{

}

}
