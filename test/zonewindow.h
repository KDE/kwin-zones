// SPDX-FileCopyrightText: 2024 Aleix Pol Gonzalez <aleix.pol_gonzalez@mercedes-benz.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <QObject>
#include <QtQmlIntegration>
#include "qwayland-ext-zones-v1.h"
#include "zonemanager.h"

class QWindow;

class ZoneWindowAttached : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use attached")
    QML_ATTACHED(ZoneWindowAttached)
    Q_PROPERTY(ZoneWindow *window MEMBER m_window CONSTANT)
    Q_PROPERTY(ZoneZone *zone READ zone WRITE setZone NOTIFY zoneChanged)
public:
    ZoneWindowAttached(ZoneWindow *window);

    ZoneZone *zone();
    void setZone(ZoneZone *zone);

    /**
     * Gets the ZoneWindow for a given Qt Window
     * Ownership is not transferred
     */
    static ZoneWindowAttached *get(QWindow *window);
    static ZoneWindowAttached *qmlAttachedProperties(QObject *object);

Q_SIGNALS:
    void zoneChanged(ZoneZone *zone);

public Q_SLOTS:
    void requestPosition(const QPoint &point);

private:
    ZoneWindow *const m_window;
    ZoneZone *m_zone = nullptr;
};
