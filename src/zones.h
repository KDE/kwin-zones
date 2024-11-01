/*
    SPDX-FileCopyrightText: 2024 Aleix Pol Gonzalez <aleix.pol_gonzalez@mercedes-benz.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <plugin.h>

namespace KWin
{
class ExtZoneManagerV1Interface;

class Zones : public Plugin
{
    Q_OBJECT
public:
    explicit Zones();

private:
    ExtZoneManagerV1Interface *const m_extZones;
};

} // namespace KWin
