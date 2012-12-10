// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef APPLICATION_WINDOW_H
#define APPLICATION_WINDOW_H

#include <Qt/QtCore>
#include "ubuntu/application/ui/ubuntu_application_ui.h"

class ApplicationWindow : public QObject {
  Q_OBJECT
  Q_ENUMS(Role)
  Q_PROPERTY(Role role READ role WRITE setRole NOTIFY roleChanged)

 public:
  explicit ApplicationWindow(QObject* parent);
  ~ApplicationWindow();

  enum Role {
    Dash = DASH_ACTOR_ROLE, Main = MAIN_ACTOR_ROLE, Indicators = INDICATOR_ACTOR_ROLE,
    Notifications = NOTIFICATIONS_ACTOR_ROLE, Greeter = GREETER_ACTOR_ROLE,
    Launcher = LAUNCHER_ACTOR_ROLE, OnScreenKeyboard = ON_SCREEN_KEYBOARD_ACTOR_ROLE,
    ShutdownDialog = SHUTDOWN_DIALOG_ACTOR_ROLE
  };

  Role role() const { return role_; }
  void setRole(Role role);

 Q_SIGNALS:
  void roleChanged();

 private:
  Role role_;
};

#endif  // APPLICATION_WINDOW_H
