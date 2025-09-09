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

    ZoneZone *fetchZone(QScreen *screen);
    static bool isActive();

private:
    QHash<QScreen *, ZoneZone *> m_zones;
};

class ZoneItemAttached;

class ZoneItem : public QObject, public QtWayland::ext_zone_item_v1
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QPoint position READ position NOTIFY positionChanged)
    Q_PROPERTY(qint32 layerIndex READ layerIndex WRITE setLayerIndex NOTIFY layerIndexChanged)
    Q_PROPERTY(QPoint requestedPosition READ requestedPosition WRITE requestPosition NOTIFY requestedPositionChanged)
public:
    ZoneItem(QWindow *window);
    ZoneItemAttached *get();

    void setZone(ZoneZone *zone);
    ZoneZone *zone();

    qint32 layerIndex() const;
    void setLayerIndex(qint32 layerIndex);

    QPoint requestedPosition() const {
        return m_requestedPosition.value_or(QPoint{});
    }
    void requestPosition(const QPoint &position);

    void updatePosition(ZoneZone *zone, const QPoint &position);
    QWindow *window() const { return m_window; }
    QPoint position() const;

Q_SIGNALS:
    void zoneChanged(ZoneZone *zone);
    void positionChanged();
    void layerIndexChanged(qint32 layerIndex);
    void requestedPositionChanged();

private:
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    bool eventFilter(QObject *watched, QEvent *event) override;
#endif
    void ext_zone_item_v1_position(int32_t x, int32_t y) override {
        updatePosition(m_zone, {x, y});
    }
    void manageSurface();
    void initZone();

    ZoneItemAttached *m_attached = nullptr;
    ZoneZone *m_zone = nullptr;
    std::optional<qint32> m_layerIndex;
    std::optional<QPoint> m_requestedPosition;

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

    QSize m_size;
    QString m_handle;
};

