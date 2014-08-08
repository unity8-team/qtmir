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

#ifndef MIRSERVERQPA_MIR_PLACEMENT_STRATEGY_H
#define MIRSERVERQPA_MIR_PLACEMENT_STRATEGY_H

#include <mir/scene/placement_strategy.h>

#include <QObject>

#include <memory>

namespace mir {
    namespace shell {
        class DisplayLayout;
    }
}

class MirPlacementStrategy : public QObject, public mir::scene::PlacementStrategy
{
    Q_OBJECT

public:
    MirPlacementStrategy(const std::shared_ptr<mir::shell::DisplayLayout> &displayLayout);

    mir::scene::SurfaceCreationParameters place(const mir::scene::Session &session,
            const mir::scene::SurfaceCreationParameters &requestParameters) override;

Q_SIGNALS:
    void sessionAboutToCreateSurface(const mir::scene::Session &session, QSize &surfaceGeometry); // requires Qt::BlockingQueuedConnection!!

private:
    const std::shared_ptr<mir::shell::DisplayLayout> m_displayLayout;
};

#endif //  MIRSERVERQPA_MIR_PLACEMENT_STRATEGY_H
