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

class Application;
class ApplicationListModel;

class DesktopData {
 public:
  DesktopData(QString desktopFile);
  ~DesktopData();

  QString file() const { return file_; }
  QString name() const { return entries_[kNameIndex]; }
  QString comment() const { return entries_[kCommentIndex]; }
  QString icon() const { return entries_[kIconIndex]; }
  QString exec() const { return entries_[kExecIndex]; }
  QString path() const { return entries_[kPathIndex]; }
  QString stageHint() const { return entries_[kStageHintIndex]; }
  bool loaded() const { return loaded_; }

 private:
  static const int kNameIndex = 0,
    kCommentIndex = 1,
    kIconIndex = 2,
    kExecIndex = 3,
    kPathIndex = 4,
    kStageHintIndex = 5,
    kNumberOfEntries = 6;

  bool loadDesktopFile(QString desktopFile);

  QString file_;
  QVector<QString> entries_;
  bool loaded_;
};

class ApplicationManager : public QObject {
  Q_OBJECT
  Q_ENUMS(Role)
  Q_ENUMS(StageHint)
  Q_ENUMS(FormFactorHint)
  Q_ENUMS(FavoriteApplication)
  Q_FLAGS(ExecFlags)
  
  // FIXME(kaleo, loicm): That keyboard API might need a cleaner design.
  Q_PROPERTY(int keyboardHeight READ keyboardHeight NOTIFY keyboardHeightChanged)
  Q_PROPERTY(bool keyboardVisible READ keyboardVisible NOTIFY keyboardVisibleChanged)

  Q_PROPERTY(int sideStageWidth READ sideStageWidth)
  Q_PROPERTY(ApplicationListModel* mainStageApplications READ mainStageApplications
             NOTIFY mainStageApplicationsChanged)
  Q_PROPERTY(ApplicationListModel* sideStageApplications READ sideStageApplications
             NOTIFY sideStageApplicationsChanged)
  Q_PROPERTY(Application* mainStageFocusedApplication READ mainStageFocusedApplication
             NOTIFY mainStageFocusedApplicationChanged)
  Q_PROPERTY(Application* sideStageFocusedApplication READ sideStageFocusedApplication
             NOTIFY sideStageFocusedApplicationChanged)

 public:
  ApplicationManager();
  ~ApplicationManager();

  // Mapping enums to Ubuntu Platform API enums.
  enum Role {
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
  ApplicationListModel* mainStageApplications() const;
  ApplicationListModel* sideStageApplications() const;
  Application* mainStageFocusedApplication() const;
  Application* sideStageFocusedApplication() const;

  Q_INVOKABLE void focusApplication(int handle);
  Q_INVOKABLE void focusFavoriteApplication(FavoriteApplication application);
  Q_INVOKABLE void unfocusCurrentApplication(StageHint stageHint);
  Q_INVOKABLE Application* startProcess(QString desktopFile, ExecFlags flags, QStringList arguments = QStringList());
  Q_INVOKABLE void stopProcess(Application* application);
  Q_INVOKABLE void startWatcher() {}

  QEvent::Type eventType() { return eventType_; }
  QEvent::Type keyboardGeometryEventType() { return keyboardGeometryEventType_; }

 Q_SIGNALS:
  void keyboardHeightChanged();
  void keyboardVisibleChanged();
  void mainStageApplicationsChanged();
  void sideStageApplicationsChanged();
  void mainStageFocusedApplicationChanged();
  void sideStageFocusedApplicationChanged();
  void focusRequested(FavoriteApplication favoriteApplication);

 private:
  void killProcess(qint64 pid);

  int keyboardHeight_;
  bool keyboardVisible_;
  ApplicationListModel* mainStageApplications_;
  ApplicationListModel* sideStageApplications_;
  Application* mainStageFocusedApplication_;
  Application* sideStageFocusedApplication_;
  QHash<int,Application*> pidHash_;
  QEvent::Type eventType_;
  QEvent::Type keyboardGeometryEventType_;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ApplicationManager::ExecFlags)

Q_DECLARE_METATYPE(ApplicationManager*)

#endif  // APPLICATION_MANAGER_H
