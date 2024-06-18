// SPDX-FileCopyrightText: 2024 Aleix Pol Gonzalez <aleix.pol_gonzalez@mercedes-benz.com>
// SPDX-License-Identifier: MIT

#include "zonemanager.h"
#include "zoneitemattached.h"

#include <QGuiApplication>
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include <qpa/qplatformnativeinterface.h>
#include <qwayland-wayland.h>

Q_GLOBAL_STATIC(ZoneManager, s_manager)

ZoneManager::ZoneManager()
    : QWaylandClientExtensionTemplate<ZoneManager>(1)
{
    initialize();
}

bool ZoneManager::isActive()
{
    return s_manager->isInitialized();
}

ZoneItem::ZoneItem(QWindow *window)
    : QtWayland::ext_zone_item_v1()
    , m_window(window)
{
    Q_ASSERT(m_window->isTopLevel());
#if QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
    connect(window, &QWindow::visibilityChanged, this, &ZoneItem::manageSurface, Qt::QueuedConnection);
#else
    connect(window, &QWindow::surfaceRoleCreated, this, &ZoneItem::manageSurface, Qt::QueuedConnection);
#endif
}

void ZoneItem::manageSurface()
{
    if (!m_window->isVisible()) {
        if (isInitialized()) {
            destroy();
        }
        return;
    }

    auto tl = (::xdg_toplevel *)QGuiApplication::platformNativeInterface()->nativeResourceForWindow("xdg_toplevel", m_window);
    if (!tl) {
        qDebug() << "No xdg_toplevel yet!" << m_window << tl;
        return;
    }

    ::ext_zone_item_v1 *item = s_manager->get_zone_item(tl);
    init(item);
    ext_zone_item_v1_set_user_data(item, (QtWayland::ext_zone_item_v1 *) this);
    Q_ASSERT(isInitialized());
    if (m_zone) {
        m_zone->add_item(object());
    }
}

void ZoneItem::setZone(ZoneZone* zone)
{
    if (m_zone == zone) {
        return;
    }
    if (m_zone && object()) {
        m_zone->remove_item(object());
    }
    m_zone = zone;
    if (m_zone && object()) {
        m_zone->add_item(object());
    }
    Q_EMIT zoneChanged(m_zone);
}

ZoneZone* ZoneItem::zone()
{
    if (!m_zone) {
        auto output = (::wl_output *)QGuiApplication::platformNativeInterface()->nativeResourceForScreen("output", m_window->screen());
        Q_ASSERT(output);
        setZone(new ZoneZone(s_manager->get_zone(output)));
    }
    return m_zone;
}

ZoneItemAttached* ZoneItem::get()
{
    if (!m_attached) {
        m_attached = new ZoneItemAttached(this);
    }
    return m_attached;
}

void ZoneItem::updatePosition(ZoneZone* zone, const QPoint& position)
{
    if (zone != m_zone) {
        qWarning() << "Something weird happened" << zone << m_zone << position;
    }
    m_pos = position;
    Q_EMIT positionChanged();
}

QPoint ZoneItem::position() const
{
    return m_pos;
}

ZoneZone::ZoneZone(::ext_zone_v1* zone)
    : QtWayland::ext_zone_v1(zone)
{
}

void ZoneZone::ext_zone_v1_position_failed(ext_zone_item_v1* item)
{
    auto www = dynamic_cast<ZoneItem *>(QtWayland::ext_zone_item_v1::fromObject(item));
    qDebug() << "failed to position window" << item << www;
}

void ZoneZone::ext_zone_v1_item_entered(ext_zone_item_v1* item)
{
    qDebug() << "item entered" << QtWayland::ext_zone_item_v1::fromObject(item) << item;
    get_position(item);
}
