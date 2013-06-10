// This file is part of QtUbuntu, a set of Qt components for Ubuntu.
// Copyright © 2013 Canonical Ltd.
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 3, as published by
// the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
// SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "input.h"
#include "integration.h"
#include "native_interface.h"
#include "logging.h"
#if !defined(QT_NO_DEBUG)
#include <QtCore/QThread>
#endif
#include <QtCore/qglobal.h>
#include <QtCore/QCoreApplication>
#include <private/qguiapplication_p.h>
#include <qpa/qplatforminputcontext.h>
#include <ubuntu/application/ui/input/event.h>

#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-keysyms.h>

#define LOG_EVENTS 0

// XKB Keysyms which do not map directly to Qt types (i.e. Unicode points)
static const uint32_t KeyTbl[] = {
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

    0,                          0
};

class QUbuntuBaseEvent : public QEvent {
 public:
  QUbuntuBaseEvent(QWindow* window, const Event* event, QEvent::Type type)
      : QEvent(type)
      , window_(window) {
    memcpy(&nativeEvent_, event, sizeof(Event));
  }
  QWindow* window_;
  Event nativeEvent_;
};

QUbuntuBaseInput::QUbuntuBaseInput(QUbuntuBaseIntegration* integration, int maxPointCount)
    : integration_(integration)
    , eventFilterType_(static_cast<QUbuntuBaseNativeInterface*>(
        integration->nativeInterface())->genericEventFilterType())
    , eventType_(static_cast<QEvent::Type>(QEvent::registerEventType())) {
  DASSERT(maxPointCount > 0);

  // Initialize touch device.
  touchDevice_ = new QTouchDevice();
  touchDevice_->setType(QTouchDevice::TouchScreen);
  touchDevice_->setCapabilities(
      QTouchDevice::Position | QTouchDevice::Area | QTouchDevice::Pressure |
      QTouchDevice::NormalizedPosition);
  QWindowSystemInterface::registerTouchDevice(touchDevice_);

  // Initialize touch points.
  touchPoints_.reserve(maxPointCount);
  for (int i = 0; i < maxPointCount; i++) {
    QWindowSystemInterface::TouchPoint tp;
    tp.id = i;
    tp.state = Qt::TouchPointReleased;
    touchPoints_ << tp;
  }

  DLOG("QUbuntuBaseInput::QUbuntuBaseInput (this=%p, integration=%p, maxPointCount=%d)", this,
       integration, maxPointCount);
}

QUbuntuBaseInput::~QUbuntuBaseInput() {
  DLOG("QUbuntuBaseInput::~QUbuntuBaseInput");
  // touchDevice_ isn't cleaned up on purpose as it crashes or asserts on "Bus Error".
  touchPoints_.clear();
}

void QUbuntuBaseInput::customEvent(QEvent* event) {
  DLOG("QUbuntuBaseInput::customEvent (this=%p, event=%p)", this, event);
  DASSERT(QThread::currentThread() == thread());
  QUbuntuBaseEvent* ubuntuEvent = static_cast<QUbuntuBaseEvent*>(event);

  // Event filtering.
  long result;
  if (QWindowSystemInterface::handleNativeEvent(
          ubuntuEvent->window_, eventFilterType_, &ubuntuEvent->nativeEvent_, &result) == true) {
    DLOG("event filtered out by native interface");
    return;
  }

  // Event dispatching.
  switch (ubuntuEvent->nativeEvent_.type) {
    case MOTION_EVENT_TYPE: {
      dispatchMotionEvent(ubuntuEvent->window_, &ubuntuEvent->nativeEvent_);
      break;
    }
    case KEY_EVENT_TYPE: {
      dispatchKeyEvent(ubuntuEvent->window_, &ubuntuEvent->nativeEvent_);
      break;
    }
    case HW_SWITCH_EVENT_TYPE: {
      dispatchHWSwitchEvent(ubuntuEvent->window_, &ubuntuEvent->nativeEvent_);
      break;
    }
    default: {
      DLOG("unhandled event type %d", ubuntuEvent->nativeEvent_.type);
    }
  }
}

void QUbuntuBaseInput::postEvent(QWindow* window, const void* event) {
  DLOG("QUbuntuBaseInput::postEvent (this=%p, window=%p, event=%p)", this, window, event);
  QCoreApplication::postEvent(this, new QUbuntuBaseEvent(
      window, reinterpret_cast<const Event*>(event), eventType_));
}

void QUbuntuBaseInput::dispatchMotionEvent(QWindow* window, const void* ev) {
  DLOG("QUbuntuBaseInput::dispatchMotionEvent (this=%p, window=%p, event=%p)", this, window, ev);
  const Event* event = reinterpret_cast<const Event*>(ev);

#if (LOG_EVENTS != 0)
  // Motion event logging.
  LOG("MOTION device_id:%d source_id:%d action:%d flags:%d meta_state:%d edge_flags:%d "
      "button_state:%d x_offset:%.2f y_offset:%.2f x_precision:%.2f y_precision:%.2f "
      "down_time:%lld event_time:%lld pointer_count:%d {", event->device_id,
      event->source_id, event->action, event->flags, event->meta_state,
      event->details.motion.edge_flags, event->details.motion.button_state,
      event->details.motion.x_offset, event->details.motion.y_offset,
      event->details.motion.x_precision, event->details.motion.y_precision,
      event->details.motion.down_time, event->details.motion.event_time,
      event->details.motion.pointer_count);
  for (size_t i = 0; i < event->details.motion.pointer_count; i++) {
    LOG("  id:%d x:%.2f y:%.2f rx:%.2f ry:%.2f maj:%.2f min:%.2f sz:%.2f press:%.2f",
        event->details.motion.pointer_coordinates[i].id,
        event->details.motion.pointer_coordinates[i].x,
        event->details.motion.pointer_coordinates[i].y,
        event->details.motion.pointer_coordinates[i].raw_x,
        event->details.motion.pointer_coordinates[i].raw_y,
        event->details.motion.pointer_coordinates[i].touch_major,
        event->details.motion.pointer_coordinates[i].touch_minor,
        event->details.motion.pointer_coordinates[i].size,
        event->details.motion.pointer_coordinates[i].pressure
        // event->details.motion.pointer_coordinates[i].orientation  -> Always 0.0.
        );
  }
  LOG("}");
#endif

  // FIXME(loicm) Max pressure is device specific. That one is for the Samsung Galaxy Nexus. That
  //     needs to be fixed as soon as the compat input lib adds query support.
  const float kMaxPressure = 1.28;
  const QRect kWindowGeometry = window->geometry();

  switch (event->action & U_MOTION_ACTION_MASK) {
    case U_MOTION_ACTION_MOVE: {
      int eventIndex = 0;
      const int kPointerCount = event->details.motion.pointer_count;
      for (int touchIndex = 0; eventIndex < kPointerCount; touchIndex++) {
        if (touchPoints_[touchIndex].state != Qt::TouchPointReleased) {
          const float kX = event->details.motion.pointer_coordinates[eventIndex].raw_x;
          const float kY = event->details.motion.pointer_coordinates[eventIndex].raw_y;
          const float kW = event->details.motion.pointer_coordinates[eventIndex].touch_major;
          const float kH = event->details.motion.pointer_coordinates[eventIndex].touch_minor;
          const float kP = event->details.motion.pointer_coordinates[eventIndex].pressure;
          touchPoints_[touchIndex].area = QRectF(kX - (kW / 2.0), kY - (kH / 2.0), kW, kH);
          touchPoints_[touchIndex].normalPosition = QPointF(
              kX / kWindowGeometry.width(), kY / kWindowGeometry.height());
          touchPoints_[touchIndex].pressure = kP / kMaxPressure;
          touchPoints_[touchIndex].state = Qt::TouchPointMoved;
          eventIndex++;
        }
      }
      break;
    }

    case U_MOTION_ACTION_DOWN: {
      const int kTouchIndex = event->details.motion.pointer_coordinates[0].id;
      const float kX = event->details.motion.pointer_coordinates[0].raw_x;
      const float kY = event->details.motion.pointer_coordinates[0].raw_y;
      const float kW = event->details.motion.pointer_coordinates[0].touch_major;
      const float kH = event->details.motion.pointer_coordinates[0].touch_minor;
      const float kP = event->details.motion.pointer_coordinates[0].pressure;
      touchPoints_[kTouchIndex].state = Qt::TouchPointPressed;
      touchPoints_[kTouchIndex].area = QRectF(kX - (kW / 2.0), kY - (kH / 2.0), kW, kH);
      touchPoints_[kTouchIndex].normalPosition = QPointF(
          kX / kWindowGeometry.width(), kY / kWindowGeometry.height());
      touchPoints_[kTouchIndex].pressure = kP / kMaxPressure;
      break;
    }

    case U_MOTION_ACTION_UP: {
      const int kTouchIndex = event->details.motion.pointer_coordinates[0].id;
      touchPoints_[kTouchIndex].state = Qt::TouchPointReleased;
      break;
    }

    case U_MOTION_ACTION_POINTER_DOWN: {
      const int eventIndex = (event->action & U_MOTION_ACTION_POINTER_INDEX_MASK) >>
          U_MOTION_ACTION_POINTER_INDEX_SHIFT;
      const int kTouchIndex = event->details.motion.pointer_coordinates[eventIndex].id;
      const float kX = event->details.motion.pointer_coordinates[eventIndex].raw_x;
      const float kY = event->details.motion.pointer_coordinates[eventIndex].raw_y;
      const float kW = event->details.motion.pointer_coordinates[eventIndex].touch_major;
      const float kH = event->details.motion.pointer_coordinates[eventIndex].touch_minor;
      const float kP = event->details.motion.pointer_coordinates[eventIndex].pressure;
      touchPoints_[kTouchIndex].state = Qt::TouchPointPressed;
      touchPoints_[kTouchIndex].area = QRectF(kX - (kW / 2.0), kY - (kH / 2.0), kW, kH);
      touchPoints_[kTouchIndex].normalPosition = QPointF(
          kX / kWindowGeometry.width(), kY / kWindowGeometry.height());
      touchPoints_[kTouchIndex].pressure = kP / kMaxPressure;
      break;
    }

    case U_MOTION_ACTION_CANCEL:
    case U_MOTION_ACTION_POINTER_UP: {
      const int kEventIndex = (event->action & U_MOTION_ACTION_POINTER_INDEX_MASK) >>
          U_MOTION_ACTION_POINTER_INDEX_SHIFT;
      const int kTouchIndex = event->details.motion.pointer_coordinates[kEventIndex].id;
      touchPoints_[kTouchIndex].state = Qt::TouchPointReleased;
      break;
    }

    case U_MOTION_ACTION_OUTSIDE:
    case U_MOTION_ACTION_HOVER_MOVE:
    case U_MOTION_ACTION_SCROLL:
    case U_MOTION_ACTION_HOVER_ENTER:
    case U_MOTION_ACTION_HOVER_EXIT:
    default: {
      DLOG("unhandled motion event action %d", event->action & U_MOTION_ACTION_MASK);
    }
  }

  // Touch event propagation.
  handleTouchEvent(window, event->details.motion.event_time / 1000000, touchDevice_, touchPoints_);
}

void QUbuntuBaseInput::handleTouchEvent(
    QWindow* window, ulong timestamp, QTouchDevice* device,
    const QList<struct QWindowSystemInterface::TouchPoint> &points) {
  DLOG("QUbuntuBaseInput::handleTouchEvent (this=%p, window=%p, timestamp=%lu, device=%p)",
       this, window, timestamp, device);
  QWindowSystemInterface::handleTouchEvent(window, timestamp, device, points);
}

namespace
{
static uint32_t translateKey(uint32_t sym, char *string, size_t size)
{
    Q_UNUSED(size);
    string[0] = '\0';

    if (sym >= XKB_KEY_F1 && sym <= XKB_KEY_F35)
        return Qt::Key_F1 + (int(sym) - XKB_KEY_F1);

    for (int i = 0; KeyTbl[i]; i += 2)
        if (sym == KeyTbl[i])
            return KeyTbl[i + 1];

    string[0] = sym;
    string[1] = '\0';
    return toupper(sym);
}
}

void QUbuntuBaseInput::dispatchKeyEvent(QWindow* window, const void* ev) {
  DLOG("QUbuntuBaseInput::dispatchKeyEvent (this=%p, window=%p, event=%p)", this, window, ev);
  const Event* event = reinterpret_cast<const Event*>(ev);

#if (LOG_EVENTS != 0)
  // Key event logging.
  LOG("KEY device_id:%d source_id:%d action:%d flags:%d meta_state:%d key_code:%d "
      "scan_code:%d repeat_count:%d down_time:%lld event_time:%lld is_system_key:%d",
      event->device_id, event->source_id, event->action, event->flags, event->meta_state,
      event->details.key.key_code, event->details.key.scan_code,
      event->details.key.repeat_count, event->details.key.down_time,
      event->details.key.event_time, event->details.key.is_system_key);
#endif

  ulong timestamp = event->details.key.event_time / 1000000;
  xkb_keysym_t xk_sym = (xkb_keysym_t)event->details.key.key_code;

  // Key modifier and unicode index mapping.
  const int kMetaState = event->meta_state;
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

  QEvent::Type keyType = event->action == U_KEY_ACTION_DOWN ? QEvent::KeyPress : QEvent::KeyRelease;

  char s[2];
  int sym = translateKey(xk_sym, s, sizeof(s));
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

  handleKeyEvent(window, timestamp, keyType, sym, modifiers, text);
}

void QUbuntuBaseInput::handleKeyEvent(
    QWindow* window, ulong timestamp, QEvent::Type type, int key, Qt::KeyboardModifiers modifiers,
    const QString& text) {
  DLOG("QUbuntuBaseInput::handleKeyEvent (this=%p window=%p, timestamp=%lu, type=%d, key=%d, "
       "modifiers=%d, text='%s')", this, window, timestamp, static_cast<int>(type), key,
       static_cast<int>(modifiers), text.toUtf8().data());
  QWindowSystemInterface::handleKeyEvent(window, timestamp, type, key, modifiers, text);
}

void QUbuntuBaseInput::dispatchHWSwitchEvent(QWindow* window, const void* ev) {
  Q_UNUSED(window);
  Q_UNUSED(ev);
  DLOG("QUbuntuBaseInput::dispatchSwitchEvent (this=%p, window=%p, event=%p)", this, window, ev);

#if (LOG_EVENTS != 0)
  // HW switch event logging.
  const Event* event = reinterpret_cast<const Event*>(ev);
  LOG("HWSWITCH device_id:%d source_id:%d action:%d flags:%d meta_state:%d event_time:%lld "
      "policy_flags:%u switch_values:%d switch_mask:%d", event->device_id, event->source_id,
      event->action, event->flags, event->meta_state, event->details.hw_switch.event_time,
      event->details.hw_switch.policy_flags, event->details.hw_switch.switch_values,
      event->details.hw_switch.switch_mask);
#endif

  // FIXME(loicm) Not sure how to interpret that kind of event.
  DLOG("hw switch events are not handled");
}
