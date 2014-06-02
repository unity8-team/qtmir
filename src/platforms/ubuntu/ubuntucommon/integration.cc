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
#include "input_adaptor_factory.h"
#include "base/logging.h"
#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatforminputcontextfactory_p.h>
#include <qpa/qplatforminputcontext.h>
#include <ubuntu/application/lifecycle_delegate.h>
#include <ubuntu/application/id.h>
#include <ubuntu/application/options.h>
#include <ubuntu/application/ui/options.h>
#include <ubuntu/application/ui/session.h>

static void resumedCallback(const UApplicationOptions *options, void* context) {
  DLOG("resumedCallback (options=%p, context=%p)", options, context);
  DASSERT(context != NULL);
  QUbuntuIntegration* integration = static_cast<QUbuntuIntegration*>(context);
  integration->screen()->toggleSensors(true);
  QCoreApplication::postEvent(QCoreApplication::instance(), new QEvent(QEvent::ApplicationActivate));

  Q_FOREACH(QWindow *window, QGuiApplication::allWindows()) {
    QGuiApplication::postEvent(window, new QExposeEvent( window->geometry() ));
  }
}

static void aboutToStopCallback(UApplicationArchive *archive, void* context) {
  DLOG("aboutToStopCallback (archive=%p, context=%p)", archive, context);
  DASSERT(context != NULL);
  QUbuntuIntegration* integration = static_cast<QUbuntuIntegration*>(context);
  integration->screen()->toggleSensors(false);
  integration->inputContext()->hideInputPanel();

  Q_FOREACH(QWindow *window, QGuiApplication::allWindows()) {
    QGuiApplication::postEvent(window, new QExposeEvent( QRegion() ));
  }

  QCoreApplication::postEvent(QCoreApplication::instance(), new QEvent(QEvent::ApplicationDeactivate));
}

QUbuntuIntegration::QUbuntuIntegration(QUbuntuInputAdaptorFactory *input_factory)
    : clipboard_(new QUbuntuClipboard()) {
  // Init Ubuntu Platform library.
  QStringList args = QCoreApplication::arguments();
  argc_ = args.size() + 1;
  argv_ = new char*[argc_];
  for (int i = 0; i < argc_ - 1; i++)
    argv_[i] = qstrdup(args.at(i).toLocal8Bit());
  argv_[argc_ - 1] = NULL;
  // Setup options
  options_ = u_application_options_new_from_cmd_line(argc_ - 1, argv_);

  // Setup application description
  desc_ = u_application_description_new();
  UApplicationId* id = u_application_id_new_from_stringn("QtUbuntu", 8);
  u_application_description_set_application_id(desc_, id);
  UApplicationLifecycleDelegate* delegate = u_application_lifecycle_delegate_new();
  u_application_lifecycle_delegate_set_application_resumed_cb(delegate, &resumedCallback);
  u_application_lifecycle_delegate_set_application_about_to_stop_cb(delegate, &aboutToStopCallback);
  u_application_lifecycle_delegate_set_context(delegate, this);
  u_application_description_set_application_lifecycle_delegate(desc_, delegate);

  // Create new application instance
  instance_ = u_application_instance_new_from_description_with_options(desc_, options_);

  if (instance_ == NULL)
    qFatal("QUbuntu: Could not create application instance");

  // Create default screen.
  screen_ = new QUbuntuScreen(options_);
  screenAdded(screen_);

  // FIXME (ricmm) We shouldn't disable sensors for the shell process
  // it is only valid right now because the shell doesnt use them
  screen_->toggleSensors(false);
  isShell_ = false;
  if (args.contains("unity8") || args.contains("/usr/bin/unity8") ||
      args.contains("unity8-greeter") || args.contains("/usr/bin/unity8-greeter"))
    isShell_ = true;

  // Initialize input.
  if (qEnvironmentVariableIsEmpty("QTUBUNTU_NO_INPUT")) {
    input_ = input_factory->create_input_adaptor(this);
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
    ASSERT(sessionType <= U_SYSTEM_SESSION);
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
        stageHintString[u_application_options_get_stage(options_)]);
    LOG("ubuntu application form factor: '%s'",
        formFactorHintString[u_application_options_get_form_factor(options_)]);
#endif
  
    LOG("callbacks %p %p", &resumedCallback, &aboutToStopCallback);
    
    props_ = ua_ui_session_properties_new();
    ua_ui_session_properties_set_type(props_, static_cast<UAUiSessionType>(sessionType));

    ua_ui_session_properties_set_remote_pid(props_,
      static_cast<uint32_t>(QCoreApplication::applicationPid()));

    session_ = ua_ui_session_new_with_properties(props_);

    input_->setSessionType(sessionType);
    once = true;
  }

  QStringList args = QCoreApplication::arguments();
    
  // Create the window.
  QPlatformWindow* platformWindow = new QUbuntuWindow(
      window, static_cast<QUbuntuScreen*>(screen_), input_, static_cast<bool>(sessionType), instance_, isShell_);
  platformWindow->requestActivateWindow();
  return platformWindow;
}
