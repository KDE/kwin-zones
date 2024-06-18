// SPDX-FileCopyrightText: 2024 Aleix Pol Gonzalez <aleix.pol_gonzalez@mercedes-benz.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <QObject>
#include <QtQmlIntegration>
#include "qwayland-ext-zones-v1.h"
#include "zonemanager.h"

class QWindow;

class ZoneItemAttached : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use attached")
    QML_ATTACHED(ZoneItemAttached)
    Q_PROPERTY(ZoneItem *item MEMBER m_item CONSTANT)
    Q_PROPERTY(ZoneZone *zone READ zone WRITE setZone NOTIFY zoneChanged)
public:
    ZoneItemAttached(ZoneItem *window);

    ZoneZone *zone() const;
    void setZone(ZoneZone *zone);

    /**
     * Gets the ZoneItem for a given Qt Window
     * Ownership is not transferred
     */
    static ZoneItemAttached *get(QWindow *window);
    static ZoneItemAttached *qmlAttachedProperties(QObject *object);

Q_SIGNALS:
    void zoneChanged(ZoneZone *zone);

public Q_SLOTS:
    void requestPosition(const QPoint &point);

private:
    ZoneItem *const m_item;
};
