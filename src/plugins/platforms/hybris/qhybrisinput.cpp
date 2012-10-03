// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "qhybrisinput.h"
#include "qhybriswindow.h"
#include "qhybrisintegration.h"
#include "qhybrislogging.h"
#include <QtCore/qglobal.h>
#include <climits>

#define LOG_EVENTS 0

namespace {

void handleMotionEvent(Event* event, QHybrisInput* input) {
  // FIXME(loicm) We need to be able to retrieve the window from an event in order to support
  //     multiple surfaces.
  QPlatformWindow* window = input->mIntegration->platformWindow();
  if (!window)
    return;

  QRect geometry = window->geometry();
  QList<QWindowSystemInterface::TouchPoint>& touchPoints = input->mTouchPoints;
  const unsigned int action = event->action;
  const float maxPressure = 1.28;

  if (action == 2) {
    // Motion event.
    int index = 0;
    const int count = MAX_POINTER_COUNT;
    for (int i = 0; i < count; i++) {
      if (touchPoints[i].state != Qt::TouchPointReleased) {
        const float x = event->details.motion.pointer_coordinates[index].x;
        const float y = event->details.motion.pointer_coordinates[index].y;
        if (touchPoints[i].area.center() != QPoint(x, y)) {
          const float w = event->details.motion.pointer_coordinates[index].touch_major;
          const float h = event->details.motion.pointer_coordinates[index].touch_minor;
          const float p = event->details.motion.pointer_coordinates[index].pressure / maxPressure;
          touchPoints[i].area = QRectF(x - (w / 2.0), y - (h / 2.0), w, h);
          touchPoints[i].normalPosition = QPointF(x / geometry.width(), y / geometry.height());
          touchPoints[i].pressure = p;
          touchPoints[i].state = Qt::TouchPointMoved;
        } else {
          touchPoints[i].state = Qt::TouchPointStationary;
        }
        index++;
      }
    }

  } else if (~action & 0x4) {
    // Single touch event.
    const float x = event->details.motion.pointer_coordinates[0].x;
    const float y = event->details.motion.pointer_coordinates[0].y;
    const float w = event->details.motion.pointer_coordinates[0].touch_major;
    const float h = event->details.motion.pointer_coordinates[0].touch_minor;
    const float p = event->details.motion.pointer_coordinates[0].pressure / maxPressure;
    if (action == 0) {
      unsigned int index_ = 0;
      while (touchPoints[index_].state != Qt::TouchPointReleased && index_ < MAX_POINTER_COUNT)
        index_++;
      touchPoints[index_].state = Qt::TouchPointPressed;
      touchPoints[index_].area = QRectF(x - (w / 2.0), y - (h / 2.0), w, h);
      touchPoints[index_].normalPosition = QPointF(x / geometry.width(), y / geometry.height());
      touchPoints[index_].pressure = p;
    } else if (action == 1) {
      unsigned int index_ = 0;
      while (touchPoints[index_].state == Qt::TouchPointReleased && index_ < MAX_POINTER_COUNT)
        index_++;
      touchPoints[index_].state = Qt::TouchPointReleased;
    }

  } else {
    // Multi touch event.
    const int index = (action >> 8) & 0xff;
    const int pressed = action & 0x1;
    if (pressed) {
      unsigned int index_ = 0;
      while (touchPoints[index_].state != Qt::TouchPointReleased && index_ < MAX_POINTER_COUNT)
        index_++;
      const float x = event->details.motion.pointer_coordinates[index].x;
      const float y = event->details.motion.pointer_coordinates[index].y;
      const float w = event->details.motion.pointer_coordinates[index].touch_major;
      const float h = event->details.motion.pointer_coordinates[index].touch_minor;
      const float p = event->details.motion.pointer_coordinates[index].pressure / maxPressure;
      touchPoints[index_].state = Qt::TouchPointPressed;
      touchPoints[index_].area = QRectF(x - (w / 2.0), y - (h / 2.0), w, h);
      touchPoints[index_].normalPosition = QPointF(x / geometry.width(), y / geometry.height());
      touchPoints[index_].pressure = p;
    } else {
      unsigned int index_ = index;
      while (touchPoints[index_].state == Qt::TouchPointReleased && index_ < MAX_POINTER_COUNT)
        index_++;
      touchPoints[index_].state = Qt::TouchPointReleased;
    }
  }

  QWindowSystemInterface::handleTouchEvent(
      window->window(), event->details.motion.event_time, input->mTouchDevice, input->mTouchPoints);

  DLOG("touch states: { %d, %d, %d, %d, %d, %d, %d, %d }", touchPoints[0].state,
       touchPoints[1].state, touchPoints[2].state, touchPoints[3].state, touchPoints[4].state,
       touchPoints[5].state, touchPoints[6].state, touchPoints[7].state);
}

void hybrisEventCallback(Event* event, void* context) {
  DLOG("hybrisEventCallback (event=%p, context=%p)", event, context);
  QHybrisInput* input = static_cast<QHybrisInput*>(context);

  if (input->mStopping.testAndSetRelease(1, 1))
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
        LOG("  x:%.2f y:%.2f major:%.2f minor:%.2f size:%.2f pressure:%.2f",
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
      LOG("}\n");
#endif
      handleMotionEvent(event, input);
      break;
    }

    case KEY_EVENT_TYPE: {
#if (LOG_EVENTS == 1)
      LOG("KEY device_id:%d source_id:%d action:%d flags:%d meta_state:%d key_code:%d "
          "scan_code:%d repeat_count:%d down_time:%lld event_time:%lld is_system_key:%d\n",
          event->device_id, event->source_id, event->action, event->flags, event->meta_state,
          event->details.key.key_code, event->details.key.scan_code,
          event->details.key.repeat_count, event->details.key.down_time,
          event->details.key.event_time, event->details.key.is_system_key);
#endif
      // FIXME(loicm) Events for the buttons on the side of the phone, not sure if we should expose
      //     them through multimedia keys or through the QPA native interface.
      break;
    }

    case HW_SWITCH_EVENT_TYPE: {
#if (LOG_EVENTS == 1)
      DLOG("HW_SWITCH device_id:%d source_id:%d action:%d flags:%d meta_state:%d event_time:%lld "
           "policy_flags:%u switch_code:%d switch_value:%d\n", event->device_id, event->source_id,
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
    : mTouchDevice(new QTouchDevice())
    , mIntegration(integration)
    , mStopping(0) {
  // Initialize touch points.
  mTouchPoints.reserve(MAX_POINTER_COUNT);
  for (unsigned int i = 0; i < MAX_POINTER_COUNT; i++) {
    QWindowSystemInterface::TouchPoint tp;
    tp.id = i;
    tp.state = Qt::TouchPointReleased;
    mTouchPoints << tp;
  }

  // Initialize touch device.
  mTouchDevice->setType(QTouchDevice::TouchScreen);
  mTouchDevice->setCapabilities(
      QTouchDevice::Position | QTouchDevice::Area | QTouchDevice::Pressure |
      QTouchDevice::NormalizedPosition);
  QWindowSystemInterface::registerTouchDevice(mTouchDevice);

  // Initialize input stack.
  mConfig.enable_touch_point_visualization = false;
  mConfig.default_layer_for_touch_point_visualization = INT_MAX - 1;
  mListener.on_new_event = hybrisEventCallback;
  mListener.context = this;
  android_input_stack_initialize(&mListener, &mConfig);
  android_input_stack_start();

  DLOG("created QHybrisInput (this=%p)", this);
}

QHybrisInput::~QHybrisInput() {
  // Stop input stack.
  mStopping.fetchAndStoreRelease(1);
  DLOG("Stopping input stack");
  android_input_stack_stop();
  DLOG("Shutting down input stack");
  android_input_stack_shutdown();

  // Clean up touch device and touch points.
  // FIXME(loicm) Generates a "Bus Error" assertion.
  // delete mTouchDevice;
  mTouchPoints.clear();

  DLOG("deleted QHybrisInput");
}
