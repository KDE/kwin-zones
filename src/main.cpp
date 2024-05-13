/*
    SPDX-FileCopyrightText: 2024 Aleix Pol Gonzalez <aleix.pol_gonzalez@mercedes-benz.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "zones.h"

namespace KWin
{

KWIN_EFFECT_FACTORY_SUPPORTED(Zones,
                              "metadata.json",
                              return true;)

} // namespace KWin

#include "main.moc"
