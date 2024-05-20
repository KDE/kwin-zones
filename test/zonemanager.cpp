// SPDX-FileCopyrightText: 2024 Aleix Pol Gonzalez <aleix.pol_gonzalez@mercedes-benz.com>
// SPDX-License-Identifier: MIT

#include "zonemanager.h"
#include "zonewindow.h"

ZoneManager::ZoneManager()
    : QWaylandClientExtensionTemplate<ZoneManager>(1)
{
    initialize();
}

ZoneWindow::ZoneWindow(::ext_zone_window_v1 *w, QWindow *window)
    : QtWayland::ext_zone_window_v1(w)
    , m_window(window)
{}

ZoneWindowAttached* ZoneWindow::get()
{
    if (!m_attached) {
        m_attached = new ZoneWindowAttached(this);
    }
    return m_attached;
}

ZoneZone::ZoneZone(::ext_zone_v1* zone)
    : QtWayland::ext_zone_v1(zone)
{
}

void ZoneZone::ext_zone_v1_position_failed(ext_zone_window_v1* window)
{
    qDebug() << "failed to position window" << window;
}

void ZoneWindow::updatePosition(ZoneZone* zone, const QPoint& position)
{
    qDebug() << "updated position" << zone << position;
    m_pos = position;
    Q_EMIT positionChanged();
}

QPoint ZoneWindow::position() const
{
    return m_pos;
}
