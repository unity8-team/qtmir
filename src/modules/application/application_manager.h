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

#ifndef APPLICATION_MANAGER_H
#define APPLICATION_MANAGER_H

#include <QtCore/QtCore>
#include <ubuntu/application/ui/window.h>
#include <ubuntu/application/ui/options.h>
#include <ubuntu/application/ui/input/event.h>
#include <ubuntu/ui/ubuntu_ui_session_service.h>

/* FIXME: undef required so that this class compiles properly.
   '#define Bool int' is part of <X11/Xlib.h> which is included
   by the following chain of includes:
   - <EGL/eglplatform.h> included by
   - <EGL/egl.h> included by
   - <ubuntu/application/ui/window.h>
*/
#undef Bool

// unity-api
#include <unity/shell/application/ApplicationManagerInterface.h>

// local
#include "application.h"

class ApplicationManager : public unity::shell::application::ApplicationManagerInterface {
  Q_OBJECT
  Q_ENUMS(SurfaceRole)
  Q_ENUMS(StageHint)
  Q_ENUMS(FormFactorHint)
  Q_ENUMS(FavoriteApplication)
  Q_FLAGS(ExecFlags)

  // FIXME(kaleo, loicm): That keyboard API might need a cleaner design.
  Q_PROPERTY(int keyboardHeight READ keyboardHeight NOTIFY keyboardHeightChanged)
  Q_PROPERTY(bool keyboardVisible READ keyboardVisible NOTIFY keyboardVisibleChanged)

  Q_PROPERTY(int sideStageWidth READ sideStageWidth CONSTANT)

 public:
  ApplicationManager(QObject *parent = nullptr);
  ~ApplicationManager();

  // Mapping enums to Ubuntu Platform API enums.
  enum SurfaceRole {
    Dash = U_DASH_ROLE, Default = U_MAIN_ROLE, Indicators = U_INDICATOR_ROLE,
    Notifications = U_NOTIFICATIONS_ROLE, Greeter = U_GREETER_ROLE,
    Launcher = U_LAUNCHER_ROLE, OnScreenKeyboard = U_ON_SCREEN_KEYBOARD_ROLE,
    ShutdownDialog = U_SHUTDOWN_DIALOG_ROLE
  };
  enum StageHint {
    MainStage = U_MAIN_STAGE, IntegrationStage = U_INTEGRATION_STAGE,
    ShareStage = U_SHARE_STAGE, ContentPickingStage = U_CONTENT_PICKING_STAGE,
    SideStage = U_SIDE_STAGE, ConfigurationStage = U_CONFIGURATION_STAGE
  };
  enum FormFactorHint {
    DesktopFormFactor = U_DESKTOP, PhoneFormFactor = U_PHONE,
    TabletFormFactor = U_TABLET
  };
  enum FavoriteApplication {
    CameraApplication = CAMERA_APP, GalleryApplication = GALLERY_APP,
    BrowserApplication = BROWSER_APP, ShareApplication = SHARE_APP,
    PhoneApplication = PHONE_APP, DialerApplication = DIALER_APP,
    MessagingApplication = MESSAGING_APP, AddressbookApplication = ADDRESSBOOK_APP
  };
  enum Flag {
    NoFlag = 0x0,
    ForceMainStage = 0x1,
  };
  Q_DECLARE_FLAGS(ExecFlags, Flag)

  // QObject methods.
  void customEvent(QEvent* event);
  void timerEvent(QTimerEvent* event);

  int keyboardHeight() const;
  bool keyboardVisible() const;
  int sideStageWidth() const;

  // QAbstractItemModel methods.
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  Q_INVOKABLE Application *get(int index) const override;
  Q_INVOKABLE Application *findApplication(const QString &appId) const override;

  Q_INVOKABLE void move(int from, int to);

  // Application control methods
  Q_INVOKABLE bool focusApplication(const QString &appId) override;
  Q_INVOKABLE void focusFavoriteApplication(FavoriteApplication application);
  Q_INVOKABLE void unfocusCurrentApplication() override;
  Q_INVOKABLE Application *startApplication(const QString &appId, const QStringList &arguments = QStringList()) override;
  Q_INVOKABLE Application *startApplication(const QString &appId, ExecFlags flags, const QStringList &arguments = QStringList());
  Q_INVOKABLE bool stopApplication(const QString &appId) override;

  QString focusedApplicationId() const override;

  QEvent::Type eventType() { return eventType_; }
  QEvent::Type keyboardGeometryEventType() { return keyboardGeometryEventType_; }

 Q_SIGNALS:
  void keyboardHeightChanged();
  void keyboardVisibleChanged();
  void focusRequested(FavoriteApplication favoriteApplication);

 private:
  void killProcess(qint64 pid);

  void add(Application *application);
  void remove(Application* application);
  Application* findFromTimerId(int timerId);

  int keyboardHeight_;
  bool keyboardVisible_;
  QList<Application*> applications_;
  QHash<int,Application*> pidHash_;
  QEvent::Type eventType_;
  QEvent::Type keyboardGeometryEventType_;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ApplicationManager::ExecFlags)

Q_DECLARE_METATYPE(ApplicationManager*)

#endif  // APPLICATION_MANAGER_H
