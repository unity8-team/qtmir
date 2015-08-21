/*
 * Copyright © 2015 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored By: Alan Griffiths <alan@octopull.co.uk>
 */

#include "canonical_window_manager_info.h"

#include "mir/scene/surface_creation_parameters.h"
#include "mir/scene/surface.h"
#include "mir/shell/display_layout.h"
#include "mir/shell/surface_specification.h"
#include "mir/geometry/displacement.h"

#include <linux/input.h>
#include <csignal>

#include <climits>

namespace msh = mir::shell;
namespace ms = mir::scene;
using namespace mir::geometry;

msh::QtmirSurfaceInfo::QtmirSurfaceInfo(
    std::shared_ptr<scene::Session> const& session,
    std::shared_ptr<scene::Surface> const& surface,
    scene::SurfaceCreationParameters const& params) :
    type{surface->type()},
    state{surface->state()},
    restore_rect{surface->top_left(), surface->size()},
    session{session},
    parent{params.parent},
    min_width{params.min_width.is_set() ? params.min_width.value()  : Width{}},
    min_height{params.min_height.is_set() ? params.min_height.value() : Height{}},
    max_width{params.max_width.is_set() ? params.max_width.value() : Width{std::numeric_limits<int>::max()}},
    max_height{params.max_height.is_set() ? params.max_height.value() : Height{std::numeric_limits<int>::max()}},
    width_inc{params.width_inc},
    height_inc{params.height_inc},
    min_aspect{params.min_aspect},
    max_aspect{params.max_aspect}
{
}

bool msh::QtmirSurfaceInfo::can_be_active() const
{
    switch (type)
    {
    case mir_surface_type_normal:       /**< AKA "regular"                       */
    case mir_surface_type_utility:      /**< AKA "floating"                      */
    case mir_surface_type_dialog:
    case mir_surface_type_satellite:    /**< AKA "toolbox"/"toolbar"             */
    case mir_surface_type_freestyle:
    case mir_surface_type_menu:
    case mir_surface_type_inputmethod:  /**< AKA "OSK" or handwriting etc.       */
        return true;

    case mir_surface_type_gloss:
    case mir_surface_type_tip:          /**< AKA "tooltip"                       */
    default:
        // Cannot have input focus
        return false;
    }
}

bool msh::QtmirSurfaceInfo::must_have_parent() const
{
    return must_have_parent(type);
}

bool msh::QtmirSurfaceInfo::can_morph_to(MirSurfaceType new_type) const
{
    switch (new_type)
    {
    case mir_surface_type_normal:
    case mir_surface_type_utility:
    case mir_surface_type_satellite:
        switch (type)
        {
        case mir_surface_type_normal:
        case mir_surface_type_utility:
        case mir_surface_type_dialog:
        case mir_surface_type_satellite:
            return true;

        default:
            break;
        }
        break;

    case mir_surface_type_dialog:
        switch (type)
        {
        case mir_surface_type_normal:
        case mir_surface_type_utility:
        case mir_surface_type_dialog:
        case mir_surface_type_popover:
        case mir_surface_type_satellite:
            return true;

        default:
            break;
        }
        break;

    default:
        break;
    }

    return false;
}

bool msh::QtmirSurfaceInfo::must_not_have_parent() const
{
    return must_not_have_parent(type);
}

bool msh::QtmirSurfaceInfo::must_not_have_parent(MirSurfaceType type)
{
    switch (type)
    {
    case mir_surface_type_normal:
    case mir_surface_type_utility:
        return true;

    default:
        return false;
    }
}

bool msh::QtmirSurfaceInfo::must_have_parent(MirSurfaceType type)
{
    switch (type)
    {
    case mir_surface_type_overlay:;
    case mir_surface_type_satellite:
    case mir_surface_type_tip:
        return true;

    default:
        return false;
    }
}
