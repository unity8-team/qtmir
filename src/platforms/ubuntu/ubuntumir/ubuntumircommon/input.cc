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
#include "base/logging.h"

#include <QtCore/qglobal.h>
#include <QtCore/QCoreApplication>
#include <private/qguiapplication_p.h>
#include <qpa/qplatforminputcontext.h>
#include <qpa/qwindowsysteminterface.h>

#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-keysyms.h>

#include <ubuntu/application/ui/input/event.h>

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

QUbuntuMirInput::QUbuntuMirInput(QUbuntuIntegration* integration)
  : QUbuntuInput(integration) {
}

QUbuntuMirInput::~QUbuntuMirInput() {
}

static uint32_t translateKeysym(uint32_t sym, char *string, size_t size) {
  Q_UNUSED(size);
  string[0] = '\0';

  if (sym >= XKB_KEY_F1 && sym <= XKB_KEY_F35)
    return Qt::Key_F1 + (int(sym) - XKB_KEY_F1);

  for (int i = 0; KeyTable[i]; i += 2)
    if (sym == KeyTable[i])
      return KeyTable[i + 1];

  string[0] = sym;
  string[1] = '\0';
  return toupper(sym);
}

void QUbuntuMirInput::dispatchKeyEvent(QWindow* window, const void* ev) {
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

  handleKeyEvent(window, timestamp, keyType, sym, modifiers, text);
}
