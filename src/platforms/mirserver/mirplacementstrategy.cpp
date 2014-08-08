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

// local
#include "mirplacementstrategy.h"
#include "logging.h"

// mir
#include <mir/geometry/rectangle.h>
#include <mir/shell/display_layout.h>
#include <mir/scene/surface_creation_parameters.h>

// Qt
#include <QSize>

namespace ms = mir::scene;
namespace msh = mir::shell;

MirPlacementStrategy::MirPlacementStrategy(const std::shared_ptr<msh::DisplayLayout> &displayLayout)
    : m_displayLayout(displayLayout)
{
    qCDebug(QTMIR_MIR_MESSAGES) << "MirPlacementStrategy::MirPlacementStrategy";
}

ms::SurfaceCreationParameters
MirPlacementStrategy::place(const ms::Session &session,
        const ms::SurfaceCreationParameters &requestParameters)
{
    using namespace mir::geometry;

    QSize surfaceGeometry(requestParameters.size.width.as_int(),
                          requestParameters.size.height.as_int());

    Q_EMIT sessionAboutToCreateSurface(session, surfaceGeometry); // should be connected to via Qt::BlockingQueuedConnection
                                                                  // as surfaceGeometry can be altered

    // Sanity check, fetch display geometry and make sure surface fits inside it
    Rectangle displayRect;
    m_displayLayout->size_to_output(displayRect);

    QSize displayGeometry(displayRect.size.width.as_int(),
                          displayRect.size.height.as_int());

    if (displayGeometry.width() < surfaceGeometry.width()
            || displayGeometry.height() < surfaceGeometry.height()) {
        qCDebug(QTMIR_MIR_MESSAGES) << "MirPlacementStrategy - need to override shell provided geometry as surface larger than display!";
        surfaceGeometry = surfaceGeometry.boundedTo(displayGeometry);
    }

    ms::SurfaceCreationParameters placedParameters = requestParameters;
    placedParameters.size = Size{ Width{surfaceGeometry.width()}, Height{surfaceGeometry.height()} };

    qCDebug(QTMIR_MIR_MESSAGES) << "MirPlacementStrategy: requested ("
        << requestParameters.size.width.as_int() << "," << requestParameters.size.height.as_int() << ") and returned ("
        << placedParameters.size.width.as_int() << "," << placedParameters.size.height.as_int() << ")";

    return placedParameters;
}
