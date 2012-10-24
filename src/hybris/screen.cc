// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "screen.h"
#include "base/logging.h"
#include <QtCore/QCoreApplication>
#include <ubuntu/application/ui/ubuntu_application_ui.h>

QHybrisScreen::QHybrisScreen() {
  // Init ubuntu application UI.
  QStringList args = QCoreApplication::arguments();
  const int size = args.size();
  argv_ = new char*[size + 1];
  for (int i = 0; i < size; i++)
    argv_[i] = args.at(i).toLocal8Bit().data();
  argv_[size] = NULL;
  ubuntu_application_ui_init(size, argv_);
  ubuntu_application_ui_start_a_new_session("QtHybris");
#if !defined(QT_NO_DEBUG)
  const char* const stageHintString[] = {
    "Main", "Integration", "Share", "Content picking", "Side", "Configuration",
  };
  const char* const formFactorHintString[] = {
    "Desktop", "Phone", "Tablet"
  };
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
}
