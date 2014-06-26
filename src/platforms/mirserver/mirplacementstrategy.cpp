/*
 * Copyright (C) 2014 Canonical, Ltd.
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

#include "mirplacementstrategy.h"
#include "logging.h"

#include <mir/geometry/rectangle.h>
#include <mir/shell/display_layout.h>
#include <mir/scene/surface_creation_parameters.h>

namespace ms = mir::scene;
namespace msh = mir::shell;

MirPlacementStrategy::MirPlacementStrategy()
{
    qCDebug(QTMIR_MIR_MESSAGES) << "MirPlacementStrategy::MirPlacementStrategy";
}

ms::SurfaceCreationParameters
MirPlacementStrategy::place(ms::Session const &session,
        ms::SurfaceCreationParameters const &requestParameters)
{
    using namespace mir::geometry;
    ms::SurfaceCreationParameters placedParameters = requestParameters;

    // allow shell to override surface geometry
    QSize size(placedParameters.size.width.as_uint32_t(), placedParameters.size.height.as_uint32_t());

    Q_EMIT requestSizeForSurface(&session, size);

    placedParameters.size = Size{ Width{size.width()}, Height{size.height()} };

    qCDebug(QTMIR_MIR_MESSAGES) << "MirPlacementStrategy: requested ("
        << requestParameters.size.width.as_int() << "," << requestParameters.size.height.as_int() << ") and returned ("
        << placedParameters.size.width.as_int() << "," << placedParameters.size.height.as_int() << ")";

    return placedParameters;
}
