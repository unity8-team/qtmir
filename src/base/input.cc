// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "input.h"
#include "integration.h"
#include "native_interface.h"
#include "logging.h"
#include <QtCore/qglobal.h>
#include <cstring>  // input_stack_compatibility_layer.h needs this for size_t.
#include <input/input_stack_compatibility_layer.h>
#include <input/input_stack_compatibility_layer_flags_motion.h>
#include <input/input_stack_compatibility_layer_flags_key.h>

#define LOG_EVENTS 0

// Lookup table for the key types.
// FIXME(loicm) Not sure what to do with that multiple thing.
static const QEvent::Type kEventType[] = {
  QEvent::KeyPress,    // ISCL_KEY_EVENT_ACTION_DOWN     = 0
  QEvent::KeyRelease,  // ISCL_KEY_EVENT_ACTION_UP       = 1
  QEvent::KeyPress     // ISCL_KEY_EVENT_ACTION_MULTIPLE = 2
};

// Lookup table for the key codes.
static const int kKeyCode[] = {
  Qt::Key_unknown,         // ISCL_KEYCODE_UNKNOWN         = 0
  Qt::Key_unknown,         // ISCL_KEYCODE_SOFT_LEFT       = 1
  Qt::Key_unknown,         // ISCL_KEYCODE_SOFT_RIGHT      = 2
  Qt::Key_Home,            // ISCL_KEYCODE_HOME            = 3
  Qt::Key_Back,            // ISCL_KEYCODE_BACK            = 4
  Qt::Key_Call,            // ISCL_KEYCODE_CALL            = 5
  Qt::Key_Hangup,          // ISCL_KEYCODE_ENDCALL         = 6
  Qt::Key_0,               // ISCL_KEYCODE_0               = 7
  Qt::Key_1,               // ISCL_KEYCODE_1               = 8
  Qt::Key_2,               // ISCL_KEYCODE_2               = 9
  Qt::Key_3,               // ISCL_KEYCODE_3               = 10
  Qt::Key_4,               // ISCL_KEYCODE_4               = 11
  Qt::Key_5,               // ISCL_KEYCODE_5               = 12
  Qt::Key_6,               // ISCL_KEYCODE_6               = 13
  Qt::Key_7,               // ISCL_KEYCODE_7               = 14
  Qt::Key_8,               // ISCL_KEYCODE_8               = 15
  Qt::Key_9,               // ISCL_KEYCODE_9               = 16
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
  Qt::Key_Camera,          // ISCL_KEYCODE_CAMERA          = 27
  Qt::Key_Clear,           // ISCL_KEYCODE_CLEAR           = 28
  Qt::Key_A,               // ISCL_KEYCODE_A               = 29
  Qt::Key_B,               // ISCL_KEYCODE_B               = 30
  Qt::Key_C,               // ISCL_KEYCODE_C               = 31
  Qt::Key_D,               // ISCL_KEYCODE_D               = 32
  Qt::Key_E,               // ISCL_KEYCODE_E               = 33
  Qt::Key_F,               // ISCL_KEYCODE_F               = 34
  Qt::Key_G,               // ISCL_KEYCODE_G               = 35
  Qt::Key_H,               // ISCL_KEYCODE_H               = 36
  Qt::Key_I,               // ISCL_KEYCODE_I               = 37
  Qt::Key_J,               // ISCL_KEYCODE_J               = 38
  Qt::Key_K,               // ISCL_KEYCODE_K               = 39
  Qt::Key_L,               // ISCL_KEYCODE_L               = 40
  Qt::Key_M,               // ISCL_KEYCODE_M               = 41
  Qt::Key_N,               // ISCL_KEYCODE_N               = 42
  Qt::Key_O,               // ISCL_KEYCODE_O               = 43
  Qt::Key_P,               // ISCL_KEYCODE_P               = 44
  Qt::Key_Q,               // ISCL_KEYCODE_Q               = 45
  Qt::Key_R,               // ISCL_KEYCODE_R               = 46
  Qt::Key_S,               // ISCL_KEYCODE_S               = 47
  Qt::Key_T,               // ISCL_KEYCODE_T               = 48
  Qt::Key_U,               // ISCL_KEYCODE_U               = 49
  Qt::Key_V,               // ISCL_KEYCODE_V               = 50
  Qt::Key_W,               // ISCL_KEYCODE_W               = 51
  Qt::Key_X,               // ISCL_KEYCODE_X               = 52
  Qt::Key_Y,               // ISCL_KEYCODE_Y               = 53
  Qt::Key_Z,               // ISCL_KEYCODE_Z               = 54
  Qt::Key_Comma,           // ISCL_KEYCODE_COMMA           = 55
  Qt::Key_Period,          // ISCL_KEYCODE_PERIOD          = 56
  Qt::Key_unknown,         // ISCL_KEYCODE_ALT_LEFT        = 57
  Qt::Key_unknown,         // ISCL_KEYCODE_ALT_RIGHT       = 58
  Qt::Key_unknown,         // ISCL_KEYCODE_SHIFT_LEFT      = 59
  Qt::Key_unknown,         // ISCL_KEYCODE_SHIFT_RIGHT     = 60
  Qt::Key_Tab,             // ISCL_KEYCODE_TAB             = 61
  Qt::Key_Space,           // ISCL_KEYCODE_SPACE           = 62
  Qt::Key_unknown,         // ISCL_KEYCODE_SYM             = 63
  Qt::Key_Explorer,        // ISCL_KEYCODE_EXPLORER        = 64
  Qt::Key_LaunchMail,      // ISCL_KEYCODE_ENVELOPE        = 65
  Qt::Key_Enter,           // ISCL_KEYCODE_ENTER           = 66
  Qt::Key_Delete,          // ISCL_KEYCODE_DEL             = 67
  Qt::Key_unknown,         // ISCL_KEYCODE_GRAVE           = 68
  Qt::Key_Minus,           // ISCL_KEYCODE_MINUS           = 69
  Qt::Key_Equal,           // ISCL_KEYCODE_EQUALS          = 70
  Qt::Key_BracketLeft,     // ISCL_KEYCODE_LEFT_BRACKET    = 71
  Qt::Key_BracketRight,    // ISCL_KEYCODE_RIGHT_BRACKET   = 72
  Qt::Key_Backslash,       // ISCL_KEYCODE_BACKSLASH       = 73
  Qt::Key_Semicolon,       // ISCL_KEYCODE_SEMICOLON       = 74
  Qt::Key_Apostrophe,      // ISCL_KEYCODE_APOSTROPHE      = 75
  Qt::Key_Slash,           // ISCL_KEYCODE_SLASH           = 76
  Qt::Key_At,              // ISCL_KEYCODE_AT              = 77
  Qt::Key_unknown,         // ISCL_KEYCODE_NUM             = 78
  Qt::Key_unknown,         // ISCL_KEYCODE_HEADSETHOOK     = 79
  Qt::Key_CameraFocus,     // ISCL_KEYCODE_FOCUS           = 80  // *Camera* focus
  Qt::Key_Plus,            // ISCL_KEYCODE_PLUS            = 81
  Qt::Key_Menu,            // ISCL_KEYCODE_MENU            = 82
  Qt::Key_unknown,         // ISCL_KEYCODE_NOTIFICATION    = 83
  Qt::Key_Search,          // ISCL_KEYCODE_SEARCH          = 84
  Qt::Key_MediaTogglePlayPause,  // ISCL_KEYCODE_MEDIA_PLAY_PAUSE= 85
  Qt::Key_MediaStop,       // ISCL_KEYCODE_MEDIA_STOP      = 86
  Qt::Key_MediaNext,       // ISCL_KEYCODE_MEDIA_NEXT      = 87
  Qt::Key_MediaPrevious,   // ISCL_KEYCODE_MEDIA_PREVIOUS  = 88
  Qt::Key_unknown,         // ISCL_KEYCODE_MEDIA_REWIND    = 89
  Qt::Key_unknown,         // ISCL_KEYCODE_MEDIA_FAST_FORWARD = 90
  Qt::Key_VolumeMute,      // ISCL_KEYCODE_MUTE            = 91
  Qt::Key_PageUp,          // ISCL_KEYCODE_PAGE_UP         = 92
  Qt::Key_PageDown,        // ISCL_KEYCODE_PAGE_DOWN       = 93
  Qt::Key_Pictures,        // ISCL_KEYCODE_PICTSYMBOLS     = 94
  Qt::Key_Mode_switch,     // ISCL_KEYCODE_SWITCH_CHARSET  = 95
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
  Qt::Key_Escape,          // ISCL_KEYCODE_ESCAPE          = 111
  Qt::Key_unknown,         // ISCL_KEYCODE_FORWARD_DEL     = 112
  Qt::Key_unknown,         // ISCL_KEYCODE_CTRL_LEFT       = 113
  Qt::Key_unknown,         // ISCL_KEYCODE_CTRL_RIGHT      = 114
  Qt::Key_CapsLock,        // ISCL_KEYCODE_CAPS_LOCK       = 115
  Qt::Key_ScrollLock,      // ISCL_KEYCODE_SCROLL_LOCK     = 116
  Qt::Key_unknown,         // ISCL_KEYCODE_META_LEFT       = 117
  Qt::Key_unknown,         // ISCL_KEYCODE_META_RIGHT      = 118
  Qt::Key_unknown,         // ISCL_KEYCODE_FUNCTION        = 119
  Qt::Key_SysReq,          // ISCL_KEYCODE_SYSRQ           = 120
  Qt::Key_unknown,         // ISCL_KEYCODE_BREAK           = 121
  Qt::Key_unknown,         // ISCL_KEYCODE_MOVE_HOME       = 122
  Qt::Key_unknown,         // ISCL_KEYCODE_MOVE_END        = 123
  Qt::Key_Insert,          // ISCL_KEYCODE_INSERT          = 124
  Qt::Key_Forward,         // ISCL_KEYCODE_FORWARD         = 125
  Qt::Key_MediaPlay,       // ISCL_KEYCODE_MEDIA_PLAY      = 126
  Qt::Key_MediaPause,      // ISCL_KEYCODE_MEDIA_PAUSE     = 127
  Qt::Key_unknown,         // ISCL_KEYCODE_MEDIA_CLOSE     = 128
  Qt::Key_Eject,           // ISCL_KEYCODE_MEDIA_EJECT     = 129
  Qt::Key_MediaRecord,     // ISCL_KEYCODE_MEDIA_RECORD    = 130
  Qt::Key_F1,              // ISCL_KEYCODE_F1              = 131
  Qt::Key_F2,              // ISCL_KEYCODE_F2              = 132
  Qt::Key_F3,              // ISCL_KEYCODE_F3              = 133
  Qt::Key_F4,              // ISCL_KEYCODE_F4              = 134
  Qt::Key_F5,              // ISCL_KEYCODE_F5              = 135
  Qt::Key_F6,              // ISCL_KEYCODE_F6              = 136
  Qt::Key_F7,              // ISCL_KEYCODE_F7              = 137
  Qt::Key_F8,              // ISCL_KEYCODE_F8              = 138
  Qt::Key_F9,              // ISCL_KEYCODE_F9              = 139
  Qt::Key_F10,             // ISCL_KEYCODE_F10             = 140
  Qt::Key_F11,             // ISCL_KEYCODE_F11             = 141
  Qt::Key_F12,             // ISCL_KEYCODE_F12             = 142
  Qt::Key_NumLock,         // ISCL_KEYCODE_NUM_LOCK        = 143
  Qt::Key_0,               // ISCL_KEYCODE_NUMPAD_0        = 144
  Qt::Key_1,               // ISCL_KEYCODE_NUMPAD_1        = 145
  Qt::Key_2,               // ISCL_KEYCODE_NUMPAD_2        = 146
  Qt::Key_3,               // ISCL_KEYCODE_NUMPAD_3        = 147
  Qt::Key_4,               // ISCL_KEYCODE_NUMPAD_4        = 148
  Qt::Key_5,               // ISCL_KEYCODE_NUMPAD_5        = 149
  Qt::Key_6,               // ISCL_KEYCODE_NUMPAD_6        = 150
  Qt::Key_7,               // ISCL_KEYCODE_NUMPAD_7        = 151
  Qt::Key_8,               // ISCL_KEYCODE_NUMPAD_8        = 152
  Qt::Key_9,               // ISCL_KEYCODE_NUMPAD_9        = 153
  Qt::Key_Slash,           // ISCL_KEYCODE_NUMPAD_DIVIDE   = 154
  Qt::Key_Asterisk,        // ISCL_KEYCODE_NUMPAD_MULTIPLY = 155
  Qt::Key_Minus,           // ISCL_KEYCODE_NUMPAD_SUBTRACT = 156
  Qt::Key_Plus,            // ISCL_KEYCODE_NUMPAD_ADD      = 157
  Qt::Key_Period,          // ISCL_KEYCODE_NUMPAD_DOT      = 158
  Qt::Key_Comma,           // ISCL_KEYCODE_NUMPAD_COMMA    = 159
  Qt::Key_Enter,           // ISCL_KEYCODE_NUMPAD_ENTER    = 160
  Qt::Key_Equal,           // ISCL_KEYCODE_NUMPAD_EQUALS   = 161
  Qt::Key_ParenLeft,       // ISCL_KEYCODE_NUMPAD_LEFT_PAREN = 162
  Qt::Key_ParenRight,      // ISCL_KEYCODE_NUMPAD_RIGHT_PAREN = 163
  Qt::Key_VolumeMute,      // ISCL_KEYCODE_VOLUME_MUTE     = 164
  Qt::Key_unknown,         // ISCL_KEYCODE_INFO            = 165
  Qt::Key_unknown,         // ISCL_KEYCODE_CHANNEL_UP      = 166
  Qt::Key_unknown,         // ISCL_KEYCODE_CHANNEL_DOWN    = 167
  Qt::Key_ZoomIn,          // ISCL_KEYCODE_ZOOM_IN         = 168
  Qt::Key_ZoomOut,         // ISCL_KEYCODE_ZOOM_OUT        = 169
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
  Qt::Key_Calendar,        // ISCL_KEYCODE_CALENDAR        = 208
  Qt::Key_Music,           // ISCL_KEYCODE_MUSIC           = 209
  Qt::Key_Calculator       // ISCL_KEYCODE_CALCULATOR      = 210
};

QHybrisBaseInput::QHybrisBaseInput(QHybrisBaseIntegration* integration, int maxPointCount)
    : integration_(integration)
    , eventFilterType_(static_cast<QHybrisBaseNativeInterface*>(
        integration->nativeInterface())->genericEventFilterType()) {
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

  DLOG("QHybrisBaseInput::QHybrisBaseInput (this=%p, integration=%p, maxPointCount=%d)", this,
       integration, maxPointCount);
}

QHybrisBaseInput::~QHybrisBaseInput() {
  DLOG("QHybrisBaseInput::~QHybrisBaseInput");

  // Clean up touch points and touch device.
  touchPoints_.clear();
  delete touchDevice_;
}

void QHybrisBaseInput::handleEvent(QWindow* window, const Event* event) {
  DLOG("QHybrisBaseInput::handleEvent (window=%p, event=%p)", window, event);

  // Event filtering.
  long result;
  if (QWindowSystemInterface::handleNativeEvent(
          window, eventFilterType_, const_cast<Event*>(event), &result) == true) {
    DLOG("event filtered out");
    return;
  }

  // Event dispatching.
  switch (event->type) {
    case MOTION_EVENT_TYPE: {
      handleMotionEvent(window, event);
      break;
    }
    case KEY_EVENT_TYPE: {
      handleKeyEvent(window, event);
      break;
    }
    case HW_SWITCH_EVENT_TYPE: {
      handleHWSwitchEvent(window, event);
      break;
    }
    default: {
      // FIXME(loicm) Never received such types yet. Let's see if people get these. Switch to
      //     DNOT_REACHED() before releasing.
      NOT_REACHED();
    }
  }
}

void QHybrisBaseInput::handleMotionEvent(QWindow* window, const Event* event) {
  DLOG("QHybrisBaseInput::handleMotionEvent (window=%p, event=%p)", window, event);

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

  switch (event->action & ISCL_MOTION_EVENT_ACTION_MASK) {
    case ISCL_MOTION_EVENT_ACTION_MOVE: {
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

    case ISCL_MOTION_EVENT_ACTION_DOWN: {
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

    case ISCL_MOTION_EVENT_ACTION_UP: {
      const int kTouchIndex = event->details.motion.pointer_coordinates[0].id;
      touchPoints_[kTouchIndex].state = Qt::TouchPointReleased;
      break;
    }

    case ISCL_MOTION_EVENT_ACTION_POINTER_DOWN: {
      const int eventIndex = (event->action & ISCL_MOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
          ISCL_MOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
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

    case ISCL_MOTION_EVENT_ACTION_CANCEL:
    case ISCL_MOTION_EVENT_ACTION_POINTER_UP: {
      const int kEventIndex = (event->action & ISCL_MOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
          ISCL_MOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
      const int kTouchIndex = event->details.motion.pointer_coordinates[kEventIndex].id;
      touchPoints_[kTouchIndex].state = Qt::TouchPointReleased;
      break;
    }

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

  // Touch event propagation.
  QWindowSystemInterface::handleTouchEvent(
      window, event->details.motion.event_time, touchDevice_, touchPoints_);
}

void QHybrisBaseInput::handleKeyEvent(QWindow* window, const Event* event) {
  DLOG("QHybrisBaseInput::handleKeyEvent (window=%p, event=%p)", window, event);

#if (LOG_EVENTS != 0)
  // Key event logging.
  LOG("KEY device_id:%d source_id:%d action:%d flags:%d meta_state:%d key_code:%d "
      "scan_code:%d repeat_count:%d down_time:%lld event_time:%lld is_system_key:%d",
      event->device_id, event->source_id, event->action, event->flags, event->meta_state,
      event->details.key.key_code, event->details.key.scan_code,
      event->details.key.repeat_count, event->details.key.down_time,
      event->details.key.event_time, event->details.key.is_system_key);
#endif

  // Key modifier mapping.
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

  // Key event propagation.
  QWindowSystemInterface::handleKeyEvent(
      window, event->details.key.event_time, kEventType[event->action],
      kKeyCode[event->details.key.key_code], modifiers);
}

void QHybrisBaseInput::handleHWSwitchEvent(QWindow* window, const Event* event) {
  Q_UNUSED(window);
  Q_UNUSED(event);
  DLOG("QHybrisBaseInput::handleSwitchEvent (window=%p, event=%p)", window, event);

#if (LOG_EVENTS != 0)
  // HW switch event logging.
  LOG("HWSWITCH device_id:%d source_id:%d action:%d flags:%d meta_state:%d event_time:%lld "
      "policy_flags:%u switch_code:%d switch_value:%d", event->device_id, event->source_id,
      event->action, event->flags, event->meta_state, event->details.hw_switch.event_time,
      event->details.hw_switch.policy_flags, event->details.hw_switch.switch_code,
      event->details.hw_switch.switch_value);
#endif

  // FIXME(loicm) Not sure how to interpret that kind of event.
  DLOG("hw switch events are not handled");
}
