/*
    SPDX-FileCopyrightText: 2024 Aleix Pol Gonzalez <aleix.pol_gonzalez@mercedes-benz.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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

#include <kwinzonescompositorlogging.h>

namespace KWin
{
static const int s_version = 1;
class ExtZoneV1Interface;

class ExtZoneItemV1Interface : public QObject, public QtWaylandServer::ext_zone_item_v1
{
    Q_OBJECT
public:
    explicit ExtZoneItemV1Interface(XdgToplevelInterface *toplevel, struct ::wl_client *client, uint32_t id, int version)
        : ext_zone_item_v1(client, id, version)
        , m_toplevel(toplevel)
    {
        connect(window(), &Window::frameGeometryChanged, this, &ExtZoneItemV1Interface::refreshPosition);
    }

    ~ExtZoneItemV1Interface()
    {
        if (m_zone) {
            m_zone->m_items.remove(this);
        }
    }

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

    void constrainPosition(QRect &windowRect) const
    {
        if (windowRect.left() > m_zone->m_area.right()) {
            windowRect.moveLeft(m_zone->m_area.right() - windowRect.width());
        }
        if (windowRect.right() < m_zone->m_area.left()) {
            windowRect.moveLeft(m_zone->m_area.left());
        }
        if (windowRect.top() > m_zone->m_area.bottom()) {
            windowRect.moveTop(m_zone->m_area.bottom() - windowRect.height());
        }
        if (windowRect.bottom() < m_zone->m_area.top()) {
            windowRect.moveTop(m_zone->m_area.top());
        }
    }

    void ext_zone_item_v1_set_position(Resource *resource, int32_t x, int32_t y) override
    {
        auto w = window();
        if (!w || !m_zone) {
            qCDebug(KWINZONES) << "set_position: Could not find surface" << m_toplevel << m_zone;
            send_position_failed(resource->handle);
            return;
        }

        QRect windowRect = w->frameGeometry().toRect();
        windowRect.moveTopLeft(QPoint(x, y));
        constrainPosition(windowRect);
        const QPoint pos = windowRect.topLeft();

        w->setObjectName("kwinzones");
        if (auto s = w->surface()) {
            if (m_setPositionDelay) {
                disconnect(m_setPositionDelay);
            }

            m_setPositionDelay = connect(s, &SurfaceInterface::committed, this, [w, pos, handle = m_zone->m_handle] {
                qCDebug(KWINZONES) << "Setting position. title:" << w->caption() << "zone:" << handle << "position:" << pos << "geometry:" << w->frameGeometry();
                w->move(pos);
            }, Qt::SingleShotConnection);
        }
    }

    void ext_zone_item_v1_set_layer(Resource *resource, int32_t idx) override
    {
        Q_ASSERT(m_zone);
        auto current = window();
        if (!current || !m_zone) {
            qCDebug(KWINZONES) << "set_layer: Could not find surface" << m_toplevel << m_zone;
            send_position_failed(resource->handle);
            return;
        }
        current->setObjectName("kwinzones");

        layer_index = idx;
        StackingUpdatesBlocker blocker(workspace());
        for (auto item : m_zone->m_items) {
            if (current == item->window()) {
                continue;
            }
            if (item->layer_index < layer_index) {
                workspace()->unconstrain(item->window(), current);
                workspace()->constrain(current, item->window());
            } else if (item->layer_index > layer_index) {
                workspace()->unconstrain(current, item->window());
                workspace()->constrain(item->window(), current);
            } else if (item->layer_index == layer_index) {
                workspace()->unconstrain(current, item->window());
                workspace()->unconstrain(item->window(), current);
            }
        }
    }

    void refreshPosition()
    {
        if (!m_zone) {
            return;
        }
        auto w = window();
        if (!w) {
            qCWarning(KWINZONES) << "Could not refresh position, could not find the toplevel's window" << m_toplevel->title() << m_toplevel->appId();
            return;
        }
        const QMargins margins = w->frameMargins();
        if (margins != m_currentMargins) {
            m_currentMargins = margins;
            send_frame_extents(margins.top(), margins.bottom(), margins.left(), margins.right());
        }

        const QPointF pos = w->frameGeometry().topLeft() - m_zone->m_area.topLeft();
        send_position(pos.x(), pos.y());
    }

    int layer_index = 0;
    XdgToplevelInterface *const m_toplevel;
    ExtZoneV1Interface* m_zone = nullptr;
    QMargins m_currentMargins;
    QMetaObject::Connection m_setPositionDelay;
};


void ExtZoneV1Interface::ext_zone_v1_remove_item(Resource* resource, struct ::wl_resource* item)
{
    auto w = ExtZoneItemV1Interface::get(item);
    if (!w)
    {
        qCDebug(KWINZONES) << "Zone Item not found" << item;
        return;
    }
    w->m_zone = nullptr;
    send_item_left(resource->handle, item);
    StackingUpdatesBlocker blocker(workspace());
    for (auto item : m_items)
    {
        workspace()->unconstrain(w->window(), item->window());
        workspace()->unconstrain(item->window(), w->window());
    }
    m_items.remove(w);
}

void ExtZoneV1Interface::setArea(const QRect& area)
{
    if (m_area == area)
    {
        return;
    }

    const bool sizeChange = m_area.size() != area.size();
    m_area = area;
    if (sizeChange)
    {
        const auto clientResources = resourceMap();
        for (auto r : clientResources)
        {
            send_size(r->handle, m_area.width(), m_area.height());
        }
    }
}

void ExtZoneV1Interface::setThisZone(wl_resource* item)
{
    auto w = ExtZoneItemV1Interface::get(item);
    if (!w || w->m_zone == this)
    {
        qCDebug(KWINZONES) << "Skip setting zone" << w << this;
        return;
    }
    if (w->m_zone && w->m_zone != this)
    {
        for (auto resource : w->m_zone->resourceMap())
        {
            if (resource->client() == item->client)
            {
                w->m_zone->send_item_left(resource->handle, item);
            }
        }
    }
    w->m_zone = this;
    m_items.insert(w);
    for (auto resource : resourceMap())
    {
        if (resource->client() == item->client)
        {
            w->m_zone->send_item_entered(resource->handle, item);
        }
    }

    auto window = w->window();
    if (window)
    {
        w->m_currentMargins = window->frameMargins();
        w->send_frame_extents(w->m_currentMargins.top(), w->m_currentMargins.bottom(), w->m_currentMargins.left(), w->m_currentMargins.right());

        const QPointF pos = window->frameGeometry().topLeft() - m_area.topLeft();
        w->send_position(pos.x(), pos.y());
    }
}

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
        if (it != m_zoneWindows.constEnd()) {
            wl_resource_post_error(resource->handle, QtWaylandServer::ext_zone_v1::error_invalid, "zone item already created");
            return;
        }
        auto zoneWindow = new ExtZoneItemV1Interface(toplevel, resource->client(), id, s_version);
        m_zoneWindows.insert(toplevel,  zoneWindow);
        connect(toplevel, &XdgToplevelInterface::aboutToBeDestroyed, this, [this, toplevel] {
            delete m_zoneWindows.take(toplevel);
        });
    }

    void ext_zone_manager_v1_get_zone(Resource *resource, uint32_t id, struct ::wl_resource *outputResource) override
    {
        OutputInterface *outputIface = nullptr;

        if (outputResource) {
            outputIface = OutputInterface::get(outputResource);
        } else {
            QList<OutputInterface *> outputs = waylandServer()->display()->outputs();

            if (!outputs.isEmpty()) {
                outputIface = outputs[0];
            }
        }

        if (!outputIface) {
            wl_resource_post_error(resource->handle, QtWaylandServer::ext_zone_v1::error_invalid, "output object not found");
            return;
        }

        auto output = outputIface->handle();
        const auto handle = output->name();
        auto it = m_zones.constFind(handle);
        if (it == m_zones.constEnd()) {
            auto zone = new ExtZoneV1Interface(output->geometry(), handle);
            connect(output, &LogicalOutput::geometryChanged, zone, [zone, output] {
                zone->setArea(output->geometry());
            });
            connect(workspace(), &Workspace::outputRemoved, this, [this, handle] (LogicalOutput *output) {
                if (handle == output->name())
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
    : m_extZones(new ExtZoneManagerV1Interface(waylandServer()->display(), this))
{
}

}

#include "zones.moc"
