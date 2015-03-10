/*
 * Copyright (C) 2013-2014 Canonical, Ltd.
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

#ifndef MIRSURFACEITEM_H
#define MIRSURFACEITEM_H

// Std
#include <memory>

// Qt
#include <QMutex>
#include <QPointer>
#include <QSet>
#include <QQuickItem>
#include <QTimer>
#include <QQmlListProperty>

// mir
#include <mir/scene/surface.h>
#include <mir_toolkit/common.h>

// local
#include "mirsurface.h"
#include "session_interface.h"

class SurfaceObserver;
class MirShell;

namespace qtmir {

class MirSurfaceManager;
class QSGMirSurfaceNode;
class QMirSurfaceTextureProvider;
class Application;

using namespace Globals;

class MirSurfaceItem : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(qtmir::Globals::WindowType type READ type NOTIFY typeChanged)
    Q_PROPERTY(qtmir::Globals::SurfaceState state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(bool live READ live NOTIFY liveChanged)
    Q_PROPERTY(Qt::ScreenOrientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged DESIGNABLE false)

public:
    explicit MirSurfaceItem(const QPointer<MirSurface> mirSurface,
                            QQuickItem *parent = 0);
    ~MirSurfaceItem();

    //getters
    WindowType type() const;
    SurfaceState state() const;
    QString name() const;
    bool live() const;
    Qt::ScreenOrientation orientation() const;
    SessionInterface *session() const;

    // Item surface/texture management
    bool isTextureProvider() const { return true; }
    QSGTextureProvider *textureProvider() const;

    void stopFrameDropper();
    void startFrameDropper();

    void setOrientation(const Qt::ScreenOrientation orientation);
    void setSession(SessionInterface *app);

Q_SIGNALS:
    void typeChanged();
    void stateChanged();
    void nameChanged();
    void orientationChanged();
    void liveChanged(bool live);
    void firstFrameDrawn(MirSurfaceItem *item);

protected Q_SLOTS:
    void onSessionStateChanged(SessionInterface::State state);

protected:
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    void touchEvent(QTouchEvent *event) override;

    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *);

private Q_SLOTS:
    void surfaceDamaged();
    void dropPendingBuffers();
    void scheduleTextureUpdate();

    void scheduleMirSurfaceSizeUpdate();
    void updateMirSurfaceSize();

private:
    bool updateTexture();
    void ensureProvider();

    bool hasTouchInsideUbuntuKeyboard(const QList<QTouchEvent::TouchPoint> &touchPoints);
    void syncSurfaceSizeWithItemSize();

    const QPointer<MirSurface> m_mirSurface;
    QMirSurfaceTextureProvider *m_textureProvider;

    QMutex m_mutex;
};

} // namespace qtmir

Q_DECLARE_METATYPE(qtmir::MirSurfaceItem*)

#endif // MIRSURFACEITEM_H
