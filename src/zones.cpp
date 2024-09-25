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

#include <KConfig>
#include <KConfigGroup>

namespace KWin
{
static const int s_version = 1;
class ExtZoneV1Interface;

class ExtZoneItemV1Interface : public QObject, public QtWaylandServer::ext_zone_item_v1
{
public:
    ExtZoneItemV1Interface(XdgToplevelInterface *toplevel)
        : m_toplevel(toplevel)
    {}

    ~ExtZoneItemV1Interface();

    static ExtZoneItemV1Interface *get(::wl_resource *resource)
    {
        return resource_cast<ExtZoneItemV1Interface *>(resource);
    }

    Window *window() const {
        if (!m_toplevel) {
            return nullptr;
        }
        return waylandServer()->findWindow(m_toplevel->surface());
    }

    int layer_index = 0;
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

    void ext_zone_v1_bind_resource(Resource *resource) override
    {
        const QSizeF size = m_area.size();
        send_size(resource->handle, size.width(), size.height());
        send_handle(resource->handle, m_handle);
        send_done(resource->handle);
    }

    void ext_zone_v1_get_position(Resource *resource, struct ::wl_resource *item) override
    {
        ExtZoneItemV1Interface *zoneItem = ExtZoneItemV1Interface::get(item);
        if (zoneItem->m_zone != this || !zoneItem->m_zone) {
            qDebug() << "Could not find item in zone" << zoneItem->m_zone;
            send_position_failed(resource->handle, item);
            return;
        }

        auto w = waylandServer()->findWindow(zoneItem->m_toplevel->surface());
        if (!w) {
            qDebug() << "Could not find item" << zoneItem->m_toplevel << zoneItem << zoneItem->m_toplevel->surface();
            send_position_failed(resource->handle, item);
            return;
        }
        const QPointF pos = w->frameGeometry().topLeft() - m_area.topLeft();
        send_position(resource->handle, item, pos.x(), pos.y());
    }
    void ext_zone_v1_set_position(Resource *resource, struct ::wl_resource *item, int32_t x, int32_t y) override
    {
        ExtZoneItemV1Interface *zoneWindow = ExtZoneItemV1Interface::get(item);
        if (!zoneWindow) {
            // Can happen when shutting down
            return;
        }
        if (zoneWindow->m_zone != this || !zoneWindow->m_zone) {
            qDebug() << "Different zone" << zoneWindow->m_zone << this;
            send_position_failed(resource->handle, item);
            return;
        }

        auto w = waylandServer()->findWindow(zoneWindow->m_toplevel->surface());
        if (!w) {
            qDebug() << "Could not find surface" << zoneWindow->m_toplevel;
            send_position_failed(resource->handle, item);
            return;
        }
        const QPoint pos = QPoint(x, y) + m_area.topLeft();
        if (!m_area.contains(pos)) {
            qDebug() << "could not position toplevel" << m_area << pos << m_area;
            send_position_failed(resource->handle, item);
            return;
        }
        w->move(pos);
    }
    void ext_zone_v1_set_layer(Resource *resource, struct ::wl_resource *item, int32_t layer_index) override {
        ExtZoneItemV1Interface *zoneWindow = ExtZoneItemV1Interface::get(item);
        if (zoneWindow->m_zone != this || !zoneWindow->m_zone) {
            qDebug() << "Mismatched zone" << zoneWindow->m_zone;
            send_position_failed(resource->handle, item);
            return;
        }

        zoneWindow->layer_index = layer_index;
        StackingUpdatesBlocker blocker(workspace());
        auto current = zoneWindow->window();
        for (auto item : m_items) {
            if (zoneWindow->layer_index < layer_index) {
                workspace()->constrain(current, item->window());
                workspace()->unconstrain(item->window(), current);
            } else if (zoneWindow->layer_index > layer_index) {
                workspace()->constrain(item->window(), current);
                workspace()->unconstrain(current, item->window());
            } else if (zoneWindow->layer_index == layer_index) {
                workspace()->unconstrain(current, item->window());
                workspace()->unconstrain(item->window(), current);
            }
        }
    }

    void ext_zone_v1_destroy(Resource *resource) override {
        wl_resource_destroy(resource->handle);
    }
    void ext_zone_v1_add_item(Resource */*resource*/, struct ::wl_resource *item) override {
        setThisZone(item);
    }
    void ext_zone_v1_remove_item(Resource *resource, struct ::wl_resource *item) override {
        auto w = ExtZoneItemV1Interface::get(item);
        if (w) {
            w->m_zone = nullptr;
        }
        send_item_left(resource->handle, item);
        StackingUpdatesBlocker blocker(workspace());
        for (auto item : m_items) {
            workspace()->unconstrain(w->window(), item->window());
            workspace()->unconstrain(item->window(), w->window());
        }
        m_items.remove(w);
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
    void setThisZone(wl_resource *item) {
        auto w = ExtZoneItemV1Interface::get(item);
        Q_ASSERT(w);
        if (w->m_zone && w->m_zone != this) {
            for (auto resource : w->m_zone->resourceMap()) {
                w->m_zone->send_item_left(resource->handle, item);
            }
        }
        w->m_zone = this;
        m_items.insert(w);
        for (auto resource : resourceMap()) {
            w->m_zone->send_item_left(resource->handle, item);
        }
    }
    void addToZone(Window *w, int layer);

    friend class ExtZoneItemV1Interface;
    QSet<ExtZoneItemV1Interface *> m_items;
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

    void ext_zone_manager_v1_get_zone_item(Resource *resource, uint32_t id, struct ::wl_resource *toplevelResource) override
    {
        XdgToplevelInterface *toplevel = XdgToplevelInterface::get(toplevelResource);
        if (!toplevel) {
            wl_resource_post_error(resource->handle, QtWaylandServer::ext_zone_v1::error_invalid, "xdg-toplevel object not found");
            return;
        }

        auto it = m_zoneWindows.constFind(toplevel);
        if (it == m_zoneWindows.constEnd()) {
            auto zoneWindow = new ExtZoneItemV1Interface(toplevel);
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
            connect(output, &Output::aboutToTurnOff, this, [this, output] {
                delete m_zones.take(output->name());
            });
            it = m_zones.insert(handle, zone);
        }
        (*it)->add(resource->client(), id, s_version);
    }

    void ext_zone_manager_v1_get_zone_from_handle(Resource *resource, uint32_t id, const QString & handle) override
    {
        auto it = m_zones.constFind(handle);
        static const KSharedConfig::Ptr cfgZones = KSharedConfig::openConfig("kwinzonesrc");
        if (it == m_zones.constEnd()) {
            KConfigGroup grp = cfgZones->group("Zones");
            static auto watcher = KConfigWatcher::create(cfgZones);
            auto zone = new ExtZoneV1Interface(grp.readEntry(handle, QRect()), handle);
            connect(watcher.get(), &KConfigWatcher::configChanged, zone, [handle, zone] (const KConfigGroup &group, const QByteArrayList &names) {
                if (!names.contains(handle)) {
                    return;
                }
                zone->setArea(group.readEntry(handle, QRect()));
            });
            it = m_zones.insert(handle, zone);
        }
        (*it)->add(resource->client(), id, s_version);
    }

    QHash<QString, ExtZoneV1Interface *> m_zones;
    QHash<XdgToplevelInterface *, ExtZoneItemV1Interface *> m_zoneWindows;
};

Zones::Zones()
    : m_extZones(new ExtZoneManagerV1Interface(effects->waylandDisplay(), this))
{
}

ExtZoneItemV1Interface::~ExtZoneItemV1Interface() {
    if (m_zone) {
        m_zone->m_items.remove(this);
    }
}

}
