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

// local
#include "mirsurface.h"
#include "mirshell.h"
#include "logging.h"
#include "session.h"

// mirserver
#include "surfaceobserver.h"

// common
#include "debughelpers.h"

// Qt
#include <QKeyEvent>

// Mir
#include <mir/geometry/rectangle.h>
#include <mir_toolkit/event.h>

namespace mg = mir::graphics;

namespace qtmir {

namespace {

bool fillInMirEvent(MirEvent &mirEvent, QKeyEvent *qtEvent)
{
    mirEvent.type = mir_event_type_key;

    // don't care
    mirEvent.key.device_id = 0;
    mirEvent.key.source_id = 0;

    switch (qtEvent->type()) {
        case QEvent::KeyPress:
            mirEvent.key.action = mir_key_action_down;
            break;
        case QEvent::KeyRelease:
            mirEvent.key.action = mir_key_action_up;
            break;
        default:
            return false;
    }

    // don't care
    mirEvent.key.flags = (MirKeyFlag)0;

    mirEvent.key.modifiers = qtEvent->nativeModifiers();
    mirEvent.key.key_code = qtEvent->nativeVirtualKey();
    mirEvent.key.scan_code = qtEvent->nativeScanCode();

    // TODO: It's not the best that we lose the actual repeat count from
    // the original mir event (pre QtEventFeeder)...of course it will
    // not matter for Qt clients...so this is an improvement for now.
    mirEvent.key.repeat_count = qtEvent->isAutoRepeat() ? 1 : 0;

    // Don't care
    mirEvent.key.down_time = 0;

    mirEvent.key.event_time = qtEvent->timestamp() * 1000000;

    // Don't care
    mirEvent.key.is_system_key = 0;

    return true;
}

bool fillInMirEvent(MirEvent &mirEvent,
                    const QList<QTouchEvent::TouchPoint> &qtTouchPoints,
                    Qt::TouchPointStates qtTouchPointStates,
                    ulong qtTimestamp)
{
    mirEvent.type = mir_event_type_motion;

    // Hardcoding it for now
    // TODO: Gather this info from a QTouchDevice-derived class created by QtEventFeeder
    mirEvent.motion.device_id = 0;
    mirEvent.motion.source_id = 0x00001002; // AINPUT_SOURCE_TOUCHSCREEN; https://bugs.launchpad.net/bugs/1311687

    // NB: it's assumed that touch points are pressed and released
    // one at a time.

    if (qtTouchPointStates.testFlag(Qt::TouchPointPressed)) {
        if (qtTouchPoints.count() > 1) {
            mirEvent.motion.action = mir_motion_action_pointer_down;
        } else {
            mirEvent.motion.action = mir_motion_action_down;
        }
    } else if (qtTouchPointStates.testFlag(Qt::TouchPointReleased)) {
        if (qtTouchPoints.count() > 1) {
            mirEvent.motion.action = mir_motion_action_pointer_up;
        } else {
            mirEvent.motion.action = mir_motion_action_up;
        }
    } else {
            mirEvent.motion.action = mir_motion_action_move;
    }

    // not used
    mirEvent.motion.flags = (MirMotionFlag) 0;

    // TODO: map QInputEvent::modifiers()
    mirEvent.motion.modifiers = 0;

    // not used
    mirEvent.motion.edge_flags = 0;

    // TODO
    mirEvent.motion.button_state = (MirMotionButton) 0;

    // Does it matter?
    mirEvent.motion.x_offset = 0.;
    mirEvent.motion.y_offset = 0.;
    mirEvent.motion.x_precision = 0.1;
    mirEvent.motion.y_precision = 0.1;

    // TODO. Not useful to Qt at least...
    mirEvent.motion.down_time = 0;

    // Note: QtEventFeeder scales the event time down, scale it back up - precision is
    // lost but the time difference should still be accurate to milliseconds
    mirEvent.motion.event_time = static_cast<nsecs_t>(qtTimestamp) * 1000000;

    mirEvent.motion.pointer_count = qtTouchPoints.count();

    for (int i = 0; i < qtTouchPoints.count(); ++i) {
        auto touchPoint = qtTouchPoints.at(i);
        auto &pointer = mirEvent.motion.pointer_coordinates[i];

        // FIXME: https://bugs.launchpad.net/mir/+bug/1311699
        // When multiple touch points are transmitted with a MirEvent
        // and one of them (only one is allowed) indicates a presse
        // state change the index is encoded in the second byte of the
        // action value.
        const int mir_motion_event_pointer_index_shift = 8;
        if (mirEvent.motion.action == mir_motion_action_pointer_up &&
            touchPoint.state() == Qt::TouchPointReleased)
        {
            mirEvent.motion.action |= i << mir_motion_event_pointer_index_shift;
        }
        if (mirEvent.motion.action == mir_motion_action_pointer_down &&
            touchPoint.state() == Qt::TouchPointPressed)
        {
            mirEvent.motion.action |= i << mir_motion_event_pointer_index_shift;
        }


        pointer.id = touchPoint.id();
        pointer.x = touchPoint.pos().x();
        pointer.y = touchPoint.pos().y();

        // FIXME: https://bugs.launchpad.net/mir/+bug/1311809

        if (touchPoint.rawScreenPositions().isEmpty()) {
            pointer.raw_x = 0.;
            pointer.raw_y = 0.;
        } else {
            pointer.raw_x = touchPoint.rawScreenPositions().at(0).x();
            pointer.raw_y =  touchPoint.rawScreenPositions().at(0).y();
        }

        pointer.touch_major = touchPoint.rect().width();
        pointer.touch_minor = touchPoint.rect().height();
        pointer.size = 0.;
        pointer.pressure = touchPoint.pressure();
        pointer.orientation = 0.;
        pointer.vscroll = 0.;
        pointer.hscroll = 0.;

        // TODO: Mir supports a wider set of tool types (finger, stylus, mouse, eraser, unknown).
        // so just because we are not TouchPoint::Pen does not mean we are motion_tool_type_finger...
        // however this is the best we can do with the QtEventFeeder approach.
        if (touchPoint.flags() & QTouchEvent::TouchPoint::Pen)
            pointer.tool_type = mir_motion_tool_type_stylus;
        else
            pointer.tool_type = mir_motion_tool_type_finger;
    }

    return true;
}

} // anonymous namespace

MirSurface::MirSurface(
        std::shared_ptr<mir::scene::Surface> surface,
        std::shared_ptr<SurfaceObserver> observer,
        SessionInterface* session,
        MirShell *shell,
        QObject *parent)
    : QObject(parent)
    , m_surface(surface)
    , m_surfaceObserver(observer)
    , m_session(session)
    , m_shell(shell)
    , m_orientation(Qt::PortraitOrientation)
    , m_lastTouchEvent(nullptr)
{
    qCDebug(QTMIR_SURFACES) << "MirSurface::MirSurface";

    m_surfaceObserver = observer;
    if (observer) {
        connect(observer.get(), &SurfaceObserver::framesPosted, this, &MirSurface::onFramePosted);
        observer->setListener(this);
    }

    if (m_session) {
        connect(m_session.data(), &Session::stateChanged, this, &MirSurfaceItem::onSessionStateChanged);
    }
}

MirSurface::~MirSurface()
{
    m_surface->remove_observer(m_surfaceObserver);
    delete m_lastTouchEvent;

}

QString MirSurface::name() const
{
    //FIXME - how to listen to change in this property?
    return QString::fromStdString(m_surface->name());
}

WindowType MirSurface::type() const
{
    return static_cast<WindowType>(m_surface->type());
}

void MirSurface::setType(const WindowType &type)
{
    if (this->type() != type) {
        m_shell->set_surface_attribute(m_session->session(), m_surface, mir_surface_attrib_type, static_cast<int>(type));
    }
}

WindowState MirSurface::state() const
{
    return static_cast<WindowState>(m_surface->state());
}

void MirSurface::setState(const WindowState &state)
{
    if (this->state() != state) {
        m_shell->set_surface_attribute(m_session->session(), m_surface, mir_surface_attrib_state, static_cast<int>(state));
    }
}

Qt::ScreenOrientation MirSurfaceItem::orientation() const
{
    return m_orientation;
}

void MirSurface::setOrientation(const Qt::ScreenOrientation orientation)
{
    qCDebug(QTMIR_SURFACES) << "MirSurface::setOrientation - orientation=" << orientation;

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


// Called by MirSurfaceManager upon a msh::Surface attribute change
void MirSurface::setAttribute(const MirSurfaceAttrib attribute, const int /*value*/)
{
    switch (attribute) {
    case mir_surface_attrib_type:
        Q_EMIT typeChanged();
        break;
    case mir_surface_attrib_state:
        Q_EMIT stateChanged();
        break;
    default:
        break;
    }
}

void MirSurface::setFocus(const bool focus)
{
    qCDebug(QTMIR_SURFACES) << "MirSurface::setFocus" << focus;
    if (focus) {
        m_shell->set_surface_attribute(m_session->session(), m_surface, mir_surface_attrib_focus, mir_surface_focused);
    } else {
        m_shell->set_surface_attribute(m_session->session(), m_surface, mir_surface_attrib_focus, mir_surface_unfocused);
    }
}

void MirSurface::endCurrentTouchSequence(ulong timestamp)
{
    MirEvent mirEvent;

    Q_ASSERT(m_lastTouchEvent);
    Q_ASSERT(m_lastTouchEvent->type != QEvent::TouchEnd);
    Q_ASSERT(m_lastTouchEvent->touchPoints.count() > 0);

    TouchEvent touchEvent = *m_lastTouchEvent;
    touchEvent.timestamp = timestamp;

    // Remove all already released touch points
    int i = 0;
    while (i < touchEvent.touchPoints.count()) {
        if (touchEvent.touchPoints[i].state() == Qt::TouchPointReleased) {
            touchEvent.touchPoints.removeAt(i);
        } else {
            ++i;
        }
    }

    // And release the others one by one as Mir expects one press/release per event
    while (touchEvent.touchPoints.count() > 0) {
        touchEvent.touchPoints[0].setState(Qt::TouchPointReleased);

        touchEvent.updateTouchPointStatesAndType();

        if (fillInMirEvent(mirEvent, touchEvent.touchPoints,
                           touchEvent.touchPointStates, touchEvent.timestamp)) {
            m_surface->consume(mirEvent);
        }
        *m_lastTouchEvent = touchEvent;

        touchEvent.touchPoints.removeAt(0);
    }
}

void MirSurface::validateAndDeliverTouchEvent(int eventType,
            ulong timestamp,
            const QList<QTouchEvent::TouchPoint> &touchPoints,
            Qt::TouchPointStates touchPointStates)
{
    MirEvent mirEvent;

    if (eventType == QEvent::TouchBegin && m_lastTouchEvent && m_lastTouchEvent->type != QEvent::TouchEnd) {
        qCWarning(QTMIR_SURFACES) << qPrintable(QString("MirSurfaceItem(%1) - Got a QEvent::TouchBegin while "
            "there's still an active/unfinished touch sequence.").arg(appId()));
        // Qt forgot to end the last touch sequence. Let's do it ourselves.
        endCurrentTouchSequence(timestamp);
    }

    if (fillInMirEvent(mirEvent, touchPoints, touchPointStates, timestamp)) {
        m_surface->consume(mirEvent);
    }

    if (!m_lastTouchEvent) {
        m_lastTouchEvent = new TouchEvent;
    }
    m_lastTouchEvent->type = eventType;
    m_lastTouchEvent->timestamp = timestamp;
    m_lastTouchEvent->touchPoints = touchPoints;
    m_lastTouchEvent->touchPointStates = touchPointStates;
}

void MirSurface::touchEvent(QTouchEvent *event)
{
    bool accepted = processTouchEvent(event->type(),
            event->timestamp(),
            event->touchPoints(),
            event->touchPointStates());
    event->setAccepted(accepted);
}

bool MirSurface::processTouchEvent(
        int eventType,
        ulong timestamp,
        const QList<QTouchEvent::TouchPoint> &touchPoints,
        Qt::TouchPointStates touchPointStates)
{
    bool accepted = true;
    if (type() == InputMethod && eventType == QEvent::TouchBegin) {
        // FIXME: Hack to get the VKB use case working while we don't have the proper solution in place.
        if (hasTouchInsideUbuntuKeyboard(touchPoints)) {
            validateAndDeliverTouchEvent(eventType, timestamp, touchPoints, touchPointStates);
        } else {
            accepted = false;
        }

    } else {
        // NB: If we are getting QEvent::TouchUpdate or QEvent::TouchEnd it's because we've
        // previously accepted the corresponding QEvent::TouchBegin
        validateAndDeliverTouchEvent(eventType, timestamp, touchPoints, touchPointStates);
    }
    return accepted;
}


} // namespace qtmir