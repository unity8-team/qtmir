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
#include <mir/shell/display_layout.h>

namespace ms = mir::scene;
using mir::shell::AbstractShell;

MirShell::MirShell(
    const std::shared_ptr<mir::shell::InputTargeter> &inputTargeter,
    const std::shared_ptr<mir::scene::SurfaceCoordinator> &surfaceCoordinator,
    const std::shared_ptr<mir::scene::SessionCoordinator> &sessionCoordinator,
    const std::shared_ptr<mir::scene::PromptSessionManager> &promptSessionManager)
    : AbstractShell(inputTargeter, surfaceCoordinator, sessionCoordinator, promptSessionManager)
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

int MirShell::set_surface_attribute(
    const std::shared_ptr<mir::scene::Session> &session,
    const std::shared_ptr<mir::scene::Surface> &surface,
    MirSurfaceAttrib attrib,
    int value)
{
    auto const result = AbstractShell::set_surface_attribute(session, surface, attrib, value);
    Q_EMIT surfaceAttributeChanged(surface.get(), attrib, result);

    return result;
}
