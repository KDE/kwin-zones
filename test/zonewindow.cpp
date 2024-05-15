// SPDX-FileCopyrightText: 2024 Aleix Pol Gonzalez <aleix.pol_gonzalez@mercedes-benz.com>
// SPDX-License-Identifier: MIT

#include "zonewindow.h"
#include <QGuiApplication>
#include <QWindow>

#include <qqml.h>
#include <qpa/qplatformnativeinterface.h>

QML_DECLARE_TYPEINFO(ZoneWindowAttached, QML_HAS_ATTACHED_PROPERTIES)
Q_GLOBAL_STATIC(ZoneManager, s_manager)

ZoneWindowAttached::ZoneWindowAttached(ZoneWindow* window)
    : m_window(window)
{
    qDebug() << "yyyyyyyy";
}

ZoneWindowAttached* ZoneWindowAttached::get(QWindow* window)
{
    if (!window) {
        qDebug() << "no window??";
        return nullptr;
    }
    if (!s_manager->isActive()) {
        qDebug() << "no zones??";
        return nullptr;
    }

    auto tl = (::xdg_toplevel *)QGuiApplication::platformNativeInterface()->nativeResourceForWindow("xdg_toplevel", window);
    if (!tl) {
        qDebug() << "window is not a top level" << window;
        return nullptr;
    }
    ::ext_zone_window_v1 *xx = s_manager->get_zone_window(tl);
    ZoneWindow *www = dynamic_cast<ZoneWindow *>(QtWayland::ext_zone_window_v1::fromObject(xx));
    if (!www) {
        www = new ZoneWindow(xx, window);
        qDebug() << "xxxxxxxx1" << www;
    }
    qDebug() << "xxxxxxxx2" << www->get();
    return www->get();
}

ZoneWindowAttached *ZoneWindowAttached::qmlAttachedProperties(QObject *object)
{
    return get(qobject_cast<QWindow *>(object));
}

void ZoneWindowAttached::requestPosition(const QPoint& point)
{
    Q_ASSERT(m_window);
    qDebug() << "requesting in" << zone() << point;

    zone()->set_position(m_window->object(), point.x(), point.y());
}

void ZoneWindowAttached::setZone(ZoneZone* zone)
{
    if (m_zone == zone) {
        return;
    }
    if (m_zone) {
        m_zone->remove_window(m_window->object());
    }
    m_zone = zone;
    m_zone->add_window(m_window->object());

    Q_EMIT zoneChanged(m_zone);

    if (m_window && m_zone) {
        m_zone->get_position(m_window->object());
    }
}

ZoneZone* ZoneWindowAttached::zone()
{
    if (!m_zone) {
        auto output = (::wl_output *)QGuiApplication::platformNativeInterface()->nativeResourceForScreen("output", m_window->window()->screen());
        m_zone = new ZoneZone(s_manager->get_zone(output, ""));
    }
    return m_zone;
}
