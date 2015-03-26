/*
 * Copyright © 2015 Canonical Ltd.
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

#ifndef MIRWINDOWMANAGER_H
#define MIRWINDOWMANAGER_H

#include <mir/shell/window_manager.h>
#include <QObject>

namespace mir {
    namespace shell {
        class DisplayLayout;
    }
}

class MirWindowManager : public QObject, public mir::shell::WindowManager
{
    Q_OBJECT

public:
    explicit MirWindowManager(const std::shared_ptr<mir::shell::DisplayLayout> &displayLayout);

    auto add_surface(
        std::shared_ptr<mir::scene::Session> const& session,
        mir::scene::SurfaceCreationParameters const& params,
        std::function<mir::frontend::SurfaceId(std::shared_ptr<mir::scene::Session> const& session, mir::scene::SurfaceCreationParameters const& params)> const& build)
    -> mir::frontend::SurfaceId override;

    int set_surface_attribute(
        std::shared_ptr<mir::scene::Session> const& session,
        std::shared_ptr<mir::scene::Surface> const& surface,
        MirSurfaceAttrib attrib,
        int value) override;

    void add_session(std::shared_ptr<mir::scene::Session> const& session) override;

    void remove_session(std::shared_ptr<mir::scene::Session> const& session) override;

    void remove_surface(
        std::shared_ptr<mir::scene::Session> const& session,
        std::weak_ptr<mir::scene::Surface> const& surface) override;

    void add_display(mir::geometry::Rectangle const& area) override;

    void remove_display(mir::geometry::Rectangle const& area) override;

    bool handle_key_event(MirKeyInputEvent const* event) override;

    bool handle_touch_event(MirTouchInputEvent const* event) override;

    bool handle_pointer_event(MirPointerInputEvent const* event) override;

Q_SIGNALS:
    void surfaceAttributeChanged(mir::scene::Surface const*, const MirSurfaceAttrib, const int);

private:
    std::shared_ptr<mir::shell::DisplayLayout> const m_displayLayout;
};

#endif /* MIRWINDOWMANAGER_H */
