// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef APPLICATION_MANAGER_H
#define APPLICATION_MANAGER_H

#include <Qt/QtCore>

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

  enum StageHint { Main = 0, Integration, Share, ContentPicking, Side, Configuration };
  enum FormFactorHint { Desktop = 0, Phone, Tablet };
  enum FavoriteApplication { Camera = 0, Gallery, Browser };

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
