/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#ifndef MIRSURFACEITEMINTERFACE_H
#define MIRSURFACEITEMINTERFACE_H

// Qt
#include <QQuickItem>

// mir
#include <mir_toolkit/common.h>

#include "session_interface.h"
#include "globals.h"

namespace qtmir {

class MirSurfaceItemInterface : public QQuickItem
{
    Q_OBJECT
    Q_ENUMS(Type)
    Q_ENUMS(State)
    Q_ENUMS(OrientationAngle)

    Q_PROPERTY(qtmir::Globals::SurfaceType type READ type NOTIFY typeChanged)
    Q_PROPERTY(qtmir::Globals::SurfaceState state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(bool live READ live NOTIFY liveChanged)

    // How many degrees, clockwise, the UI in the surface has to rotate to match with the
    // shell UI orientation
    Q_PROPERTY(qtmir::Globals::OrientationAngle orientationAngle READ orientationAngle WRITE setOrientationAngle
               NOTIFY orientationAngleChanged DESIGNABLE false)

public:
    MirSurfaceItemInterface(QQuickItem *parent) : QQuickItem(parent) {}
    virtual ~MirSurfaceItemInterface() {}

    //getters
    virtual Globals::SurfaceType type() const = 0;
    virtual Globals::SurfaceState state() const = 0;
    virtual QString name() const = 0;
    virtual bool live() const = 0;
    virtual SessionInterface *session() const = 0;
    virtual Globals::OrientationAngle orientationAngle() const = 0;

    virtual Q_INVOKABLE void release() = 0;

    virtual void stopFrameDropper() = 0;
    virtual void startFrameDropper() = 0;

    virtual bool isFirstFrameDrawn() const = 0;

    virtual void setOrientationAngle(Globals::OrientationAngle angle) = 0;
    virtual void setSession(SessionInterface *app) = 0;

Q_SIGNALS:
    void typeChanged();
    void stateChanged();
    void nameChanged();
    void orientationAngleChanged(Globals::OrientationAngle angle);
    void liveChanged(bool live);
    void firstFrameDrawn();

private:
    virtual void setLive(bool) = 0;

    friend class MirSurfaceManager;
};

} // namespace qtmir

Q_DECLARE_METATYPE(qtmir::MirSurfaceItemInterface*)

#endif // MIRSURFACEITEMINTERFACE_H

