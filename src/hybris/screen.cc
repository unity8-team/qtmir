// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "screen.h"
#include "base/logging.h"
#include <QtCore/QCoreApplication>
#include <ubuntu/application/ui/ubuntu_application_ui.h>

static void resumedCallback(void* context) {
  DLOG("resumedCallback (context=%p)", context);
  DASSERT(context != NULL);
  // FIXME(loicm) Add support for resumed callback.
  // QHybrisScreen* screen = static_cast<QHybrisScreen*>(context);
}

static void suspendedCallback(void* context) {
  DLOG("suspendedCallback (context=%p)", context);
  DASSERT(context != NULL);
  // FIXME(loicm) Add support for suspended callback.
  // QHybrisScreen* screen = static_cast<QHybrisScreen*>(context);
}

static void focusedCallback(void* context) {
  DLOG("focusedCallback (context=%p)", context);
  DASSERT(context != NULL);
  // FIXME(loicm) Add support for focused callback.
  // QHybrisScreen* screen = static_cast<QHybrisScreen*>(context);
}

static void unfocusedCallback(void* context) {
  DLOG("unfocusedCallback (context=%p)", context);
  DASSERT(context != NULL);
  // FIXME(loicm) Add support for unfocused callback.
  // QHybrisScreen* screen = static_cast<QHybrisScreen*>(context);
}

QHybrisScreen::QHybrisScreen(QHybrisBaseNativeInterface* nativeInterface) {
  // Init ubuntu application UI.
  QStringList args = QCoreApplication::arguments();
  const int size = args.size();
  argv_ = new char*[size + 1];
  for (int i = 0; i < size; i++)
    argv_[i] = args.at(i).toLocal8Bit().data();
  argv_[size] = NULL;
  ubuntu_application_ui_init(size, argv_);

  // Start a session.
  uint type = nativeInterface->property("UbuntuSessionType").toUInt();
  if (type > 2)
    type = 0;
  const SessionCredentials kCredentials = {
    type, APPLICATION_SUPPORTS_OVERLAYED_MENUBAR, "QtHybris", resumedCallback, suspendedCallback,
    focusedCallback, unfocusedCallback, this
  };
  ubuntu_application_ui_start_a_new_session(&kCredentials);
#if !defined(QT_NO_DEBUG)
  const char* const typeString[] = {
    "User", "System"
  };
  const char* const stageHintString[] = {
    "Main", "Integration", "Share", "Content picking", "Side", "Configuration",
  };
  const char* const formFactorHintString[] = {
    "Desktop", "Phone", "Tablet"
  };
  LOG("ubuntu session type: '%s'", typeString[type]);
  LOG("ubuntu application stage hint: '%s'",
      stageHintString[ubuntu_application_ui_setup_get_stage_hint()]);
  LOG("ubuntu application form factor: '%s'",
      formFactorHintString[ubuntu_application_ui_setup_get_form_factor_hint()]);
#endif

  // Store the screen size.
  // FIXME(loicm) Ubuntu application UI doesn't provide the screen size.
  const int kScreenWidth = 720;
  const int kScreenHeight = 1280;
  ASSERT(kScreenWidth > 0 && kScreenHeight > 0);
  geometry_ = QRect(0, 0, kScreenWidth, kScreenHeight);

  DLOG("QHybrisScreen::QHybrisScreen (this=%p)", this);
}

QHybrisScreen::~QHybrisScreen() {
  DLOG("QHybrisScreen::~QHybrisScreen");
  delete [] argv_;
}
