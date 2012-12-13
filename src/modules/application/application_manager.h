// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef APPLICATION_MANAGER_H
#define APPLICATION_MANAGER_H

#include <Qt/QtCore>
#include "ubuntu/application/ui/ubuntu_application_ui.h"
#include "ubuntu/ui/ubuntu_ui_session_service.h"

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
  bool loaded() const { return loaded_; }

 private:
  static const int kNameIndex = 0,
    kCommentIndex = 1,
    kIconIndex = 2,
    kExecIndex = 3,
    kNumberOfEntries = 4;

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
  Q_PROPERTY(StageHint stageHint READ stageHint)
  Q_PROPERTY(FormFactorHint formFactorHint READ formFactorHint)
  Q_PROPERTY(ApplicationListModel* applications READ applications NOTIFY applicationsChanged)

 public:
  ApplicationManager();
  ~ApplicationManager();

  // Mapping enums to Ubuntu application API enums.
  enum Role {
    Dash = DASH_ACTOR_ROLE, Default = MAIN_ACTOR_ROLE, Indicators = INDICATOR_ACTOR_ROLE,
    Notifications = NOTIFICATIONS_ACTOR_ROLE, Greeter = GREETER_ACTOR_ROLE,
    Launcher = LAUNCHER_ACTOR_ROLE, OnScreenKeyboard = ON_SCREEN_KEYBOARD_ACTOR_ROLE,
    ShutdownDialog = SHUTDOWN_DIALOG_ACTOR_ROLE
  };
  enum StageHint {
    Main = MAIN_STAGE_HINT, Integration = INTEGRATION_STAGE_HINT, Share = SHARE_STAGE_HINT,
    ContentPicking = CONTENT_PICKING_STAGE_HINT, Side = SIDE_STAGE_HINT,
    Configuration = CONFIGURATION_STAGE_HINT
  };
  enum FormFactorHint {
    Desktop = DESKTOP_FORM_FACTOR_HINT, Phone = PHONE_FORM_FACTOR_HINT,
    Tablet = TABLET_FORM_FACTOR_HINT
  };
  enum FavoriteApplication {
    Camera = CAMERA_APP, Gallery = GALLERY_APP, Browser = BROWSER_APP
  };

  // QObject methods.
  void customEvent(QEvent* event);
  void timerEvent(QTimerEvent* event);

  StageHint stageHint() const;
  FormFactorHint formFactorHint() const;
  ApplicationListModel* applications() const;

  Q_INVOKABLE void focusApplication(Application* application);
  Q_INVOKABLE void focusFavoriteApplication(FavoriteApplication application);
  Q_INVOKABLE void startProcess(QString desktopFile, QStringList arguments);
  Q_INVOKABLE void stopProcess(Application* application);

  QEvent::Type eventType() { return eventType_; }

 Q_SIGNALS:
  void applicationsChanged();

 private:
  struct Process {
    Process(DesktopData* desktopData, QStringList arguments, int timerId);
    ~Process();
    void clear();
    DesktopData* desktopData_;
    QProcess* process_;
    int timerId_;
  };

  ApplicationListModel* applications_;
  QHash<int,Application*> pidHash_;
  QList<Process> unmatchedProcesses_;
  QEvent::Type eventType_;
};

Q_DECLARE_METATYPE(ApplicationManager*)

#endif  // APPLICATION_MANAGER_H
