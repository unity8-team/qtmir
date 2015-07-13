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

#include "mirshell.h"
#include "logging.h"
#include "tracepoints.h" // generated from tracepoints.tp

#include <mir/geometry/rectangle.h>
#include <mir/scene/session.h>
#include <mir/scene/surface_creation_parameters.h>
#include <mir/scene/surface.h>
#include <mir/shell/display_layout.h>
#include <mir/shell/window_manager.h>

namespace ms = mir::scene;
using mir::shell::AbstractShell;

namespace
{
class NullWindowManager : public mir::shell::WindowManager
{
public:
    void add_session(std::shared_ptr<ms::Session> const& session) override;

    void remove_session(std::shared_ptr<ms::Session> const& session) override;

    mir::frontend::SurfaceId add_surface(
        std::shared_ptr<ms::Session> const& session,
        ms::SurfaceCreationParameters const& params,
        std::function<mir::frontend::SurfaceId(std::shared_ptr<ms::Session> const& session, ms::SurfaceCreationParameters const& params)> const& build) override;

    void remove_surface(
        std::shared_ptr<ms::Session> const& session,
        std::weak_ptr<ms::Surface> const& surface) override;

    void add_display(mir::geometry::Rectangle const& area) override;

    void remove_display(mir::geometry::Rectangle const& area) override;

    bool handle_keyboard_event(MirKeyboardEvent const* event) override;

    bool handle_touch_event(MirTouchEvent const* event) override;

    bool handle_pointer_event(MirPointerEvent const* event) override;

    int set_surface_attribute(
        std::shared_ptr<ms::Session> const& session,
        std::shared_ptr<ms::Surface> const& surface,
        MirSurfaceAttrib attrib,
        int value) override;

    void modify_surface(const std::shared_ptr<mir::scene::Session>&, const std::shared_ptr<mir::scene::Surface>&, const mir::shell::SurfaceSpecification&);
};
}


MirShell::MirShell(
    const std::shared_ptr<mir::shell::InputTargeter> &inputTargeter,
    const std::shared_ptr<mir::scene::SurfaceCoordinator> &surfaceCoordinator,
    const std::shared_ptr<mir::scene::SessionCoordinator> &sessionCoordinator,
    const std::shared_ptr<mir::scene::PromptSessionManager> &promptSessionManager) :
    AbstractShell(inputTargeter, surfaceCoordinator, sessionCoordinator, promptSessionManager,
        [](mir::shell::FocusController*) { return std::make_shared<NullWindowManager>(); })
{
    qCDebug(QTMIR_MIR_MESSAGES) << "MirShell::MirShell";
}

mir::frontend::SurfaceId MirShell::create_surface(const std::shared_ptr<ms::Session> &session,
                                                  const ms::SurfaceCreationParameters &requestParameters)
{
    using namespace mir::geometry;
    using namespace qtmir;
    tracepoint(qtmirserver, surfacePlacementStart);

    SurfaceParameters params;
    params.geometry.setWidth(requestParameters.size.width.as_int());
    params.geometry.setHeight(requestParameters.size.height.as_int());
    if (requestParameters.state.is_set()) {
        params.state = static_cast<Globals::SurfaceState>(requestParameters.state.value());
    } else {
        params.state = Globals::SurfaceState::Unknown;
    }

    Q_EMIT sessionAboutToCreateSurface(session, params); // can be connected to via Qt::BlockingQueuedConnection
                                                         // to alter surface initial geometry and state

    ms::SurfaceCreationParameters placedParameters = requestParameters;
    placedParameters.size = Size{ Width{params.geometry.width()}, Height{params.geometry.height()} };

    qCDebug(QTMIR_MIR_MESSAGES) << "MirPlacementStrategy: requested ("
        << requestParameters.size.width.as_int() << "," << requestParameters.size.height.as_int() << ") and returned ("
        << placedParameters.size.width.as_int() << "," << placedParameters.size.height.as_int() << ")";

    tracepoint(qtmirserver, surfacePlacementEnd);

    return AbstractShell::create_surface(session, placedParameters);
}

void NullWindowManager::add_session(std::shared_ptr<ms::Session> const& /*session*/)
{
}

void NullWindowManager::remove_session(std::shared_ptr<ms::Session> const& /*session*/)
{
}

auto NullWindowManager::add_surface(
    std::shared_ptr<ms::Session> const& session,
    ms::SurfaceCreationParameters const& params,
    std::function<mir::frontend::SurfaceId(std::shared_ptr<ms::Session> const& session, ms::SurfaceCreationParameters const& params)> const& build)
-> mir::frontend::SurfaceId
{
    return build(session, params);
}

void NullWindowManager::remove_surface(
    std::shared_ptr<ms::Session> const& /*session*/,
    std::weak_ptr<ms::Surface> const& /*surface*/)
{
}

void NullWindowManager::add_display(mir::geometry::Rectangle const& /*area*/)
{
}

void NullWindowManager::remove_display(mir::geometry::Rectangle const& /*area*/)
{
}

bool NullWindowManager::handle_keyboard_event(MirKeyboardEvent const* /*event*/)
{
    return false;
}

bool NullWindowManager::handle_touch_event(MirTouchEvent const* /*event*/)
{
    return false;
}

bool NullWindowManager::handle_pointer_event(MirPointerEvent const* /*event*/)
{
    return false;
}

int NullWindowManager::set_surface_attribute(
    std::shared_ptr<ms::Session> const& /*session*/,
    std::shared_ptr<ms::Surface> const& surface,
    MirSurfaceAttrib attrib,
    int value)
{
    return surface->configure(attrib, value);
}

void NullWindowManager::modify_surface(const std::shared_ptr<mir::scene::Session>&, const std::shared_ptr<mir::scene::Surface>&, const mir::shell::SurfaceSpecification&)
{
}
