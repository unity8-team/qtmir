// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef UBUNTU_APPLICATION_PLUGIN_APPLICATION_MANAGER_H
#define UBUNTU_APPLICATION_PLUGIN_APPLICATION_MANAGER_H

#include <Qt/QtCore>

class ApplicationManager : public QObject {
  Q_OBJECT
  Q_ENUMS(StageHint)
  Q_ENUMS(FormFactorHint)
  Q_ENUMS(FavoriteApplication)
  Q_PROPERTY(StageHint stageHint READ stageHint)
  Q_PROPERTY(FormFactorHint formFactorHint READ formFactorHint)

 public:
  ApplicationManager();
  ~ApplicationManager();

  enum StageHint { Main = 0, Integration, Share, ContentPicking, Side, Configuration };
  enum FormFactorHint { Desktop = 0, Phone, Tablet };
  enum FavoriteApplication { Camera = 0, Gallery, Browser };

  StageHint stageHint() const;
  FormFactorHint formFactorHint() const;

  Q_INVOKABLE void focusApplication(int applicationId);
  Q_INVOKABLE void focusFavoriteApplication(FavoriteApplication application);
  // Q_INVOKABLE void startWatcher();
  // Q_INVOKABLE void stopWatcher();

 private:
  // bool watching_;
};

Q_DECLARE_METATYPE(ApplicationManager*)

#endif  // UBUNTU_APPLICATION_PLUGIN_APPLICATION_MANAGER_H
