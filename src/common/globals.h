/*
 * Copyright (C) 2015 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <QObject>
#include <QSize>
#include <mir_toolkit/common.h>

namespace qtmir {

class Globals
{
    Q_GADGET
    Q_ENUMS(SurfaceState)
    Q_ENUMS(SurfaceType)

public:
    enum SurfaceState {
        Unknown = mir_surface_state_unknown,
        Restored = mir_surface_state_restored,
        Minimized = mir_surface_state_minimized,
        Maximized = mir_surface_state_maximized,
        VertMaximized = mir_surface_state_vertmaximized,
        /* SemiMaximized = mir_surface_state_semimaximized, // see mircommon/mir_toolbox/common.h*/
        Fullscreen = mir_surface_state_fullscreen,
    };

    enum SurfaceType {
        Normal = mir_surface_type_normal,
        Utility = mir_surface_type_utility,
        Dialog = mir_surface_type_dialog,
        Overlay = mir_surface_type_overlay,
        Freestyle = mir_surface_type_freestyle,
        Popover = mir_surface_type_popover,
        InputMethod = mir_surface_type_inputmethod,
    };

private:
    Globals() = default;
    ~Globals() = default;
};




struct SurfaceParameters
{
    QSize geometry;
    Globals::SurfaceState state;
};

} // namespace qtmir

#endif // GLOBALS_H
