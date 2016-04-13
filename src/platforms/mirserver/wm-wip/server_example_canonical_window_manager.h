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

#ifndef MIR_EXAMPLE_CANONICAL_WINDOW_MANAGER_H_
#define MIR_EXAMPLE_CANONICAL_WINDOW_MANAGER_H_

#include "server_example_basic_window_manager.h"

#include "mir/geometry/displacement.h"

#include <atomic>
#include <set>

///\example server_example_canonical_window_manager.h
// Based on "Mir and Unity: Surfaces, input, and displays (v0.3)"

namespace mir
{
namespace shell { class DisplayLayout; }
namespace examples
{
// standard window management algorithm:
//  o Switch apps: tap or click on the corresponding tile
//  o Move window: Alt-leftmousebutton drag (three finger drag)
//  o Resize window: Alt-middle_button drag (two finger drag)
//  o Maximize/restore current window (to display size): Alt-F11
//  o Maximize/restore current window (to display height): Shift-F11
//  o Maximize/restore current window (to display width): Ctrl-F11
//  o client requests to maximize, vertically maximize & restore
class CanonicalWindowManagerPolicyCopy  : public WindowManagementPolicy
{
public:

    explicit CanonicalWindowManagerPolicyCopy(
        WindowManagerTools* const tools,
        std::shared_ptr<shell::DisplayLayout> const& display_layout);

    void click(geometry::Point cursor);

    void handle_session_info_updated(SessionInfoMap& session_info, geometry::Rectangles const& displays);

    void handle_displays_updated(SessionInfoMap& session_info, geometry::Rectangles const& displays);

    void resize(geometry::Point cursor);

    auto handle_place_new_surface(
        std::shared_ptr<scene::Session> const& session,
        scene::SurfaceCreationParameters const& request_parameters)
    -> scene::SurfaceCreationParameters;

    void handle_new_surface(std::shared_ptr<scene::Session> const& session, std::shared_ptr<scene::Surface> const& surface);

    void handle_modify_surface(
        std::shared_ptr<scene::Session> const& session,
        std::shared_ptr<scene::Surface> const& surface,
        shell::SurfaceSpecification const& modifications);

    void handle_delete_surface(std::shared_ptr<scene::Session> const& session, std::weak_ptr<scene::Surface> const& surface);

    int handle_set_state(std::shared_ptr<scene::Surface> const& surface, MirSurfaceState value);

    void drag(geometry::Point cursor);

    bool handle_keyboard_event(MirKeyboardEvent const* event);

    bool handle_touch_event(MirTouchEvent const* event);

    bool handle_pointer_event(MirPointerEvent const* event);

    void handle_raise_surface(
        std::shared_ptr<scene::Session> const& session,
        std::shared_ptr<scene::Surface> const& surface);

    void generate_decorations_for(
        std::shared_ptr<scene::Session> const& session,
        std::shared_ptr<scene::Surface> const& surface,
        SurfaceInfoMap& surface_map,
        std::function<frontend::SurfaceId(std::shared_ptr<scene::Session> const& session, scene::SurfaceCreationParameters const& params)> const& build);

private:
    static const int modifier_mask =
        mir_input_event_modifier_alt |
        mir_input_event_modifier_shift |
        mir_input_event_modifier_sym |
        mir_input_event_modifier_ctrl |
        mir_input_event_modifier_meta;

    void toggle(MirSurfaceState state);

    // "Mir and Unity: Surfaces, input, and displays (v0.3)" talks about active
    //  *window*,but Mir really only understands surfaces
    void select_active_surface(std::shared_ptr<scene::Surface> const& surface);
    auto active_surface() const -> std::shared_ptr<scene::Surface>;

    bool resize(std::shared_ptr<scene::Surface> const& surface, geometry::Point cursor, geometry::Point old_cursor, geometry::Rectangle bounds);
    bool drag(std::shared_ptr<scene::Surface> surface, geometry::Point to, geometry::Point from, geometry::Rectangle bounds);
    void move_tree(std::shared_ptr<scene::Surface> const& root, geometry::Displacement movement) const;
    void apply_resize(
        std::shared_ptr<mir::scene::Surface> const& surface,
        std::shared_ptr<mir::scene::Surface> const& titlebar,
        geometry::Point const& new_pos,
        geometry::Size const& new_size) const;

    WindowManagerTools* const tools;
    std::shared_ptr<shell::DisplayLayout> const display_layout;

    geometry::Rectangle display_area;
    geometry::Point old_cursor{};
    std::weak_ptr<scene::Surface> active_surface_;
    using FullscreenSurfaces = std::set<std::weak_ptr<scene::Surface>, std::owner_less<std::weak_ptr<scene::Surface>>>;

    FullscreenSurfaces fullscreen_surfaces;
};
}
}

#endif /* MIR_EXAMPLE_CANONICAL_WINDOW_MANAGER_H_ */