// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef APPLICATION_MANAGER_H
#define APPLICATION_MANAGER_H

#include <Qt/QtCore>
#include "ubuntu/application/ui/ubuntu_application_ui.h"
#include "ubuntu/ui/ubuntu_ui_session_service.h"

class Application;
class ApplicationListModel;

class ApplicationManager : public QObject {
  Q_OBJECT
  Q_ENUMS(StageHint)
  Q_ENUMS(FormFactorHint)
  Q_ENUMS(FavoriteApplication)
  Q_PROPERTY(StageHint stageHint READ stageHint)
  Q_PROPERTY(FormFactorHint formFactorHint READ formFactorHint)
  Q_PROPERTY(ApplicationListModel* applications READ applications NOTIFY applicationsChanged)

 public:
  ApplicationManager();
  ~ApplicationManager();

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

  StageHint stageHint() const;
  FormFactorHint formFactorHint() const;
  ApplicationListModel* applications() const;

  Q_INVOKABLE void focusApplication(int applicationId);
  Q_INVOKABLE void focusFavoriteApplication(FavoriteApplication application);
  Q_INVOKABLE void startWatcher();

  QEvent::Type eventType() { return eventType_; }

 Q_SIGNALS:
  void applicationsChanged();

 private:
  Application* createApplication(const char* desktopFile, int id);

  ApplicationListModel* applications_;
  QHash<int,Application*> idHash_;
  QEvent::Type eventType_;
};

Q_DECLARE_METATYPE(ApplicationManager*)

#endif  // APPLICATION_MANAGER_H
