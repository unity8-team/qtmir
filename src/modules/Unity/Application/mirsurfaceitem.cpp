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
 *
 * Authors:
 *     Daniel d'Andrada <daniel.dandrada@canonical.com>
 *     Gerry Boland <gerry.boland@canonical.com>
 */

// local
#include "application.h"
#include "mirbuffersgtexture.h"
#include "session.h"
#include "mirsurfaceitem.h"
#include "mirshell.h"
#include "logging.h"
#include "ubuntukeyboardinfo.h"

// mirserver
#include "surfaceobserver.h"

// common
#include <debughelpers.h>

// Qt
#include <QDebug>
#include <QGuiApplication>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QScreen>
#include <private/qsgdefaultimagenode_p.h>
#include <QSGTextureProvider>
#include <QTimer>

// Mir
#include <mir/geometry/rectangle.h>
#include <mir_toolkit/event.h>

namespace mg = mir::graphics;

namespace qtmir {

class QMirSurfaceTextureProvider : public QSGTextureProvider
{
    Q_OBJECT
public:
    QMirSurfaceTextureProvider() : t(0) { }
    ~QMirSurfaceTextureProvider() { delete t; }

    QSGTexture *texture() const {
        if (t)
            t->setFiltering(smooth ? QSGTexture::Linear : QSGTexture::Nearest);
        return t;
    }

    bool smooth;
    MirBufferSGTexture *t;

public Q_SLOTS:
    void invalidate()
    {
        delete t;
        t = 0;
    }
};

MirSurfaceItem::MirSurfaceItem(const QPointer<MirSurface> mirSurface,
                               QQuickItem *parent)
    : QQuickItem(parent)
    , m_textureProvider(nullptr)
{
    qCDebug(QTMIR_SURFACES) << "MirSurfaceItem::MirSurfaceItem" << mirSurface;

    setSmooth(true);
    setFlag(QQuickItem::ItemHasContents, true); //so scene graph will render this item
    setAcceptedMouseButtons(Qt::LeftButton | Qt::MiddleButton | Qt::RightButton |
        Qt::ExtraButton1 | Qt::ExtraButton2 | Qt::ExtraButton3 | Qt::ExtraButton4 |
        Qt::ExtraButton5 | Qt::ExtraButton6 | Qt::ExtraButton7 | Qt::ExtraButton8 |
        Qt::ExtraButton9 | Qt::ExtraButton10 | Qt::ExtraButton11 |
        Qt::ExtraButton12 | Qt::ExtraButton13);
    setAcceptHoverEvents(true);

    // fetch surface geometry
    setImplicitSize(mirSurface->size().width(), mirSurface->size().height());

    if (!UbuntuKeyboardInfo::instance()) {
        new UbuntuKeyboardInfo;
    }

    connect(&m_frameDropperTimer, &QTimer::timeout,
            this, &MirSurfaceItem::dropPendingBuffers);
    // Rationale behind the frame dropper and its interval value:
    //
    // We want to give ample room for Qt scene graph to have a chance to fetch and render
    // the next pending buffer before we take the drastic action of dropping it (so don't set
    // it anywhere close to our target render interval).
    //
    // We also want to guarantee a minimal frames-per-second (fps) frequency for client applications
    // as they get stuck on swap_buffers() if there's no free buffer to swap to yet (ie, they
    // are all pending consumption by the compositor, us). But on the other hand, we don't want
    // that minimal fps to be too high as that would mean this timer would be triggered way too often
    // for nothing causing unnecessary overhead as actually dropping frames from an app should
    // in practice rarely happen.
    m_frameDropperTimer.setInterval(200);
    m_frameDropperTimer.setSingleShot(false);

    m_updateMirSurfaceSizeTimer.setSingleShot(true);
    m_updateMirSurfaceSizeTimer.setInterval(1);
    connect(&m_updateMirSurfaceSizeTimer, &QTimer::timeout, this, &MirSurfaceItem::updateMirSurfaceSize);
    connect(this, &QQuickItem::widthChanged, this, &MirSurfaceItem::scheduleMirSurfaceSizeUpdate);
    connect(this, &QQuickItem::heightChanged, this, &MirSurfaceItem::scheduleMirSurfaceSizeUpdate);

    // FIXME - setting surface unfocused immediately breaks camera & video apps, but is
    // technically the correct thing to do (surface should be unfocused until shell focuses it)
    //m_surface->configure(mir_surface_attrib_focus, mir_surface_unfocused);
    connect(this, &QQuickItem::activeFocusChanged, this, &MirSurfaceItem::updateMirSurfaceFocus);
}

MirSurfaceItem::~MirSurfaceItem()
{
    if (m_session) {
        m_session->setSurface(nullptr);
    }

    qCDebug(QTMIR_SURFACES) << "MirSurfaceItem::~MirSurfaceItem - this=" << this;
    QMutexLocker locker(&m_mutex);
    if (m_textureProvider)
        m_textureProvider->deleteLater();
}

SessionInterface* MirSurfaceItem::session() const
{
    return m_session.data();
}

WindowType MirSurfaceItem::type() const
{
    if (m_mirSurface) {
        return m_mirSurface->type();
    }
}

WindowState MirSurfaceItem::state() const
{
    if (m_mirSurface) {
        return m_mirSurface->state();
    }
}

Qt::ScreenOrientation MirSurfaceItem::orientation() const
{
    return m_orientation;
}

void MirSurfaceItem::setOrientation(const Qt::ScreenOrientation orientation)
{
    qCDebug(QTMIR_SURFACES) << "MirSurfaceItem::setOrientation - orientation=" << orientation;

    if (m_orientation == orientation)
        return;

    MirOrientation mirOrientation;
    Qt::ScreenOrientation nativeOrientation = QGuiApplication::primaryScreen()->nativeOrientation();
    const bool landscapeNativeOrientation = (nativeOrientation == Qt::LandscapeOrientation);

    Qt::ScreenOrientation requestedOrientation = orientation;
    if (orientation == Qt::PrimaryOrientation) { // means orientation equals native orientation, set it as such
        requestedOrientation = nativeOrientation;
    }

    switch(requestedOrientation) {
    case Qt::PortraitOrientation:
        mirOrientation = (landscapeNativeOrientation) ? mir_orientation_right : mir_orientation_normal;
        break;
    case Qt::LandscapeOrientation:
        mirOrientation = (landscapeNativeOrientation) ? mir_orientation_normal : mir_orientation_left;
        break;
    case Qt::InvertedPortraitOrientation:
        mirOrientation = (landscapeNativeOrientation) ? mir_orientation_left : mir_orientation_inverted;
        break;
    case Qt::InvertedLandscapeOrientation:
        mirOrientation = (landscapeNativeOrientation) ? mir_orientation_inverted : mir_orientation_right;
        break;
    default:
        qWarning("Unrecognized Qt::ScreenOrientation!");
        return;
    }

    m_surface->set_orientation(mirOrientation);

    m_orientation = orientation;
    Q_EMIT orientationChanged();
}

QString MirSurfaceItem::name() const
{
    if (m_mirSurface) {
        return m_mirSurface->name();
    }
}

bool MirSurfaceItem::live() const
{
    if (m_mirSurface) {
        return m_mirSurface->live();
    }
    return false;
}

// Called from the rendering (scene graph) thread
QSGTextureProvider *MirSurfaceItem::textureProvider() const
{
    const_cast<MirSurfaceItem *>(this)->ensureProvider();
    return m_textureProvider;
}

void MirSurfaceItem::ensureProvider()
{
    if (!m_textureProvider) {
        m_textureProvider = new QMirSurfaceTextureProvider();
        connect(window(), SIGNAL(sceneGraphInvalidated()),
                m_textureProvider, SLOT(invalidate()), Qt::DirectConnection);
    }
}

void MirSurfaceItem::surfaceDamaged()
{
    if (!m_firstFrameDrawn) {
        m_firstFrameDrawn = true;
        Q_EMIT firstFrameDrawn(this);
    }

    scheduleTextureUpdate();
}

bool MirSurfaceItem::updateTexture()    // called by rendering thread (scene graph)
{
    QMutexLocker locker(&m_mutex);
    ensureProvider();
    bool textureUpdated = false;

    const void* const userId = (void*)123;
    std::unique_ptr<mg::Renderable> renderable =
        m_surface->compositor_snapshot(userId);

    if (m_surface->buffers_ready_for_compositor(userId) > 0) {
        if (!m_textureProvider->t) {
            m_textureProvider->t = new MirBufferSGTexture(renderable->buffer());
        } else {
            // Avoid holding two buffers for the compositor at the same time. Thus free the current
            // before acquiring the next
            m_textureProvider->t->freeBuffer();
            m_textureProvider->t->setBuffer(renderable->buffer());
        }
        textureUpdated = true;
    }

    if (m_surface->buffers_ready_for_compositor(userId) > 0) {
        QTimer::singleShot(0, this, SLOT(update()));
        // restart the frame dropper so that we have enough time to render the next frame.
        // queued since the timer lives in a different thread
        QMetaObject::invokeMethod(&m_frameDropperTimer, "start", Qt::QueuedConnection);
    }

    m_textureProvider->smooth = smooth();

    return textureUpdated;
}

QSGNode *MirSurfaceItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)    // called by render thread
{
    if (!m_surface) {
        delete oldNode;
        return 0;
    }

    bool textureUpdated = updateTexture();
    if (!m_textureProvider->t) {
        delete oldNode;
        return 0;
    }

    QSGDefaultImageNode *node = static_cast<QSGDefaultImageNode*>(oldNode);
    if (!node) {
        node = new QSGDefaultImageNode;
        node->setTexture(m_textureProvider->t);

        node->setMipmapFiltering(QSGTexture::None);
        node->setHorizontalWrapMode(QSGTexture::ClampToEdge);
        node->setVerticalWrapMode(QSGTexture::ClampToEdge);
        node->setSubSourceRect(QRectF(0, 0, 1, 1));
    } else {
        if (textureUpdated) {
            node->markDirty(QSGNode::DirtyMaterial);
        }
    }

    node->setTargetRect(QRectF(0, 0, width(), height()));
    node->setInnerTargetRect(QRectF(0, 0, width(), height()));

    node->setFiltering(smooth() ? QSGTexture::Linear : QSGTexture::Nearest);
    node->setAntialiasing(antialiasing());

    node->update();

    return node;
}

void MirSurfaceItem::focusInEvent(QFocusEvent *event)
{
    if (m_mirSurface) {
        m_mirSurface->setFocus(true);
    }
    event->accept();
}

void MirSurfaceItem::focusOutEvent(QFocusEvent *event)
{
    if (m_mirSurface) {
        m_mirSurface->setFocus(false);
    }
    event->accept();
}

void MirSurfaceItem::mousePressEvent(QMouseEvent *event)
{
    // TODO: Implement for desktop support
    event->ignore();
}

void MirSurfaceItem::mouseMoveEvent(QMouseEvent *event)
{
    event->accept();
}

void MirSurfaceItem::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();
}

void MirSurfaceItem::wheelEvent(QWheelEvent *event)
{
    event->accept();
}

void MirSurfaceItem::keyPressEvent(QKeyEvent *qtEvent)
{
    MirEvent mirEvent;
    if (fillInMirEvent(mirEvent, qtEvent)) {
        m_surface->consume(mirEvent);
    }
}

void MirSurfaceItem::keyReleaseEvent(QKeyEvent *qtEvent)
{
    MirEvent mirEvent;
    if (fillInMirEvent(mirEvent, qtEvent)) {
        m_surface->consume(mirEvent);
    }
}

QString MirSurfaceItem::appId() const
{
    QString appId;
    if (session() && session()->application()) {
        appId = session()->application()->appId();
    } else {
        appId.append("-");
    }
    return appId;
}



bool MirSurfaceItem::hasTouchInsideUbuntuKeyboard(const QList<QTouchEvent::TouchPoint> &touchPoints)
{
    UbuntuKeyboardInfo *ubuntuKeyboardInfo = UbuntuKeyboardInfo::instance();

    for (int i = 0; i < touchPoints.count(); ++i) {
        QPoint pos = touchPoints.at(i).pos().toPoint();
        if (pos.x() >= ubuntuKeyboardInfo->x()
                && pos.x() <= (ubuntuKeyboardInfo->x() + ubuntuKeyboardInfo->width())
                && pos.y() >= ubuntuKeyboardInfo->y()
                && pos.y() <= (ubuntuKeyboardInfo->y() + ubuntuKeyboardInfo->height())) {
            return true;
        }
    }
    return false;
}

void MirSurfaceItem::setType(const Type &type)
{
    if (m_mirSurface) {
        m_mirSurface->setType(type);
    }
}

void MirSurfaceItem::setState(const State &state)
{
    if (m_mirSurface) {
        m_mirSurface->setState(state);
    }
}

void MirSurfaceItem::setLive(const bool live)
{
    if (m_live != live) {
        m_live = live;
        Q_EMIT liveChanged(m_live);
    }
}

void MirSurfaceItem::scheduleMirSurfaceSizeUpdate()
{
    if (clientIsRunning() && !m_updateMirSurfaceSizeTimer.isActive()) {
        m_updateMirSurfaceSizeTimer.start();
    }
}

void MirSurfaceItem::updateMirSurfaceSize()
{
    int mirWidth = m_surface->size().width.as_int();
    int mirHeight = m_surface->size().height.as_int();

    int qmlWidth = (int)width();
    int qmlHeight = (int)height();

    bool mirSizeIsDifferent = qmlWidth != mirWidth || qmlHeight != mirHeight;

    const char *didResize = clientIsRunning() && mirSizeIsDifferent ? "surface resized" : "surface NOT resized";
    qCDebug(QTMIR_SURFACES) << "MirSurfaceItem::updateMirSurfaceSize"
            << "surface =" << this
            << ", old (" << mirWidth << "," << mirHeight << ")"
            << ", new (" << qmlWidth << "," << qmlHeight << ")"
            << didResize;

    if (clientIsRunning() && mirSizeIsDifferent) {
        mir::geometry::Size newMirSize(qmlWidth, qmlHeight);
        m_surface->resize(newMirSize);
        setImplicitSize(qmlWidth, qmlHeight);
    }
}

void MirSurfaceItem::dropPendingBuffers()
{
    QMutexLocker locker(&m_mutex);

    const void* const userId = (void*)123;  // TODO: Multimonitor support

    while (m_surface->buffers_ready_for_compositor(userId) > 0) {
        // The line below looks like an innocent, effect-less, getter. But as this
        // method returns a unique_pointer, not holding its reference causes the
        // buffer to be destroyed/released straight away.
        m_surface->compositor_snapshot(userId)->buffer();
        qCDebug(QTMIR_SURFACES) << "MirSurfaceItem::dropPendingBuffers()"
            << "surface =" << this
            << "buffer dropped."
            << m_surface->buffers_ready_for_compositor(userId)
            << "left.";
    }
}

void MirSurfaceItem::stopFrameDropper()
{
    qCDebug(QTMIR_SURFACES) << "MirSurfaceItem::stopFrameDropper surface = " << this;
    QMutexLocker locker(&m_mutex);
    m_frameDropperTimer.stop();
}

void MirSurfaceItem::startFrameDropper()
{
    qCDebug(QTMIR_SURFACES) << "MirSurfaceItem::startFrameDropper surface = " << this;
    QMutexLocker locker(&m_mutex);
    if (!m_frameDropperTimer.isActive()) {
        m_frameDropperTimer.start();
    }
}

void MirSurfaceItem::scheduleTextureUpdate()
{
    QMutexLocker locker(&m_mutex);

    // Notify QML engine that this needs redrawing, schedules call to updatePaintItem
    update();
    // restart the frame dropper so that we have enough time to render the next frame.
    m_frameDropperTimer.start();
}

void MirSurfaceItem::setSession(SessionInterface *session)
{
    m_session = session;
}

void MirSurfaceItem::onSessionStateChanged(SessionInterface::State state)
{
    switch (state) {
        case SessionInterface::State::Running:
            syncSurfaceSizeWithItemSize();
            break;
        default:
            break;
    }
}

void MirSurfaceItem::syncSurfaceSizeWithItemSize()
{
    int mirWidth = m_surface->size().width.as_int();
    int mirHeight = m_surface->size().width.as_int();

    if ((int)width() != mirWidth || (int)height() != mirHeight) {
        qCDebug(QTMIR_SURFACES) << "MirSurfaceItem::syncSurfaceSizeWithItemSize()";
        mir::geometry::Size newMirSize((int)width(), (int)height());
        m_surface->resize(newMirSize);
        setImplicitSize(width(), height());
    }
}

bool MirSurfaceItem::clientIsRunning() const
{
    return (m_session &&
            (m_session->state() == Session::State::Running
             || m_session->state() == Session::State::Starting))
        || !m_session;
}

void MirSurfaceItem::TouchEvent::updateTouchPointStatesAndType()
{
    touchPointStates = 0;
    for (int i = 0; i < touchPoints.count(); ++i) {
        touchPointStates |= touchPoints.at(i).state();
    }

    if (touchPointStates == Qt::TouchPointReleased) {
        type = QEvent::TouchEnd;
    } else if (touchPointStates == Qt::TouchPointPressed) {
        type = QEvent::TouchBegin;
    } else {
        type = QEvent::TouchUpdate;
    }
}

} // namespace qtmir

#include "mirsurfaceitem.moc"
