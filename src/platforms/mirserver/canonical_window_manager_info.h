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

#ifndef MIR_SHELL_CANONICAL_WINDOW_MANAGER_INFO_H_
#define MIR_SHELL_CANONICAL_WINDOW_MANAGER_INFO_H_

#include <mir/geometry/rectangle.h>
#include <mir/geometry/displacement.h>
#include <mir/optional_value.h>
#include <mir/shell/surface_specification.h>
#include <mir_toolkit/common.h>

#include <memory>
#include <vector>

namespace mir
{
namespace scene
{
class Session;
class Surface;
class SurfaceCreationParameters;
}

namespace shell
{
class DisplayLayout;

struct QtmirSessionInfo
{
    int surfaces{0};
};

struct QtmirSurfaceInfo
{
    QtmirSurfaceInfo(
        std::shared_ptr<scene::Session> const& session,
        std::shared_ptr<scene::Surface> const& surface,
        scene::SurfaceCreationParameters const& params);

    bool can_be_active() const;

    bool can_morph_to(MirSurfaceType new_type) const;
    bool must_have_parent() const;
    bool must_not_have_parent() const;

    static bool must_not_have_parent(MirSurfaceType type);
    static bool must_have_parent(MirSurfaceType type);

    void constrain_resize(
        std::shared_ptr<scene::Surface> const& surface,
        geometry::Point& requested_pos,
        geometry::Size& requested_size,
        const bool left_resize,
        const bool top_resize,
        geometry::Rectangle const& bounds) const;

    MirSurfaceType type;
    MirSurfaceState state;
    geometry::Rectangle restore_rect;
    std::weak_ptr<scene::Session> session;
    std::weak_ptr<scene::Surface> parent;
    std::vector<std::weak_ptr<scene::Surface>> children;
    geometry::Width min_width;
    geometry::Height min_height;
    geometry::Width max_width;
    geometry::Height max_height;
    mir::optional_value<geometry::DeltaX> width_inc;
    mir::optional_value<geometry::DeltaY> height_inc;
    mir::optional_value<SurfaceAspectRatio> min_aspect;
    mir::optional_value<SurfaceAspectRatio> max_aspect;
};

}
}

#endif /* MIR_SHELL_CANONICAL_WINDOW_MANAGER_INFO_H_ */
