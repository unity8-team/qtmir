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
#include <input/input_stack_compatibility_layer_flags.h>

#define LOG_EVENTS 0

// Lookup table for the key types.
// FIXME(loicm) Not sure what to do with that multiple thing.
static const QEvent::Type kEventType[] = {
  QEvent::KeyPress,    // ISCL_KEY_EVENT_ACTION_DOWN     = 0
  QEvent::KeyRelease,  // ISCL_KEY_EVENT_ACTION_UP       = 1
  QEvent::KeyPress     // ISCL_KEY_EVENT_ACTION_MULTIPLE = 2
};

// Lookup table for the key codes and unicode values.
static const struct {
  const quint32 keycode;
  const quint16 unicode[3];  // { no modifier, shift modifier, other modifiers }
} kKeyCode[] = {
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_UNKNOWN         = 0
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_SOFT_LEFT       = 1
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_SOFT_RIGHT      = 2
  { Qt::Key_Home, { 0xffff, 0xffff, 0xffff } },            // ISCL_KEYCODE_HOME            = 3
  { Qt::Key_Back, { 0xffff, 0xffff, 0xffff } },            // ISCL_KEYCODE_BACK            = 4
  { Qt::Key_Call, { 0xffff, 0xffff, 0xffff } },            // ISCL_KEYCODE_CALL            = 5
  { Qt::Key_Hangup, { 0xffff, 0xffff, 0xffff } },          // ISCL_KEYCODE_ENDCALL         = 6
  { Qt::Key_0, { 0x0030, 0xffff, 0xffff } },               // ISCL_KEYCODE_0               = 7
  { Qt::Key_1, { 0x0031, 0xffff, 0xffff } },               // ISCL_KEYCODE_1               = 8
  { Qt::Key_2, { 0x0032, 0xffff, 0xffff } },               // ISCL_KEYCODE_2               = 9
  { Qt::Key_3, { 0x0033, 0xffff, 0xffff } },               // ISCL_KEYCODE_3               = 10
  { Qt::Key_4, { 0x0034, 0xffff, 0xffff } },               // ISCL_KEYCODE_4               = 11
  { Qt::Key_5, { 0x0035, 0xffff, 0xffff } },               // ISCL_KEYCODE_5               = 12
  { Qt::Key_6, { 0x0036, 0xffff, 0xffff } },               // ISCL_KEYCODE_6               = 13
  { Qt::Key_7, { 0x0037, 0xffff, 0xffff } },               // ISCL_KEYCODE_7               = 14
  { Qt::Key_8, { 0x0038, 0xffff, 0xffff } },               // ISCL_KEYCODE_8               = 15
  { Qt::Key_9, { 0x0039, 0xffff, 0xffff } },               // ISCL_KEYCODE_9               = 16
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_STAR            = 17
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_POUND           = 18
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_DPAD_UP         = 19
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_DPAD_DOWN       = 20
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_DPAD_LEFT       = 21
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_DPAD_RIGHT      = 22
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_DPAD_CENTER     = 23
  { Qt::Key_VolumeUp, { 0xffff, 0xffff, 0xffff } },        // ISCL_KEYCODE_VOLUME_UP       = 24
  { Qt::Key_VolumeDown, { 0xffff, 0xffff, 0xffff } },      // ISCL_KEYCODE_VOLUME_DOWN     = 25
  { Qt::Key_PowerOff, { 0xffff, 0xffff, 0xffff } },        // ISCL_KEYCODE_POWER           = 26
  { Qt::Key_Camera, { 0xffff, 0xffff, 0xffff } },          // ISCL_KEYCODE_CAMERA          = 27
  { Qt::Key_Clear, { 0xffff, 0xffff, 0xffff } },           // ISCL_KEYCODE_CLEAR           = 28
  { Qt::Key_A, { 0x0061, 0x0041, 0xffff } },               // ISCL_KEYCODE_A               = 29
  { Qt::Key_B, { 0x0062, 0x0042, 0xffff } },               // ISCL_KEYCODE_B               = 30
  { Qt::Key_C, { 0x0063, 0x0043, 0xffff } },               // ISCL_KEYCODE_C               = 31
  { Qt::Key_D, { 0x0064, 0x0044, 0xffff } },               // ISCL_KEYCODE_D               = 32
  { Qt::Key_E, { 0x0065, 0x0045, 0xffff } },               // ISCL_KEYCODE_E               = 33
  { Qt::Key_F, { 0x0066, 0x0046, 0xffff } },               // ISCL_KEYCODE_F               = 34
  { Qt::Key_G, { 0x0067, 0x0047, 0xffff } },               // ISCL_KEYCODE_G               = 35
  { Qt::Key_H, { 0x0068, 0x0048, 0xffff } },               // ISCL_KEYCODE_H               = 36
  { Qt::Key_I, { 0x0069, 0x0049, 0xffff } },               // ISCL_KEYCODE_I               = 37
  { Qt::Key_J, { 0x006a, 0x004a, 0xffff } },               // ISCL_KEYCODE_J               = 38
  { Qt::Key_K, { 0x006b, 0x004b, 0xffff } },               // ISCL_KEYCODE_K               = 39
  { Qt::Key_L, { 0x006c, 0x004c, 0xffff } },               // ISCL_KEYCODE_L               = 40
  { Qt::Key_M, { 0x006d, 0x004d, 0xffff } },               // ISCL_KEYCODE_M               = 41
  { Qt::Key_N, { 0x006e, 0x004e, 0xffff } },               // ISCL_KEYCODE_N               = 42
  { Qt::Key_O, { 0x006f, 0x004f, 0xffff } },               // ISCL_KEYCODE_O               = 43
  { Qt::Key_P, { 0x0070, 0x0050, 0xffff } },               // ISCL_KEYCODE_P               = 44
  { Qt::Key_Q, { 0x0071, 0x0051, 0xffff } },               // ISCL_KEYCODE_Q               = 45
  { Qt::Key_R, { 0x0072, 0x0052, 0xffff } },               // ISCL_KEYCODE_R               = 46
  { Qt::Key_S, { 0x0073, 0x0053, 0xffff } },               // ISCL_KEYCODE_S               = 47
  { Qt::Key_T, { 0x0074, 0x0054, 0xffff } },               // ISCL_KEYCODE_T               = 48
  { Qt::Key_U, { 0x0075, 0x0055, 0xffff } },               // ISCL_KEYCODE_U               = 49
  { Qt::Key_V, { 0x0076, 0x0056, 0xffff } },               // ISCL_KEYCODE_V               = 50
  { Qt::Key_W, { 0x0077, 0x0057, 0xffff } },               // ISCL_KEYCODE_W               = 51
  { Qt::Key_X, { 0x0078, 0x0058, 0xffff } },               // ISCL_KEYCODE_X               = 52
  { Qt::Key_Y, { 0x0079, 0x0059, 0xffff } },               // ISCL_KEYCODE_Y               = 53
  { Qt::Key_Z, { 0x007a, 0x005a, 0xffff } },               // ISCL_KEYCODE_Z               = 54
  { Qt::Key_Comma, { 0x002c, 0xffff, 0xffff } },           // ISCL_KEYCODE_COMMA           = 55
  { Qt::Key_Period, { 0x002e, 0xffff, 0xffff } },          // ISCL_KEYCODE_PERIOD          = 56
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_ALT_LEFT        = 57
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_ALT_RIGHT       = 58
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_SHIFT_LEFT      = 59
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_SHIFT_RIGHT     = 60
  { Qt::Key_Tab, { 0xffff, 0xffff, 0xffff } },             // ISCL_KEYCODE_TAB             = 61
  { Qt::Key_Space, { 0x0020, 0xffff, 0xffff } },           // ISCL_KEYCODE_SPACE           = 62
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_SYM             = 63
  { Qt::Key_Explorer, { 0xffff, 0xffff, 0xffff } },        // ISCL_KEYCODE_EXPLORER        = 64
  { Qt::Key_LaunchMail, { 0xffff, 0xffff, 0xffff } },      // ISCL_KEYCODE_ENVELOPE        = 65
  { Qt::Key_Enter, { 0xffff, 0xffff, 0xffff } },           // ISCL_KEYCODE_ENTER           = 66
  { Qt::Key_Delete, { 0xffff, 0xffff, 0xffff } },          // ISCL_KEYCODE_DEL             = 67
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_GRAVE           = 68
  { Qt::Key_Minus, { 0x002d, 0xffff, 0xffff } },           // ISCL_KEYCODE_MINUS           = 69
  { Qt::Key_Equal, { 0x003d, 0xffff, 0xffff } },           // ISCL_KEYCODE_EQUALS          = 70
  { Qt::Key_BracketLeft, { 0x005b, 0xffff, 0xffff } },     // ISCL_KEYCODE_LEFT_BRACKET    = 71
  { Qt::Key_BracketRight, { 0x005d, 0xffff, 0xffff } },    // ISCL_KEYCODE_RIGHT_BRACKET   = 72
  { Qt::Key_Backslash, { 0x005c, 0xffff, 0xffff } },       // ISCL_KEYCODE_BACKSLASH       = 73
  { Qt::Key_Semicolon, { 0x003b, 0x003a, 0xffff } },       // ISCL_KEYCODE_SEMICOLON       = 74
  { Qt::Key_Apostrophe, { 0x0027, 0xffff, 0xffff } },      // ISCL_KEYCODE_APOSTROPHE      = 75
  { Qt::Key_Slash, { 0x002f, 0xffff, 0xffff } },           // ISCL_KEYCODE_SLASH           = 76
  { Qt::Key_At, { 0x0040, 0xffff, 0xffff } },              // ISCL_KEYCODE_AT              = 77
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_NUM             = 78
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_HEADSETHOOK     = 79
  { Qt::Key_CameraFocus, { 0xffff, 0xffff, 0xffff } },     // ISCL_KEYCODE_FOCUS           = 80  // *Camera* focus
  { Qt::Key_Plus, { 0x002b, 0xffff, 0xffff } },            // ISCL_KEYCODE_PLUS            = 81
  { Qt::Key_Menu, { 0xffff, 0xffff, 0xffff } },            // ISCL_KEYCODE_MENU            = 82
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_NOTIFICATION    = 83
  { Qt::Key_Search, { 0xffff, 0xffff, 0xffff } },          // ISCL_KEYCODE_SEARCH          = 84
  { Qt::Key_MediaTogglePlayPause, { 0xffff, 0xffff, 0xffff } },  // ISCL_KEYCODE_MEDIA_PLAY_PAUSE= 85
  { Qt::Key_MediaStop, { 0xffff, 0xffff, 0xffff } },       // ISCL_KEYCODE_MEDIA_STOP      = 86
  { Qt::Key_MediaNext, { 0xffff, 0xffff, 0xffff } },       // ISCL_KEYCODE_MEDIA_NEXT      = 87
  { Qt::Key_MediaPrevious, { 0xffff, 0xffff, 0xffff } },   // ISCL_KEYCODE_MEDIA_PREVIOUS  = 88
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_MEDIA_REWIND    = 89
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_MEDIA_FAST_FORWARD = 90
  { Qt::Key_VolumeMute, { 0xffff, 0xffff, 0xffff } },      // ISCL_KEYCODE_MUTE            = 91
  { Qt::Key_PageUp, { 0xffff, 0xffff, 0xffff } },          // ISCL_KEYCODE_PAGE_UP         = 92
  { Qt::Key_PageDown, { 0xffff, 0xffff, 0xffff } },        // ISCL_KEYCODE_PAGE_DOWN       = 93
  { Qt::Key_Pictures, { 0xffff, 0xffff, 0xffff } },        // ISCL_KEYCODE_PICTSYMBOLS     = 94
  { Qt::Key_Mode_switch, { 0xffff, 0xffff, 0xffff } },     // ISCL_KEYCODE_SWITCH_CHARSET  = 95
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_A        = 96
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_B        = 97
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_C        = 98
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_X        = 99
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_Y        = 100
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_Z        = 101
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_L1       = 102
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_R1       = 103
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_L2       = 104
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_R2       = 105
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_THUMBL   = 106
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_THUMBR   = 107
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_START    = 108
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_SELECT   = 109
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_MODE     = 110
  { Qt::Key_Escape, { 0xffff, 0xffff, 0xffff } },          // ISCL_KEYCODE_ESCAPE          = 111
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_FORWARD_DEL     = 112
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_CTRL_LEFT       = 113
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_CTRL_RIGHT      = 114
  { Qt::Key_CapsLock, { 0xffff, 0xffff, 0xffff } },        // ISCL_KEYCODE_CAPS_LOCK       = 115
  { Qt::Key_ScrollLock, { 0xffff, 0xffff, 0xffff } },      // ISCL_KEYCODE_SCROLL_LOCK     = 116
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_META_LEFT       = 117
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_META_RIGHT      = 118
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_FUNCTION        = 119
  { Qt::Key_SysReq, { 0xffff, 0xffff, 0xffff } },          // ISCL_KEYCODE_SYSRQ           = 120
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BREAK           = 121
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_MOVE_HOME       = 122
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_MOVE_END        = 123
  { Qt::Key_Insert, { 0xffff, 0xffff, 0xffff } },          // ISCL_KEYCODE_INSERT          = 124
  { Qt::Key_Forward, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_FORWARD         = 125
  { Qt::Key_MediaPlay, { 0xffff, 0xffff, 0xffff } },       // ISCL_KEYCODE_MEDIA_PLAY      = 126
  { Qt::Key_MediaPause, { 0xffff, 0xffff, 0xffff } },      // ISCL_KEYCODE_MEDIA_PAUSE     = 127
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_MEDIA_CLOSE     = 128
  { Qt::Key_Eject, { 0xffff, 0xffff, 0xffff } },           // ISCL_KEYCODE_MEDIA_EJECT     = 129
  { Qt::Key_MediaRecord, { 0xffff, 0xffff, 0xffff } },     // ISCL_KEYCODE_MEDIA_RECORD    = 130
  { Qt::Key_F1, { 0xffff, 0xffff, 0xffff } },              // ISCL_KEYCODE_F1              = 131
  { Qt::Key_F2, { 0xffff, 0xffff, 0xffff } },              // ISCL_KEYCODE_F2              = 132
  { Qt::Key_F3, { 0xffff, 0xffff, 0xffff } },              // ISCL_KEYCODE_F3              = 133
  { Qt::Key_F4, { 0xffff, 0xffff, 0xffff } },              // ISCL_KEYCODE_F4              = 134
  { Qt::Key_F5, { 0xffff, 0xffff, 0xffff } },              // ISCL_KEYCODE_F5              = 135
  { Qt::Key_F6, { 0xffff, 0xffff, 0xffff } },              // ISCL_KEYCODE_F6              = 136
  { Qt::Key_F7, { 0xffff, 0xffff, 0xffff } },              // ISCL_KEYCODE_F7              = 137
  { Qt::Key_F8, { 0xffff, 0xffff, 0xffff } },              // ISCL_KEYCODE_F8              = 138
  { Qt::Key_F9, { 0xffff, 0xffff, 0xffff } },              // ISCL_KEYCODE_F9              = 139
  { Qt::Key_F10, { 0xffff, 0xffff, 0xffff } },             // ISCL_KEYCODE_F10             = 140
  { Qt::Key_F11, { 0xffff, 0xffff, 0xffff } },             // ISCL_KEYCODE_F11             = 141
  { Qt::Key_F12, { 0xffff, 0xffff, 0xffff } },             // ISCL_KEYCODE_F12             = 142
  { Qt::Key_NumLock, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_NUM_LOCK        = 143
  { Qt::Key_0, { 0x0030, 0xffff, 0xffff } },               // ISCL_KEYCODE_NUMPAD_0        = 144
  { Qt::Key_1, { 0x0031, 0xffff, 0xffff } },               // ISCL_KEYCODE_NUMPAD_1        = 145
  { Qt::Key_2, { 0x0032, 0xffff, 0xffff } },               // ISCL_KEYCODE_NUMPAD_2        = 146
  { Qt::Key_3, { 0x0033, 0xffff, 0xffff } },               // ISCL_KEYCODE_NUMPAD_3        = 147
  { Qt::Key_4, { 0x0034, 0xffff, 0xffff } },               // ISCL_KEYCODE_NUMPAD_4        = 148
  { Qt::Key_5, { 0x0035, 0xffff, 0xffff } },               // ISCL_KEYCODE_NUMPAD_5        = 149
  { Qt::Key_6, { 0x0036, 0xffff, 0xffff } },               // ISCL_KEYCODE_NUMPAD_6        = 150
  { Qt::Key_7, { 0x0037, 0xffff, 0xffff } },               // ISCL_KEYCODE_NUMPAD_7        = 151
  { Qt::Key_8, { 0x0038, 0xffff, 0xffff } },               // ISCL_KEYCODE_NUMPAD_8        = 152
  { Qt::Key_9, { 0x0039, 0xffff, 0xffff } },               // ISCL_KEYCODE_NUMPAD_9        = 153
  { Qt::Key_Slash, { 0x002f, 0xffff, 0xffff } },           // ISCL_KEYCODE_NUMPAD_DIVIDE   = 154
  { Qt::Key_Asterisk, { 0x002a, 0xffff, 0xffff } },        // ISCL_KEYCODE_NUMPAD_MULTIPLY = 155
  { Qt::Key_Minus, { 0x002d, 0xffff, 0xffff } },           // ISCL_KEYCODE_NUMPAD_SUBTRACT = 156
  { Qt::Key_Plus, { 0x002b, 0xffff, 0xffff } },            // ISCL_KEYCODE_NUMPAD_ADD      = 157
  { Qt::Key_Period, { 0x002e, 0xffff, 0xffff } },          // ISCL_KEYCODE_NUMPAD_DOT      = 158
  { Qt::Key_Comma, { 0x002c, 0xffff, 0xffff } },           // ISCL_KEYCODE_NUMPAD_COMMA    = 159
  { Qt::Key_Enter, { 0xffff, 0xffff, 0xffff } },           // ISCL_KEYCODE_NUMPAD_ENTER    = 160
  { Qt::Key_Equal, { 0x003d, 0xffff, 0xffff } },           // ISCL_KEYCODE_NUMPAD_EQUALS   = 161
  { Qt::Key_ParenLeft, { 0x0028, 0xffff, 0xffff } },       // ISCL_KEYCODE_NUMPAD_LEFT_PAREN = 162
  { Qt::Key_ParenRight, { 0x0029, 0xffff, 0xffff } },      // ISCL_KEYCODE_NUMPAD_RIGHT_PAREN = 163
  { Qt::Key_VolumeMute, { 0xffff, 0xffff, 0xffff } },      // ISCL_KEYCODE_VOLUME_MUTE     = 164
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_INFO            = 165
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_CHANNEL_UP      = 166
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_CHANNEL_DOWN    = 167
  { Qt::Key_ZoomIn, { 0xffff, 0xffff, 0xffff } },          // ISCL_KEYCODE_ZOOM_IN         = 168
  { Qt::Key_ZoomOut, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_ZOOM_OUT        = 169
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_TV              = 170
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_WINDOW          = 171
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_GUIDE           = 172
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_DVR             = 173
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BOOKMARK        = 174
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_CAPTIONS        = 175
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_SETTINGS        = 176
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_TV_POWER        = 177
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_TV_INPUT        = 178
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_STB_POWER       = 179
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_STB_INPUT       = 180
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_AVR_POWER       = 181
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_AVR_INPUT       = 182
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_PROG_RED        = 183
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_PROG_GREEN      = 184
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_PROG_YELLOW     = 185
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_PROG_BLUE       = 186
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_APP_SWITCH      = 187
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_1        = 188
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_2        = 189
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_3        = 190
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_4        = 191
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_5        = 192
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_6        = 193
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_7        = 194
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_8        = 195
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_9        = 196
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_10       = 197
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_11       = 198
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_12       = 199
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_13       = 200
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_14       = 201
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_15       = 202
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_BUTTON_16       = 203
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_LANGUAGE_SWITCH = 204
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_MANNER_MODE     = 205
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_3D_MODE         = 206
  { Qt::Key_unknown, { 0xffff, 0xffff, 0xffff } },         // ISCL_KEYCODE_CONTACTS        = 207
  { Qt::Key_Calendar, { 0xffff, 0xffff, 0xffff } },        // ISCL_KEYCODE_CALENDAR        = 208
  { Qt::Key_Music, { 0xffff, 0xffff, 0xffff } },           // ISCL_KEYCODE_MUSIC           = 209
  { Qt::Key_Calculator, { 0xffff, 0xffff, 0xffff } }       // ISCL_KEYCODE_CALCULATOR      = 210
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
      DLOG("unhandled motion event action %d", event->action & ISCL_MOTION_EVENT_ACTION_MASK);
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

  // Key modifier and unicode index mapping.
  const int kMetaState = event->meta_state;
  Qt::KeyboardModifiers modifiers = Qt::NoModifier;
  int unicodeIndex = 0;
  if (kMetaState & ISCL_META_SHIFT_ON) {
    modifiers |= Qt::ShiftModifier;
    unicodeIndex = 1;
  }
  if (kMetaState & ISCL_META_CTRL_ON) {
    modifiers |= Qt::ControlModifier;
    unicodeIndex = 2;
  }
  if (kMetaState & ISCL_META_ALT_ON) {
    modifiers |= Qt::AltModifier;
    unicodeIndex = 2;
  }
  if (kMetaState & ISCL_META_META_ON) {
    modifiers |= Qt::MetaModifier;
    unicodeIndex = 2;
  }

  // Key event propagation.
  QEvent::Type keyType = kEventType[event->action];
  quint32 keyCode = kKeyCode[event->details.key.key_code].keycode;
  QString text(kKeyCode[event->details.key.key_code].unicode[unicodeIndex]);
  ulong timestamp = event->details.key.event_time / 1000000;
  QPlatformInputContext* context = QGuiApplicationPrivate::platformIntegration()->inputContext();
  if (context) {
    QKeyEvent qKeyEvent(keyType, keyCode, modifiers, text);
    qKeyEvent.setTimestamp(timestamp);
    if (context->filterEvent(&qKeyEvent)) {
      DLOG("key event filtered out by input context");
      return;
    }
  }
  handleKeyEvent(window, timestamp, keyType, keyCode, modifiers, text);
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
