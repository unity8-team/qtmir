// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "application_manager.h"
#include "logging.h"
#include <ubuntu/ui/ubuntu_ui_session_service.h>
#include <ubuntu/application/ui/ubuntu_application_ui.h>

// static sessionBornCallback(ubuntu_ui_session_properties props, void* context) {
//   DLOG("sessionBornCallback (thread=%p, props=%p, context=%p)", QThread::currentThread(),
//        props, context);
//   DASSERT(context != NULL);
//   // ApplicationManager* manager = static_cast<ApplicationManager*>(context);
// }

// static sessionDiedCallback(ubuntu_ui_session_properties props, void* context) {
//   DLOG("sessionDiedCallback (thread=%p, props=%p, context=%p)", QThread::currentThread(),
//        props, context);
//   DASSERT(context != NULL);
//   // ApplicationManager* manager = static_cast<ApplicationManager*>(context);
// }

// static sessionFocusedCallback(ubuntu_ui_session_properties props, void* context) {
//   DLOG("sessionFocusedCallback (thread=%p, props=%p, context=%p)", QThread::currentThread(),
//        props, context);
//   DASSERT(context != NULL);
//   // ApplicationManager* manager = static_cast<ApplicationManager*>(context);
// }

ApplicationManager::ApplicationManager() {
      // : watching(false) {
  DLOG("ApplicationManager::ApplicationManager (thread=%p, this=%p)", QThread::currentThread(),
       this);
}

ApplicationManager::~ApplicationManager() {
  DLOG("ApplicationManager::~ApplicationManager");
}

ApplicationManager::StageHint ApplicationManager::stageHint() const {
  DLOG("ApplicationManager::stageHint (this=%p)", this);
  return static_cast<ApplicationManager::StageHint>(ubuntu_application_ui_setup_get_stage_hint());
}

ApplicationManager::FormFactorHint ApplicationManager::formFactorHint() const {
  DLOG("ApplicationManager::formFactorHint (this=%p)", this);
  return static_cast<ApplicationManager::FormFactorHint>(
      ubuntu_application_ui_setup_get_form_factor_hint());
}

void ApplicationManager::focusApplication(int applicationId) {
  DLOG("ApplicationManager::focusApplication (this=%p, applicationId=%d)", this, applicationId);
  ubuntu_ui_session_focus_running_session_with_id(applicationId);
}

void ApplicationManager::focusFavoriteApplication(
    ApplicationManager::FavoriteApplication application) {
  DLOG("ApplicationManager::focusFavoriteApplication (this=%p, application=%d)",
       this, static_cast<int>(application));
  ubuntu_ui_session_trigger_switch_to_well_known_application(
      static_cast<ubuntu_ui_well_known_application>(application));
}

// void ApplicationManager::startWatcher() {
//   DLOG("ApplicationManager::startWatcher (this=%p)", this);
//   ubuntu_ui_session_lifecycle_observer watcher = {
//     sessionBornCallback, sessionFocusedCallback, sessionDiedCallback, this
//   };
//   ubuntu_ui_session_install_session_lifecycle_observer(&watcher);
//   watching_ = true;
// }

// void ApplicationManager::stopWatcher() {
//   DLOG("ApplicationManager::stopWatcher (this=%p)", this);
//   watching_ = false;
// }
