// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "qhybrisinput.h"
#include "qhybriswindow.h"
#include "qhybrisintegration.h"
#include "qhybrislogging.h"
#include <QtCore/qglobal.h>
#include <input/input_stack_compatibility_layer_flags_motion.h>
#include <climits>

#define LOG_EVENTS 0

namespace {

void handleMotionEvent(Event* event, QHybrisInput* input) {
  DLOG("handleMotionEvent (event=%p, input=%p)", event, input);
  // FIXME(loicm) We need to be able to retrieve the window from an event in order to support
  //     multiple surfaces.
  QPlatformWindow* window = input->integration_->platformWindow();
  if (!window)
    return;

  // FIXME(loicm) Max pressure is device specific. That one is for the Samsung Galaxy Nexus. That
  //     needs to be fixed as soon as the compat input lib adds query support.
  const float kMaxPressure = 1.28;
  const QRect kWindowGeometry = window->geometry();
  QList<QWindowSystemInterface::TouchPoint>& touchPoints = input->touchPoints_;

  switch (event->action & ISCL_MOTION_EVENT_ACTION_MASK) {
    case ISCL_MOTION_EVENT_ACTION_MOVE: {
      int eventIndex = 0;
      const int kPointerCount = event->details.motion.pointer_count;
      for (int touchIndex = 0; eventIndex < kPointerCount; touchIndex++) {
        if (touchPoints[touchIndex].state != Qt::TouchPointReleased) {
          const float kX = event->details.motion.pointer_coordinates[eventIndex].x;
          const float kY = event->details.motion.pointer_coordinates[eventIndex].y;
          const float kW = event->details.motion.pointer_coordinates[eventIndex].touch_major;
          const float kH = event->details.motion.pointer_coordinates[eventIndex].touch_minor;
          const float kP = event->details.motion.pointer_coordinates[eventIndex].pressure;
          touchPoints[touchIndex].area = QRectF(kX - (kW / 2.0), kY - (kH / 2.0), kW, kH);
          touchPoints[touchIndex].normalPosition = QPointF(
              kX / kWindowGeometry.width(), kY / kWindowGeometry.height());
          touchPoints[touchIndex].pressure = kP / kMaxPressure;
          touchPoints[touchIndex].state = Qt::TouchPointMoved;
          eventIndex++;
        }
      }
      break;
    }

    case ISCL_MOTION_EVENT_ACTION_DOWN: {
      const int kTouchIndex = event->details.motion.pointer_coordinates[0].id;
      const float kX = event->details.motion.pointer_coordinates[0].x;
      const float kY = event->details.motion.pointer_coordinates[0].y;
      const float kW = event->details.motion.pointer_coordinates[0].touch_major;
      const float kH = event->details.motion.pointer_coordinates[0].touch_minor;
      const float kP = event->details.motion.pointer_coordinates[0].pressure;
      touchPoints[kTouchIndex].state = Qt::TouchPointPressed;
      touchPoints[kTouchIndex].area = QRectF(kX - (kW / 2.0), kY - (kH / 2.0), kW, kH);
      touchPoints[kTouchIndex].normalPosition = QPointF(
          kX / kWindowGeometry.width(), kY / kWindowGeometry.height());
      touchPoints[kTouchIndex].pressure = kP / kMaxPressure;
      break;
    }

    case ISCL_MOTION_EVENT_ACTION_UP: {
      const int kTouchIndex = event->details.motion.pointer_coordinates[0].id;
      touchPoints[kTouchIndex].state = Qt::TouchPointReleased;
      break;
    }

    case ISCL_MOTION_EVENT_ACTION_POINTER_DOWN: {
      const int eventIndex = (event->action & ISCL_MOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
          ISCL_MOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
      const int kTouchIndex = event->details.motion.pointer_coordinates[eventIndex].id;
      const float kX = event->details.motion.pointer_coordinates[eventIndex].x;
      const float kY = event->details.motion.pointer_coordinates[eventIndex].y;
      const float kW = event->details.motion.pointer_coordinates[eventIndex].touch_major;
      const float kH = event->details.motion.pointer_coordinates[eventIndex].touch_minor;
      const float kP = event->details.motion.pointer_coordinates[eventIndex].pressure;
      touchPoints[kTouchIndex].state = Qt::TouchPointPressed;
      touchPoints[kTouchIndex].area = QRectF(kX - (kW / 2.0), kY - (kH / 2.0), kW, kH);
      touchPoints[kTouchIndex].normalPosition = QPointF(
          kX / kWindowGeometry.width(), kY / kWindowGeometry.height());
      touchPoints[kTouchIndex].pressure = kP / kMaxPressure;
      break;
    }

    case ISCL_MOTION_EVENT_ACTION_POINTER_UP: {
      const int kEventIndex = (event->action & ISCL_MOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
          ISCL_MOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
      const int kTouchIndex = event->details.motion.pointer_coordinates[kEventIndex].id;
      touchPoints[kTouchIndex].state = Qt::TouchPointReleased;
      break;
    }

    case ISCL_MOTION_EVENT_ACTION_CANCEL:
    case ISCL_MOTION_EVENT_ACTION_OUTSIDE:
    case ISCL_MOTION_EVENT_ACTION_HOVER_MOVE:
    case ISCL_MOTION_EVENT_ACTION_SCROLL:
    case ISCL_MOTION_EVENT_ACTION_HOVER_ENTER:
    case ISCL_MOTION_EVENT_ACTION_HOVER_EXIT:
    default: {
      // FIXME(loicm) Never received such values yet. Let's see if people get these. Switch to
      //     DNOT_REACHED() before releasing.
      NOT_REACHED();
    }
  }

  QWindowSystemInterface::handleTouchEvent(
      window->window(), event->details.motion.event_time, input->touchDevice_, input->touchPoints_);
}

void handleKeyEvent(Event* event, QHybrisInput* input) {
  DLOG("handleKeyEvent (event=%p, input=%p)", event, input);
  // FIXME(loicm) We need to be able to retrieve the window from an event in order to support
  //     multiple surfaces.
  QPlatformWindow* window = input->integration_->platformWindow();
  if (!window)
    return;

  int key = 0;
#if defined(QHYBRIS_DEBUG)
  const char* keyName;
#endif

  switch (event->details.key.key_code) {
    case 24: {
      key = Qt::Key_VolumeUp;
#if defined(QHYBRIS_DEBUG)
      keyName = static_cast<const char*>("VolumeUp");
#endif
      break;
    }
    case 25: {
      key = Qt::Key_VolumeDown;
#if defined(QHYBRIS_DEBUG)
      keyName = static_cast<const char*>("VolumeDown");
#endif
      break;
    }
    case 26: {
      key = Qt::Key_PowerOff;
#if defined(QHYBRIS_DEBUG)
      keyName = static_cast<const char*>("PowerOff");
#endif
      break;
    }
    default: {
      return;
    }
  }

  QEvent::Type type = (event->action == 0) ? QEvent::KeyPress : QEvent::KeyRelease;
  QWindowSystemInterface::handleExtendedKeyEvent(
      window->window(), event->details.key.event_time, type, key, Qt::NoModifier,
      event->details.key.scan_code, event->details.key.key_code, 0);

#if defined(QHYBRIS_DEBUG)
  const char* stateName[] = { "pressed", "released" };
  LOG("key state: %s %s", keyName, stateName[event->action]);
#endif
}

void hybrisEventCallback(Event* event, void* context) {
  DLOG("hybrisEventCallback (event=%p, context=%p)", event, context);
  QHybrisInput* input = static_cast<QHybrisInput*>(context);

  if (input->stopping_.testAndSetRelease(1, 1))
    return;

  switch (event->type) {
    case MOTION_EVENT_TYPE: {
#if (LOG_EVENTS == 1)
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
        LOG("  id:%d x:%.2f y:%.2f major:%.2f minor:%.2f size:%.2f pressure:%.2f",
            event->details.motion.pointer_coordinates[i].id,
            event->details.motion.pointer_coordinates[i].x,
            event->details.motion.pointer_coordinates[i].y,
            // event->details.motion.pointer_coordinates[i].raw_x,
            // event->details.motion.pointer_coordinates[i].raw_y,
            event->details.motion.pointer_coordinates[i].touch_major,
            event->details.motion.pointer_coordinates[i].touch_minor,
            event->details.motion.pointer_coordinates[i].size,
            event->details.motion.pointer_coordinates[i].pressure
            // event->details.motion.pointer_coordinates[i].orientation  -> Always 0.0.
            );
      }
      LOG("}");
#endif
      handleMotionEvent(event, input);
      break;
    }

    case KEY_EVENT_TYPE: {
#if (LOG_EVENTS == 1)
      LOG("KEY device_id:%d source_id:%d action:%d flags:%d meta_state:%d key_code:%d "
          "scan_code:%d repeat_count:%d down_time:%lld event_time:%lld is_system_key:%d",
          event->device_id, event->source_id, event->action, event->flags, event->meta_state,
          event->details.key.key_code, event->details.key.scan_code,
          event->details.key.repeat_count, event->details.key.down_time,
          event->details.key.event_time, event->details.key.is_system_key);
#endif
      handleKeyEvent(event, input);
      break;
    }

    case HW_SWITCH_EVENT_TYPE: {
#if (LOG_EVENTS == 1)
      LOG("HW_SWITCH device_id:%d source_id:%d action:%d flags:%d meta_state:%d event_time:%lld "
          "policy_flags:%u switch_code:%d switch_value:%d", event->device_id, event->source_id,
          event->action, event->flags, event->meta_state, event->details.hw_switch.event_time,
          event->details.hw_switch.policy_flags, event->details.hw_switch.switch_code,
          event->details.hw_switch.switch_value);
#endif
      // FIXME(loicm) Haven't received such type of events yet, not sure how to interpret them.
      break;
    }

    default: {
      break;
    }
  }
}

}  // Anonymous namespace.

QHybrisInput::QHybrisInput(QHybrisIntegration* integration)
    : touchDevice_(new QTouchDevice())
    , integration_(integration)
    , stopping_(0) {
  // Initialize touch points.
  touchPoints_.reserve(MAX_POINTER_COUNT);
  for (unsigned int i = 0; i < MAX_POINTER_COUNT; i++) {
    QWindowSystemInterface::TouchPoint tp;
    tp.id = i;
    tp.state = Qt::TouchPointReleased;
    touchPoints_ << tp;
  }

  // Initialize touch device.
  touchDevice_->setType(QTouchDevice::TouchScreen);
  touchDevice_->setCapabilities(
      QTouchDevice::Position | QTouchDevice::Area | QTouchDevice::Pressure |
      QTouchDevice::NormalizedPosition);
  QWindowSystemInterface::registerTouchDevice(touchDevice_);

  // Initialize input stack.
  config_.enable_touch_point_visualization = false;
  config_.default_layer_for_touch_point_visualization = INT_MAX - 1;
  listener_.on_new_event = hybrisEventCallback;
  listener_.context = this;
  DLOG("initializing input stack");
  android_input_stack_initialize(&listener_, &config_);
  DLOG("starting input stack");
  android_input_stack_start();

  DLOG("QHybrisInput::QHybrisInput (this=%p)", this);
}

QHybrisInput::~QHybrisInput() {
  DLOG("QHybrisInput::~QHybrisInput");

  // Stop input stack.
  stopping_.fetchAndStoreRelease(1);
  DLOG("stopping input stack");
  android_input_stack_stop();
  DLOG("shutting down input stack");
  android_input_stack_shutdown();

  // Clean up touch device and touch points.
  // FIXME(loicm) Generates a "Bus Error" assertion.
  // delete touchDevice_;
  touchPoints_.clear();
}
