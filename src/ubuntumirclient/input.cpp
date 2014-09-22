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

// Local
#include "input.h"
#include "integration.h"
#include "nativeinterface.h"
#include "window.h"
#include "logging.h"

// Qt
#if !defined(QT_NO_DEBUG)
#include <QtCore/QThread>
#endif
#include <QtCore/qglobal.h>
#include <QtCore/QCoreApplication>
#include <private/qguiapplication_p.h>
#include <qpa/qplatforminputcontext.h>
#include <qpa/qwindowsysteminterface.h>

#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-keysyms.h>

// Platform API
#include <ubuntu/application/ui/input/event.h>

#define LOG_EVENTS 0

// XKB Keysyms which do not map directly to Qt types (i.e. Unicode points)
static const uint32_t KeyTable[] = {
    XKB_KEY_Escape,                  Qt::Key_Escape,
    XKB_KEY_Tab,                     Qt::Key_Tab,
    XKB_KEY_ISO_Left_Tab,            Qt::Key_Backtab,
    XKB_KEY_BackSpace,               Qt::Key_Backspace,
    XKB_KEY_Return,                  Qt::Key_Return,
    XKB_KEY_Insert,                  Qt::Key_Insert,
    XKB_KEY_Delete,                  Qt::Key_Delete,
    XKB_KEY_Clear,                   Qt::Key_Delete,
    XKB_KEY_Pause,                   Qt::Key_Pause,
    XKB_KEY_Print,                   Qt::Key_Print,

    XKB_KEY_Home,                    Qt::Key_Home,
    XKB_KEY_End,                     Qt::Key_End,
    XKB_KEY_Left,                    Qt::Key_Left,
    XKB_KEY_Up,                      Qt::Key_Up,
    XKB_KEY_Right,                   Qt::Key_Right,
    XKB_KEY_Down,                    Qt::Key_Down,
    XKB_KEY_Prior,                   Qt::Key_PageUp,
    XKB_KEY_Next,                    Qt::Key_PageDown,

    XKB_KEY_Shift_L,                 Qt::Key_Shift,
    XKB_KEY_Shift_R,                 Qt::Key_Shift,
    XKB_KEY_Shift_Lock,              Qt::Key_Shift,
    XKB_KEY_Control_L,               Qt::Key_Control,
    XKB_KEY_Control_R,               Qt::Key_Control,
    XKB_KEY_Meta_L,                  Qt::Key_Meta,
    XKB_KEY_Meta_R,                  Qt::Key_Meta,
    XKB_KEY_Alt_L,                   Qt::Key_Alt,
    XKB_KEY_Alt_R,                   Qt::Key_Alt,
    XKB_KEY_Caps_Lock,               Qt::Key_CapsLock,
    XKB_KEY_Num_Lock,                Qt::Key_NumLock,
    XKB_KEY_Scroll_Lock,             Qt::Key_ScrollLock,
    XKB_KEY_Super_L,                 Qt::Key_Super_L,
    XKB_KEY_Super_R,                 Qt::Key_Super_R,
    XKB_KEY_Menu,                    Qt::Key_Menu,
    XKB_KEY_Hyper_L,                 Qt::Key_Hyper_L,
    XKB_KEY_Hyper_R,                 Qt::Key_Hyper_R,
    XKB_KEY_Help,                    Qt::Key_Help,

    XKB_KEY_KP_Space,                Qt::Key_Space,
    XKB_KEY_KP_Tab,                  Qt::Key_Tab,
    XKB_KEY_KP_Enter,                Qt::Key_Enter,
    XKB_KEY_KP_Home,                 Qt::Key_Home,
    XKB_KEY_KP_Left,                 Qt::Key_Left,
    XKB_KEY_KP_Up,                   Qt::Key_Up,
    XKB_KEY_KP_Right,                Qt::Key_Right,
    XKB_KEY_KP_Down,                 Qt::Key_Down,
    XKB_KEY_KP_Prior,                Qt::Key_PageUp,
    XKB_KEY_KP_Next,                 Qt::Key_PageDown,
    XKB_KEY_KP_End,                  Qt::Key_End,
    XKB_KEY_KP_Begin,                Qt::Key_Clear,
    XKB_KEY_KP_Insert,               Qt::Key_Insert,
    XKB_KEY_KP_Delete,               Qt::Key_Delete,
    XKB_KEY_KP_Equal,                Qt::Key_Equal,
    XKB_KEY_KP_Multiply,             Qt::Key_Asterisk,
    XKB_KEY_KP_Add,                  Qt::Key_Plus,
    XKB_KEY_KP_Separator,            Qt::Key_Comma,
    XKB_KEY_KP_Subtract,             Qt::Key_Minus,
    XKB_KEY_KP_Decimal,              Qt::Key_Period,
    XKB_KEY_KP_Divide,               Qt::Key_Slash,

    XKB_KEY_ISO_Level3_Shift,        Qt::Key_AltGr,
    XKB_KEY_Multi_key,               Qt::Key_Multi_key,
    XKB_KEY_Codeinput,               Qt::Key_Codeinput,
    XKB_KEY_SingleCandidate,         Qt::Key_SingleCandidate,
    XKB_KEY_MultipleCandidate,       Qt::Key_MultipleCandidate,
    XKB_KEY_PreviousCandidate,       Qt::Key_PreviousCandidate,

    XKB_KEY_Mode_switch,             Qt::Key_Mode_switch,
    XKB_KEY_script_switch,           Qt::Key_Mode_switch,
    XKB_KEY_XF86AudioRaiseVolume,    Qt::Key_VolumeUp,
    XKB_KEY_XF86AudioLowerVolume,    Qt::Key_VolumeDown,
    XKB_KEY_XF86PowerOff,            Qt::Key_PowerOff,
    XKB_KEY_XF86PowerDown,           Qt::Key_PowerDown,

    0,                          0
};

// Lookup table for the key types.
// FIXME(loicm) Not sure what to do with that multiple thing.
static const QEvent::Type kEventType[] = {
    QEvent::KeyPress,    // U_KEY_ACTION_DOWN     = 0
    QEvent::KeyRelease,  // U_KEY_ACTION_UP       = 1
    QEvent::KeyPress     // U_KEY_ACTION_MULTIPLE = 2
};

class UbuntuEvent : public QEvent
{
public:
    UbuntuEvent(UbuntuWindow* window, const WindowEvent* event, QEvent::Type type)
        : QEvent(type), window(window) {
        memcpy(&nativeEvent, event, sizeof(WindowEvent));
    }
    UbuntuWindow* window;
    WindowEvent nativeEvent;
};

UbuntuInput::UbuntuInput(UbuntuClientIntegration* integration)
    : QObject(nullptr)
    , mIntegration(integration)
    , mEventFilterType(static_cast<UbuntuNativeInterface*>(
        integration->nativeInterface())->genericEventFilterType())
    , mEventType(static_cast<QEvent::Type>(QEvent::registerEventType()))
{
    // Initialize touch device.
    mTouchDevice = new QTouchDevice;
    mTouchDevice->setType(QTouchDevice::TouchScreen);
    mTouchDevice->setCapabilities(
            QTouchDevice::Position | QTouchDevice::Area | QTouchDevice::Pressure |
            QTouchDevice::NormalizedPosition);
    QWindowSystemInterface::registerTouchDevice(mTouchDevice);
}

UbuntuInput::~UbuntuInput()
{
  // Qt will take care of deleting mTouchDevice.
}

#ifndef QT_NO_DEBUG
/*
static const char* nativeEventTypeToStr(int eventType)
{
    switch (eventType) {
    case MOTION_WEVENT_TYPE:
        return "MOTION_WEVENT_TYPE";
        break;
    case KEY_WEVENT_TYPE:
        return "KEY_WEVENT_TYPE";
        break;
    case RESIZE_WEVENT_TYPE:
        return "RESIZE_WEVENT_TYPE";
        break;
    case SURFACE_WEVENT_TYPE:
        return "SURFACE_WEVENT_TYPE";
    default:
        return "INVALID!";
    }
}
*/
#endif

void UbuntuInput::customEvent(QEvent* event)
{
    DASSERT(QThread::currentThread() == thread());
    UbuntuEvent* ubuntuEvent = static_cast<UbuntuEvent*>(event);
    WindowEvent *nativeEvent = &ubuntuEvent->nativeEvent;

    if (ubuntuEvent->window->window() == nullptr) {
        qWarning() << "Attempted to deliver an event to a non-existant QWindow, ignoring.";
        return;
    }

    // Event filtering.
    long result;
    if (QWindowSystemInterface::handleNativeEvent(
                ubuntuEvent->window->window(), mEventFilterType, nativeEvent, &result) == true) {
        DLOG("event filtered out by native interface");
        return;
    }

    //DLOG("UbuntuInput::customEvent(type=%s)", nativeEventTypeToStr(nativeEvent->type));

    // Event dispatching.
    switch (nativeEvent->type) {
    case MOTION_WEVENT_TYPE:
        dispatchMotionEvent(ubuntuEvent->window->window(), nativeEvent);
        break;
    case KEY_WEVENT_TYPE:
        dispatchKeyEvent(ubuntuEvent->window->window(), nativeEvent);
        break;
    case RESIZE_WEVENT_TYPE:
        ubuntuEvent->window->handleSurfaceResize(nativeEvent->resize.width,
                                                 nativeEvent->resize.height);
        break;
    case SURFACE_WEVENT_TYPE:
        if (nativeEvent->surface.attribute == SURFACE_ATTRIBUTE_FOCUS) {
            ubuntuEvent->window->handleSurfaceFocusChange(nativeEvent->surface.value == 1);
        }
        break;
    default:
        DLOG("unhandled event type %d", nativeEvent->type);
    }
}

void UbuntuInput::postEvent(UbuntuWindow* platformWindow, const void* event)
{
    QWindow *window = platformWindow->window();

    QCoreApplication::postEvent(this, new UbuntuEvent(
            platformWindow, reinterpret_cast<const WindowEvent*>(event), mEventType));

    if ((window->flags() && Qt::WindowTransparentForInput) && window->parent()) {
        QCoreApplication::postEvent(this, new UbuntuEvent(
                    static_cast<UbuntuWindow*>(platformWindow->QPlatformWindow::parent()),
                    reinterpret_cast<const WindowEvent*>(event), mEventType));
    }
}

void UbuntuInput::dispatchMotionEvent(QWindow* window, const void* ev)
{
    const WindowEvent* event = reinterpret_cast<const WindowEvent*>(ev);

    #if (LOG_EVENTS != 0)
    // Motion event logging.
    LOG("MOTION device_id:%d source_id:%d action:%d flags:%d meta_state:%d edge_flags:%d "
            "button_state:%d x_offset:%.2f y_offset:%.2f x_precision:%.2f y_precision:%.2f "
            "down_time:%lld event_time:%lld pointer_count:%d {", event->motion.device_id,
            event->motion.source_id, event->motion.action,
            event->motion.flags, event->motion.meta_state,
            event->motion.edge_flags, event->motion.button_state,
            event->motion.x_offset, event->motion.y_offset,
            event->motion.x_precision, event->motion.y_precision,
            event->motion.down_time, event->motion.event_time,
            event->motion.pointer_count);
    for (size_t i = 0; i < event->motion.pointer_count; i++) {
        LOG("  id:%d x:%.2f y:%.2f rx:%.2f ry:%.2f maj:%.2f min:%.2f sz:%.2f press:%.2f",
                event->motion.pointer_coordinates[i].id,
                event->motion.pointer_coordinates[i].x,
                event->motion.pointer_coordinates[i].y,
                event->motion.pointer_coordinates[i].raw_x,
                event->motion.pointer_coordinates[i].raw_y,
                event->motion.pointer_coordinates[i].touch_major,
                event->motion.pointer_coordinates[i].touch_minor,
                event->motion.pointer_coordinates[i].size,
                event->motion.pointer_coordinates[i].pressure
                // event->motion.pointer_coordinates[i].orientation  -> Always 0.0.
           );
    }
    LOG("}");
    #endif

    // FIXME(loicm) Max pressure is device specific. That one is for the Samsung Galaxy Nexus. That
    //     needs to be fixed as soon as the compat input lib adds query support.
    const float kMaxPressure = 1.28;
    const QRect kWindowGeometry = window->geometry();
    QList<QWindowSystemInterface::TouchPoint> touchPoints;


    // TODO: Is it worth setting the Qt::TouchPointStationary ones? Currently they are left
    //       as Qt::TouchPointMoved
    const int kPointerCount = event->motion.pointer_count;
    for (int i = 0; i < kPointerCount; ++i) {
        QWindowSystemInterface::TouchPoint touchPoint;

        const float kX = event->motion.pointer_coordinates[i].raw_x + kWindowGeometry.x();
        const float kY = event->motion.pointer_coordinates[i].raw_y + kWindowGeometry.y(); // see bug lp:1346633 workaround comments elsewhere
        const float kW = event->motion.pointer_coordinates[i].touch_major;
        const float kH = event->motion.pointer_coordinates[i].touch_minor;
        const float kP = event->motion.pointer_coordinates[i].pressure;
        touchPoint.id = event->motion.pointer_coordinates[i].id;
        touchPoint.normalPosition = QPointF(kX / kWindowGeometry.width(), kY / kWindowGeometry.height());
        touchPoint.area = QRectF(kX - (kW / 2.0), kY - (kH / 2.0), kW, kH);
        touchPoint.pressure = kP / kMaxPressure;
        touchPoint.state = Qt::TouchPointMoved;

        touchPoints.append(touchPoint);
    }

    switch (event->motion.action & U_MOTION_ACTION_MASK) {
    case U_MOTION_ACTION_MOVE:
        // No extra work needed.
        break;

    case U_MOTION_ACTION_DOWN:
        touchPoints[0].state = Qt::TouchPointPressed;
        break;

    case U_MOTION_ACTION_UP:
        touchPoints[0].state = Qt::TouchPointReleased;
        break;

    case U_MOTION_ACTION_POINTER_DOWN: {
        const int index = (event->motion.action & U_MOTION_ACTION_POINTER_INDEX_MASK) >>
            U_MOTION_ACTION_POINTER_INDEX_SHIFT;
        touchPoints[index].state = Qt::TouchPointPressed;
        break;
    }

    case U_MOTION_ACTION_CANCEL:
    case U_MOTION_ACTION_POINTER_UP: {
        const int index = (event->motion.action & U_MOTION_ACTION_POINTER_INDEX_MASK) >>
            U_MOTION_ACTION_POINTER_INDEX_SHIFT;
        touchPoints[index].state = Qt::TouchPointReleased;
        break;
    }

    case U_MOTION_ACTION_OUTSIDE:
    case U_MOTION_ACTION_HOVER_MOVE:
    case U_MOTION_ACTION_SCROLL:
    case U_MOTION_ACTION_HOVER_ENTER:
    case U_MOTION_ACTION_HOVER_EXIT:
    default:
        DLOG("unhandled motion event action %d", event->motion.action & U_MOTION_ACTION_MASK);
    }

    QWindowSystemInterface::handleTouchEvent(window, event->motion.event_time / 1000000,
            mTouchDevice, touchPoints);
}

static uint32_t translateKeysym(uint32_t sym, char *string, size_t size)
{
    Q_UNUSED(size);
    string[0] = '\0';

    if (sym >= XKB_KEY_F1 && sym <= XKB_KEY_F35)
        return Qt::Key_F1 + (int(sym) - XKB_KEY_F1);

    for (int i = 0; KeyTable[i]; i += 2) {
        if (sym == KeyTable[i])
            return KeyTable[i + 1];
    }

    string[0] = sym;
    string[1] = '\0';
    return toupper(sym);
}

void UbuntuInput::dispatchKeyEvent(QWindow* window, const void* ev)
{
    const WindowEvent* event = reinterpret_cast<const WindowEvent*>(ev);

    #if (LOG_EVENTS != 0)
    // Key event logging.
    LOG("KEY device_id:%d source_id:%d action:%d flags:%d meta_state:%d key_code:%d "
            "scan_code:%d repeat_count:%d down_time:%lld event_time:%lld is_system_key:%d",
            event->key.device_id, event->key.source_id,
            event->key.action, event->key.flags, event->key.meta_state,
            event->key.key_code, event->key.scan_code,
            event->key.repeat_count, event->key.down_time,
            event->key.event_time, event->key.is_system_key);
    #endif

    ulong timestamp = event->key.event_time / 1000000;
    xkb_keysym_t xk_sym = (xkb_keysym_t)event->key.key_code;

    // Key modifier and unicode index mapping.
    const int kMetaState = event->key.meta_state;
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    if (kMetaState & U_KEY_MODIFIER_SHIFT) {
        modifiers |= Qt::ShiftModifier;
    }
    if (kMetaState & U_KEY_MODIFIER_CTRL) {
        modifiers |= Qt::ControlModifier;
    }
    if (kMetaState & U_KEY_MODIFIER_ALT) {
        modifiers |= Qt::AltModifier;
    }
    if (kMetaState & U_KEY_MODIFIER_META) {
        modifiers |= Qt::MetaModifier;
    }

    QEvent::Type keyType = event->key.action == U_KEY_ACTION_DOWN ? QEvent::KeyPress : QEvent::KeyRelease;

    char s[2];
    int sym = translateKeysym(xk_sym, s, sizeof(s));
    QString text = QString::fromLatin1(s);

    QPlatformInputContext* context = QGuiApplicationPrivate::platformIntegration()->inputContext();
    if (context) {
        QKeyEvent qKeyEvent(keyType, sym, modifiers, text);
        qKeyEvent.setTimestamp(timestamp);
        if (context->filterEvent(&qKeyEvent)) {
            DLOG("key event filtered out by input context");
            return;
        }
    }

    QWindowSystemInterface::handleKeyEvent(window, timestamp, keyType, sym, modifiers, text);
}
