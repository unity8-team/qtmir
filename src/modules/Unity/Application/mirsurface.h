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

#ifndef MIRSURFACE_H
#define MIRSURFACE_H

// Qt
#include <QObject>

// mir
#include <mir_toolkit/common.h>
#include <mir/scene/surface.h>

// local
#include "globals.h"
#include "session_interface.h"

class SurfaceObserver;
class MirShell;

namespace qtmir {

class MirSurfaceManager;
class Application;

using namespace Globals;

class MirSurface : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QSize size READ size NOTIFY sizeChanged)
    Q_PROPERTY(MirSurface* parent READ parentSurface NOTIFY parentSurfaceChanged)
    Q_PROPERTY(QList<MirSurface*> children READ childSurfaces NOTIFY childSurfacesChanged)

    Q_PROPERTY(qtmir::Globals::WindowType type READ type NOTIFY typeChanged)
    Q_PROPERTY(qtmir::Globals::WindowState state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(bool focus READ focus WRITE setFocus NOTIFY focusChanged)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(Qt::ScreenOrientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(Qt::ScreenOrientations preferredOrientations READ preferredOrientations NOTIFY preferredOrientationsChanged)

    Q_PROPERTY(PixelFormat pixelFormat READ pixelFormat CONSTANT)
    Q_PROPERTY(bool supportsInput READ supportsInput CONSTANT)


public:
    explicit MirSurface(QObject *parent = 0);

    // getters
    QSize size() const;
    MirSurface* parentSurface() const;
    QList<MirSurface*> childSurfaces() const;
    WindowType type() const;
    WindowState state() const;
    QString name() const;
    bool focus() const;
    bool visible() const;
    Qt::ScreenOrientation orientation() const;
    Qt::ScreenOrientations preferredOrientations() const;
    PixelFormat pixelFormat() const;
    bool supportsInput() const;

    bool isFirstFrameDrawn() const { return m_firstFrameDrawn; }

    // setters
    void setFocus(const bool focus);
    void setVisible(const bool focus);
    void setOrientation(const Qt::ScreenOrientation orientation);

    // others
    Q_INVOKABLE void requestSize(const QSize &size);
    Q_INVOKABLE void requestClose();
    void sendEvent(QEvent *event);

    void allowFramedropping(const bool allow);


    // to allow easy touch event injection from tests
    bool processTouchEvent(int eventType,
            ulong timestamp,
            const QList<QTouchEvent::TouchPoint> &touchPoints,
            Qt::TouchPointStates touchPointStates);

Q_SIGNALS:
    void sizeChanged();
    void parentSurfaceChanged();
    void childSurfacesChanged();
    void typeChanged();
    void stateChanged();
    void nameChanged();
    void focusChanged();
    void visibleChanged();
    void orientationChanged();
    void preferredOrientationsChanged();

    void firstFrameDrawn();
    void damaged(const QRegion &rect);

private Q_SLOTS:
    void surfaceDamaged();
    void dropPendingBuffers();
    void scheduleTextureUpdate();

    void scheduleMirSurfaceSizeUpdate();
    void updateMirSurfaceSize();

    void updateMirSurfaceFocus(bool focused);
    
private:
    std::shared_ptr<mir::scene::Surface> m_surface;
    std::shared_ptr<SurfaceObserver> m_surfaceObserver;
    QPointer<SessionInterface> m_session;
    const MirShell *m_shell;
    bool m_firstFrameDrawn;
    bool m_live;
    Qt::ScreenOrientation m_orientation; //FIXME -  have to save the state as Mir has no getter for it (bug:1357429)

    QTimer m_frameDropperTimer;

    QTimer m_updateMirSurfaceSizeTimer;

    void setType(const WindowType &);
    void setState(const WindowState &);
    void setLive(const bool);

    // called by MirSurfaceManager
    void setAttribute(const MirSurfaceAttrib, const int);
    void setSurfaceValid(const bool);


    bool clientIsRunning() const;

    QString appId() const;

    void endCurrentTouchSequence(ulong timestamp);
    void validateAndDeliverTouchEvent(int eventType,
            ulong timestamp,
            const QList<QTouchEvent::TouchPoint> &touchPoints,
            Qt::TouchPointStates touchPointStates);

    class TouchEvent {
    public:
        TouchEvent &operator= (const QTouchEvent &qtEvent) {
            type = qtEvent.type();
            timestamp = qtEvent.timestamp();
            touchPoints = qtEvent.touchPoints();
            touchPointStates = qtEvent.touchPointStates();
            return *this;
        }

        void updateTouchPointStatesAndType();

        int type;
        ulong timestamp;
        QList<QTouchEvent::TouchPoint> touchPoints;
        Qt::TouchPointStates touchPointStates;
    } *m_lastTouchEvent;

    friend class MirSurfaceManager;
};

} // namespace qtmir

#endif // MIRSURFACE_H
