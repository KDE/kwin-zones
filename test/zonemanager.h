// SPDX-FileCopyrightText: 2024 Aleix Pol Gonzalez <aleix.pol_gonzalez@mercedes-benz.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <QWaylandClientExtensionTemplate>
#include <QWindow>
#include <QtQmlIntegration>
#include "qwayland-ext-zones-v1.h"

class ZoneZone;

class ZoneManager : public QWaylandClientExtensionTemplate<ZoneManager>
                  , public QtWayland::ext_zone_manager_v1
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
public:
    ZoneManager();

    static bool isActive();
};

class ZoneItemAttached;

class ZoneItem : public QObject, public QtWayland::ext_zone_item_v1
{
    Q_OBJECT
    Q_PROPERTY(QPoint position READ position NOTIFY positionChanged)
public:
    ZoneItem(QWindow *window);
    ZoneItemAttached *get();

    void setZone(ZoneZone *zone);
    ZoneZone *zone();

    void updatePosition(ZoneZone *zone, const QPoint &position);
    QWindow *window() const { return m_window; }
    QPoint position() const;

Q_SIGNALS:
    void zoneChanged(ZoneZone *zone);
    void positionChanged();

private:
    void manageSurface();

    ZoneItemAttached *m_attached = nullptr;
    ZoneZone *m_zone = nullptr;

    QWindow *const m_window;
    QPoint m_pos;
};

class ZoneZone : public QObject, public QtWayland::ext_zone_v1
{
    Q_OBJECT
    Q_PROPERTY(QSize size MEMBER m_size NOTIFY done)
    Q_PROPERTY(QString handle MEMBER m_handle NOTIFY done)
public:
    ZoneZone(::ext_zone_v1 *zone);

Q_SIGNALS:
    void done();
private:
    void ext_zone_v1_size(int32_t width, int32_t height) override { m_size = {width, height}; }
    void ext_zone_v1_handle(const QString &handle) override { m_handle = handle; setObjectName(m_handle); }
    void ext_zone_v1_done() override { Q_EMIT done(); }
    void ext_zone_v1_item_entered(struct ::ext_zone_item_v1 */*item*/) override;
    void ext_zone_v1_item_left(struct ::ext_zone_item_v1 */*item*/) override {}
    void ext_zone_v1_position(struct ::ext_zone_item_v1 *item, int32_t x, int32_t y) override {
        auto www = dynamic_cast<ZoneItem *>(QtWayland::ext_zone_item_v1::fromObject(item));
        Q_ASSERT(www);
        www->updatePosition(this, {x, y});
    }
    void ext_zone_v1_position_failed(struct ::ext_zone_item_v1 *item) override;

    QSize m_size;
    QString m_handle;
};

