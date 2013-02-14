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

#include "integration.h"
#include "window.h"
#include "input.h"
#include "clipboard.h"
#include "base/logging.h"
#include <QtCore/QCoreApplication>
#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatforminputcontextfactory_p.h>
#include <qpa/qplatforminputcontext.h>

static void resumedCallback(void* context) {
  DLOG("resumedCallback (context=%p)", context);
  DASSERT(context != NULL);
  // FIXME(loicm) Add support for resumed callback.
  // QUbuntuScreen* screen = static_cast<QUbuntuScreen*>(context);
}

static void suspendedCallback(void* context) {
  DLOG("suspendedCallback (context=%p)", context);
  DASSERT(context != NULL);
  // FIXME(loicm) Add support for suspended callback.
  // QUbuntuScreen* screen = static_cast<QUbuntuScreen*>(context);
}

static void focusedCallback(void* context) {
  DLOG("focusedCallback (context=%p)", context);
  DASSERT(context != NULL);
  // FIXME(loicm) Add support for focused callback.
  // QUbuntuScreen* screen = static_cast<QUbuntuScreen*>(context);
}

static void unfocusedCallback(void* context) {
  DLOG("unfocusedCallback (context=%p)", context);
  DASSERT(context != NULL);
  // FIXME(loicm) Add support for unfocused callback.
  // QUbuntuScreen* screen = static_cast<QUbuntuScreen*>(context);
}

QUbuntuIntegration::QUbuntuIntegration()
    : clipboard_(new QUbuntuClipboard()) {
  // Init Ubuntu Platform library.
  QStringList args = QCoreApplication::arguments();
  argc_ = args.size() + 1;
  argv_ = new char*[argc_];
  for (int i = 0; i < argc_ - 1; i++)
    argv_[i] = qstrdup(args.at(i).toLocal8Bit());
  argv_[argc_ - 1] = NULL;
  ubuntu_application_ui_init(argc_ - 1, argv_);

  // Create default screen.
  screen_ = new QUbuntuScreen();
  screenAdded(screen_);

  // Initialize input.
  if (qEnvironmentVariableIsEmpty("QTUBUNTU_NO_INPUT")) {
    input_ = new QUbuntuInput(this);
    inputContext_ = QPlatformInputContextFactory::create();
  } else {
    input_ = NULL;
    inputContext_ = NULL;
  }

  DLOG("QUbuntuIntegration::QUbuntuIntegration (this=%p)", this);
}

QUbuntuIntegration::~QUbuntuIntegration() {
  DLOG("QUbuntuIntegration::~QUbuntuIntegration");
  delete clipboard_;
  delete input_;
  delete inputContext_;
  delete screen_;
  for (int i = 0; i < argc_; i++)
    delete [] argv_[i];
  delete [] argv_;
}

QPlatformWindow* QUbuntuIntegration::createPlatformWindow(QWindow* window) const {
  DLOG("QUbuntuIntegration::createPlatformWindow const (this=%p, window=%p)", this, window);
  return const_cast<QUbuntuIntegration*>(this)->createPlatformWindow(window);
}

QPlatformWindow* QUbuntuIntegration::createPlatformWindow(QWindow* window) {
  DLOG("QUbuntuIntegration::createPlatformWindow (this=%p, window=%p)", this, window);
  static uint sessionType;

  // Start a session before creating the first window.
  static bool once = false;
  if (!once) {
    sessionType = nativeInterface()->property("session").toUInt();
    // FIXME(loicm) Remove that once all system applications have been ported to the new property.
    if (sessionType == 0) {
      sessionType = nativeInterface()->property("ubuntuSessionType").toUInt();
    }
#if !defined(QT_NO_DEBUG)
    ASSERT(sessionType <= SYSTEM_SESSION_TYPE);
    const char* const sessionTypeString[] = {
      "User", "System"
    };
    const char* const stageHintString[] = {
      "Main", "Integration", "Share", "Content picking", "Side", "Configuration",
    };
    const char* const formFactorHintString[] = {
      "Desktop", "Phone", "Tablet"
    };
    LOG("ubuntu session type: '%s'", sessionTypeString[sessionType]);
    LOG("ubuntu application stage hint: '%s'",
        stageHintString[ubuntu_application_ui_setup_get_stage_hint()]);
    LOG("ubuntu application form factor: '%s'",
        formFactorHintString[ubuntu_application_ui_setup_get_form_factor_hint()]);
#endif
    SessionCredentials credentials = {
      static_cast<SessionType>(sessionType), APPLICATION_SUPPORTS_OVERLAYED_MENUBAR, "QtUbuntu",
      resumedCallback, suspendedCallback, focusedCallback, unfocusedCallback, this
    };
    ubuntu_application_ui_start_a_new_session(&credentials);
    input_->setSessionType(sessionType);
    once = true;
  }

  // Create the window.
  QPlatformWindow* platformWindow = new QUbuntuWindow(
      window, static_cast<QUbuntuScreen*>(screen_), input_, static_cast<bool>(sessionType));
  platformWindow->requestActivateWindow();
  return platformWindow;
}
