// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "qhybrisinput.h"
#include "qhybriswindow.h"
#include "qhybrisintegration.h"
#include "qhybrislogging.h"
#include <QtCore/qglobal.h>
#include <input/input_stack_compatibility_layer_flags_motion.h>
#include <input/input_stack_compatibility_layer_flags_key.h>
#include <climits>

#define LOG_EVENTS 0

namespace {

// Lookup table for the key codes.
// FIXME(loicm) It's currently missing several key codes from Qt.
static const int kKeyCode[] = {
  Qt::Key_unknown,         // ISCL_KEYCODE_UNKNOWN         = 0
  Qt::Key_unknown,         // ISCL_KEYCODE_SOFT_LEFT       = 1
  Qt::Key_unknown,         // ISCL_KEYCODE_SOFT_RIGHT      = 2
  Qt::Key_unknown,         // ISCL_KEYCODE_HOME            = 3
  Qt::Key_unknown,         // ISCL_KEYCODE_BACK            = 4
  Qt::Key_unknown,         // ISCL_KEYCODE_CALL            = 5
  Qt::Key_unknown,         // ISCL_KEYCODE_ENDCALL         = 6
  Qt::Key_unknown,         // ISCL_KEYCODE_0               = 7
  Qt::Key_unknown,         // ISCL_KEYCODE_1               = 8
  Qt::Key_unknown,         // ISCL_KEYCODE_2               = 9
  Qt::Key_unknown,         // ISCL_KEYCODE_3               = 10
  Qt::Key_unknown,         // ISCL_KEYCODE_4               = 11
  Qt::Key_unknown,         // ISCL_KEYCODE_5               = 12
  Qt::Key_unknown,         // ISCL_KEYCODE_6               = 13
  Qt::Key_unknown,         // ISCL_KEYCODE_7               = 14
  Qt::Key_unknown,         // ISCL_KEYCODE_8               = 15
  Qt::Key_unknown,         // ISCL_KEYCODE_9               = 16
  Qt::Key_unknown,         // ISCL_KEYCODE_STAR            = 17
  Qt::Key_unknown,         // ISCL_KEYCODE_POUND           = 18
  Qt::Key_unknown,         // ISCL_KEYCODE_DPAD_UP         = 19
  Qt::Key_unknown,         // ISCL_KEYCODE_DPAD_DOWN       = 20
  Qt::Key_unknown,         // ISCL_KEYCODE_DPAD_LEFT       = 21
  Qt::Key_unknown,         // ISCL_KEYCODE_DPAD_RIGHT      = 22
  Qt::Key_unknown,         // ISCL_KEYCODE_DPAD_CENTER     = 23
  Qt::Key_VolumeUp,        // ISCL_KEYCODE_VOLUME_UP       = 24
  Qt::Key_VolumeDown,      // ISCL_KEYCODE_VOLUME_DOWN     = 25
  Qt::Key_PowerOff,        // ISCL_KEYCODE_POWER           = 26
  Qt::Key_unknown,         // ISCL_KEYCODE_CAMERA          = 27
  Qt::Key_unknown,         // ISCL_KEYCODE_CLEAR           = 28
  Qt::Key_unknown,         // ISCL_KEYCODE_A               = 29
  Qt::Key_unknown,         // ISCL_KEYCODE_B               = 30
  Qt::Key_unknown,         // ISCL_KEYCODE_C               = 31
  Qt::Key_unknown,         // ISCL_KEYCODE_D               = 32
  Qt::Key_unknown,         // ISCL_KEYCODE_E               = 33
  Qt::Key_unknown,         // ISCL_KEYCODE_F               = 34
  Qt::Key_unknown,         // ISCL_KEYCODE_G               = 35
  Qt::Key_unknown,         // ISCL_KEYCODE_H               = 36
  Qt::Key_unknown,         // ISCL_KEYCODE_I               = 37
  Qt::Key_unknown,         // ISCL_KEYCODE_J               = 38
  Qt::Key_unknown,         // ISCL_KEYCODE_K               = 39
  Qt::Key_unknown,         // ISCL_KEYCODE_L               = 40
  Qt::Key_unknown,         // ISCL_KEYCODE_M               = 41
  Qt::Key_unknown,         // ISCL_KEYCODE_N               = 42
  Qt::Key_unknown,         // ISCL_KEYCODE_O               = 43
  Qt::Key_unknown,         // ISCL_KEYCODE_P               = 44
  Qt::Key_unknown,         // ISCL_KEYCODE_Q               = 45
  Qt::Key_unknown,         // ISCL_KEYCODE_R               = 46
  Qt::Key_unknown,         // ISCL_KEYCODE_S               = 47
  Qt::Key_unknown,         // ISCL_KEYCODE_T               = 48
  Qt::Key_unknown,         // ISCL_KEYCODE_U               = 49
  Qt::Key_unknown,         // ISCL_KEYCODE_V               = 50
  Qt::Key_unknown,         // ISCL_KEYCODE_W               = 51
  Qt::Key_unknown,         // ISCL_KEYCODE_X               = 52
  Qt::Key_unknown,         // ISCL_KEYCODE_Y               = 53
  Qt::Key_unknown,         // ISCL_KEYCODE_Z               = 54
  Qt::Key_unknown,         // ISCL_KEYCODE_COMMA           = 55
  Qt::Key_unknown,         // ISCL_KEYCODE_PERIOD          = 56
  Qt::Key_unknown,         // ISCL_KEYCODE_ALT_LEFT        = 57
  Qt::Key_unknown,         // ISCL_KEYCODE_ALT_RIGHT       = 58
  Qt::Key_unknown,         // ISCL_KEYCODE_SHIFT_LEFT      = 59
  Qt::Key_unknown,         // ISCL_KEYCODE_SHIFT_RIGHT     = 60
  Qt::Key_unknown,         // ISCL_KEYCODE_TAB             = 61
  Qt::Key_unknown,         // ISCL_KEYCODE_SPACE           = 62
  Qt::Key_unknown,         // ISCL_KEYCODE_SYM             = 63
  Qt::Key_unknown,         // ISCL_KEYCODE_EXPLORER        = 64
  Qt::Key_unknown,         // ISCL_KEYCODE_ENVELOPE        = 65
  Qt::Key_unknown,         // ISCL_KEYCODE_ENTER           = 66
  Qt::Key_unknown,         // ISCL_KEYCODE_DEL             = 67
  Qt::Key_unknown,         // ISCL_KEYCODE_GRAVE           = 68
  Qt::Key_unknown,         // ISCL_KEYCODE_MINUS           = 69
  Qt::Key_unknown,         // ISCL_KEYCODE_EQUALS          = 70
  Qt::Key_unknown,         // ISCL_KEYCODE_LEFT_BRACKET    = 71
  Qt::Key_unknown,         // ISCL_KEYCODE_RIGHT_BRACKET   = 72
  Qt::Key_unknown,         // ISCL_KEYCODE_BACKSLASH       = 73
  Qt::Key_unknown,         // ISCL_KEYCODE_SEMICOLON       = 74
  Qt::Key_unknown,         // ISCL_KEYCODE_APOSTROPHE      = 75
  Qt::Key_unknown,         // ISCL_KEYCODE_SLASH           = 76
  Qt::Key_unknown,         // ISCL_KEYCODE_AT              = 77
  Qt::Key_unknown,         // ISCL_KEYCODE_NUM             = 78
  Qt::Key_unknown,         // ISCL_KEYCODE_HEADSETHOOK     = 79
  Qt::Key_unknown,         // ISCL_KEYCODE_FOCUS           = 80  // *Camera* focus
  Qt::Key_unknown,         // ISCL_KEYCODE_PLUS            = 81
  Qt::Key_unknown,         // ISCL_KEYCODE_MENU            = 82
  Qt::Key_unknown,         // ISCL_KEYCODE_NOTIFICATION    = 83
  Qt::Key_unknown,         // ISCL_KEYCODE_SEARCH          = 84
  Qt::Key_unknown,         // ISCL_KEYCODE_MEDIA_PLAY_PAUSE= 85
  Qt::Key_unknown,         // ISCL_KEYCODE_MEDIA_STOP      = 86
  Qt::Key_unknown,         // ISCL_KEYCODE_MEDIA_NEXT      = 87
  Qt::Key_unknown,         // ISCL_KEYCODE_MEDIA_PREVIOUS  = 88
  Qt::Key_unknown,         // ISCL_KEYCODE_MEDIA_REWIND    = 89
  Qt::Key_unknown,         // ISCL_KEYCODE_MEDIA_FAST_FORWARD = 90
  Qt::Key_unknown,         // ISCL_KEYCODE_MUTE            = 91
  Qt::Key_unknown,         // ISCL_KEYCODE_PAGE_UP         = 92
  Qt::Key_unknown,         // ISCL_KEYCODE_PAGE_DOWN       = 93
  Qt::Key_unknown,         // ISCL_KEYCODE_PICTSYMBOLS     = 94
  Qt::Key_unknown,         // ISCL_KEYCODE_SWITCH_CHARSET  = 95
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_A        = 96
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_B        = 97
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_C        = 98
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_X        = 99
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_Y        = 100
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_Z        = 101
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_L1       = 102
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_R1       = 103
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_L2       = 104
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_R2       = 105
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_THUMBL   = 106
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_THUMBR   = 107
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_START    = 108
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_SELECT   = 109
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_MODE     = 110
  Qt::Key_unknown,         // ISCL_KEYCODE_ESCAPE          = 111
  Qt::Key_unknown,         // ISCL_KEYCODE_FORWARD_DEL     = 112
  Qt::Key_unknown,         // ISCL_KEYCODE_CTRL_LEFT       = 113
  Qt::Key_unknown,         // ISCL_KEYCODE_CTRL_RIGHT      = 114
  Qt::Key_unknown,         // ISCL_KEYCODE_CAPS_LOCK       = 115
  Qt::Key_unknown,         // ISCL_KEYCODE_SCROLL_LOCK     = 116
  Qt::Key_unknown,         // ISCL_KEYCODE_META_LEFT       = 117
  Qt::Key_unknown,         // ISCL_KEYCODE_META_RIGHT      = 118
  Qt::Key_unknown,         // ISCL_KEYCODE_FUNCTION        = 119
  Qt::Key_unknown,         // ISCL_KEYCODE_SYSRQ           = 120
  Qt::Key_unknown,         // ISCL_KEYCODE_BREAK           = 121
  Qt::Key_unknown,         // ISCL_KEYCODE_MOVE_HOME       = 122
  Qt::Key_unknown,         // ISCL_KEYCODE_MOVE_END        = 123
  Qt::Key_unknown,         // ISCL_KEYCODE_INSERT          = 124
  Qt::Key_unknown,         // ISCL_KEYCODE_FORWARD         = 125
  Qt::Key_unknown,         // ISCL_KEYCODE_MEDIA_PLAY      = 126
  Qt::Key_unknown,         // ISCL_KEYCODE_MEDIA_PAUSE     = 127
  Qt::Key_unknown,         // ISCL_KEYCODE_MEDIA_CLOSE     = 128
  Qt::Key_unknown,         // ISCL_KEYCODE_MEDIA_EJECT     = 129
  Qt::Key_unknown,         // ISCL_KEYCODE_MEDIA_RECORD    = 130
  Qt::Key_unknown,         // ISCL_KEYCODE_F1              = 131
  Qt::Key_unknown,         // ISCL_KEYCODE_F2              = 132
  Qt::Key_unknown,         // ISCL_KEYCODE_F3              = 133
  Qt::Key_unknown,         // ISCL_KEYCODE_F4              = 134
  Qt::Key_unknown,         // ISCL_KEYCODE_F5              = 135
  Qt::Key_unknown,         // ISCL_KEYCODE_F6              = 136
  Qt::Key_unknown,         // ISCL_KEYCODE_F7              = 137
  Qt::Key_unknown,         // ISCL_KEYCODE_F8              = 138
  Qt::Key_unknown,         // ISCL_KEYCODE_F9              = 139
  Qt::Key_unknown,         // ISCL_KEYCODE_F10             = 140
  Qt::Key_unknown,         // ISCL_KEYCODE_F11             = 141
  Qt::Key_unknown,         // ISCL_KEYCODE_F12             = 142
  Qt::Key_unknown,         // ISCL_KEYCODE_NUM_LOCK        = 143
  Qt::Key_unknown,         // ISCL_KEYCODE_NUMPAD_0        = 144
  Qt::Key_unknown,         // ISCL_KEYCODE_NUMPAD_1        = 145
  Qt::Key_unknown,         // ISCL_KEYCODE_NUMPAD_2        = 146
  Qt::Key_unknown,         // ISCL_KEYCODE_NUMPAD_3        = 147
  Qt::Key_unknown,         // ISCL_KEYCODE_NUMPAD_4        = 148
  Qt::Key_unknown,         // ISCL_KEYCODE_NUMPAD_5        = 149
  Qt::Key_unknown,         // ISCL_KEYCODE_NUMPAD_6        = 150
  Qt::Key_unknown,         // ISCL_KEYCODE_NUMPAD_7        = 151
  Qt::Key_unknown,         // ISCL_KEYCODE_NUMPAD_8        = 152
  Qt::Key_unknown,         // ISCL_KEYCODE_NUMPAD_9        = 153
  Qt::Key_unknown,         // ISCL_KEYCODE_NUMPAD_DIVIDE   = 154
  Qt::Key_unknown,         // ISCL_KEYCODE_NUMPAD_MULTIPLY = 155
  Qt::Key_unknown,         // ISCL_KEYCODE_NUMPAD_SUBTRACT = 156
  Qt::Key_unknown,         // ISCL_KEYCODE_NUMPAD_ADD      = 157
  Qt::Key_unknown,         // ISCL_KEYCODE_NUMPAD_DOT      = 158
  Qt::Key_unknown,         // ISCL_KEYCODE_NUMPAD_COMMA    = 159
  Qt::Key_unknown,         // ISCL_KEYCODE_NUMPAD_ENTER    = 160
  Qt::Key_unknown,         // ISCL_KEYCODE_NUMPAD_EQUALS   = 161
  Qt::Key_unknown,         // ISCL_KEYCODE_NUMPAD_LEFT_PAREN = 162
  Qt::Key_unknown,         // ISCL_KEYCODE_NUMPAD_RIGHT_PAREN = 163
  Qt::Key_unknown,         // ISCL_KEYCODE_VOLUME_MUTE     = 164
  Qt::Key_unknown,         // ISCL_KEYCODE_INFO            = 165
  Qt::Key_unknown,         // ISCL_KEYCODE_CHANNEL_UP      = 166
  Qt::Key_unknown,         // ISCL_KEYCODE_CHANNEL_DOWN    = 167
  Qt::Key_unknown,         // ISCL_KEYCODE_ZOOM_IN         = 168
  Qt::Key_unknown,         // ISCL_KEYCODE_ZOOM_OUT        = 169
  Qt::Key_unknown,         // ISCL_KEYCODE_TV              = 170
  Qt::Key_unknown,         // ISCL_KEYCODE_WINDOW          = 171
  Qt::Key_unknown,         // ISCL_KEYCODE_GUIDE           = 172
  Qt::Key_unknown,         // ISCL_KEYCODE_DVR             = 173
  Qt::Key_unknown,         // ISCL_KEYCODE_BOOKMARK        = 174
  Qt::Key_unknown,         // ISCL_KEYCODE_CAPTIONS        = 175
  Qt::Key_unknown,         // ISCL_KEYCODE_SETTINGS        = 176
  Qt::Key_unknown,         // ISCL_KEYCODE_TV_POWER        = 177
  Qt::Key_unknown,         // ISCL_KEYCODE_TV_INPUT        = 178
  Qt::Key_unknown,         // ISCL_KEYCODE_STB_POWER       = 179
  Qt::Key_unknown,         // ISCL_KEYCODE_STB_INPUT       = 180
  Qt::Key_unknown,         // ISCL_KEYCODE_AVR_POWER       = 181
  Qt::Key_unknown,         // ISCL_KEYCODE_AVR_INPUT       = 182
  Qt::Key_unknown,         // ISCL_KEYCODE_PROG_RED        = 183
  Qt::Key_unknown,         // ISCL_KEYCODE_PROG_GREEN      = 184
  Qt::Key_unknown,         // ISCL_KEYCODE_PROG_YELLOW     = 185
  Qt::Key_unknown,         // ISCL_KEYCODE_PROG_BLUE       = 186
  Qt::Key_unknown,         // ISCL_KEYCODE_APP_SWITCH      = 187
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_1        = 188
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_2        = 189
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_3        = 190
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_4        = 191
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_5        = 192
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_6        = 193
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_7        = 194
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_8        = 195
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_9        = 196
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_10       = 197
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_11       = 198
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_12       = 199
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_13       = 200
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_14       = 201
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_15       = 202
  Qt::Key_unknown,         // ISCL_KEYCODE_BUTTON_16       = 203
  Qt::Key_unknown,         // ISCL_KEYCODE_LANGUAGE_SWITCH = 204
  Qt::Key_unknown,         // ISCL_KEYCODE_MANNER_MODE     = 205
  Qt::Key_unknown,         // ISCL_KEYCODE_3D_MODE         = 206
  Qt::Key_unknown,         // ISCL_KEYCODE_CONTACTS        = 207
  Qt::Key_unknown,         // ISCL_KEYCODE_CALENDAR        = 208
  Qt::Key_unknown,         // ISCL_KEYCODE_MUSIC           = 209
  Qt::Key_unknown          // ISCL_KEYCODE_CALCULATOR      = 210
};

// Lookup table for the key types.
// FIXME(loicm) Not sure what to do with that multiple thing.
static const QEvent::Type kEventType[] = {
  QEvent::KeyPress,    // ISCL_KEY_EVENT_ACTION_DOWN     = 0
  QEvent::KeyRelease,  // ISCL_KEY_EVENT_ACTION_UP       = 1
  QEvent::KeyPress     // ISCL_KEY_EVENT_ACTION_MULTIPLE = 2
};

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

  const int kMetaState = event->meta_state;
  Qt::KeyboardModifiers modifiers = Qt::NoModifier;
  if (kMetaState & ISCL_META_SHIFT_ON)
    modifiers |= Qt::ShiftModifier;
  if (kMetaState & ISCL_META_CTRL_ON)
    modifiers |= Qt::ControlModifier;
  if (kMetaState & ISCL_META_ALT_ON)
    modifiers |= Qt::AltModifier;
  if (kMetaState & ISCL_META_META_ON)
    modifiers |= Qt::MetaModifier;
  if (kMetaState & ISCL_META_META_ON)
    modifiers |= Qt::MetaModifier;

  QWindowSystemInterface::handleKeyEvent(
      window->window(), event->details.key.event_time, kEventType[event->action],
      kKeyCode[event->details.key.key_code], modifiers);
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
