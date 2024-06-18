// SPDX-FileCopyrightText: 2024 Aleix Pol Gonzalez <aleix.pol_gonzalez@mercedes-benz.com>
// SPDX-License-Identifier: MIT

#include "zoneitemattached.h"
#include <QGuiApplication>
#include <QWindow>

#include <qqml.h>
#include <qpa/qplatformnativeinterface.h>

QML_DECLARE_TYPEINFO(ZoneItemAttached, QML_HAS_ATTACHED_PROPERTIES)

ZoneItemAttached::ZoneItemAttached(ZoneItem* item)
    : m_item(item)
{
}

ZoneItemAttached* ZoneItemAttached::get(QWindow* window)
{
    if (!window) {
        qDebug() << "no window??";
        return nullptr;
    }
    if (!ZoneManager::isActive()) {
        qDebug() << "no zones??";
        return nullptr;
    }

    ZoneItem *item = window->property("_zones").value<ZoneItem *>();
    if (!item) {
        item = new ZoneItem(window);
        item->setProperty("_zones", QVariant::fromValue<ZoneItem *>(item));
    }
    return item->get();
}

ZoneItemAttached *ZoneItemAttached::qmlAttachedProperties(QObject *object)
{
    auto window = qobject_cast<QWindow *>(object);
    if (!window) {
        qDebug() << "Cannot attach to" << object;
    }
    return get(window);
}

void ZoneItemAttached::requestPosition(const QPoint& point)
{
    Q_ASSERT(m_item);
    qDebug() << "requesting in" << zone() << point;

    zone()->set_position(m_item->object(), point.x(), point.y());
}

void ZoneItemAttached::setZone(ZoneZone* zone)
{
    m_item->setZone(zone);
}

ZoneZone* ZoneItemAttached::zone() const
{
    return m_item->zone();
}
