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

// Qt
#include <QObject>
#include <QSize>

class MirPlacementStrategy : public QObject, public mir::scene::PlacementStrategy
{
    Q_OBJECT
public:
    MirPlacementStrategy();

    mir::scene::SurfaceCreationParameters place(mir::scene::Session const &session,
            mir::scene::SurfaceCreationParameters const &request_parameters) override;

Q_SIGNALS:
    void requestSizeForSurface(const mir::scene::Session *session, QSize &size);

private:
};

#endif //  MIRSERVERQPA_MIR_PLACEMENT_STRATEGY_H
