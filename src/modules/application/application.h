
// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef APPLICATION_H
#define APPLICATION_H

#include <Qt/QtCore>

class DesktopData;

class Application : public QObject {
  Q_OBJECT
  Q_ENUMS(State)
  Q_PROPERTY(QString desktopFile READ desktopFile NOTIFY desktopFileChanged)
  Q_PROPERTY(QString name READ name NOTIFY nameChanged)
  Q_PROPERTY(QString comment READ comment NOTIFY commentChanged)
  Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
  Q_PROPERTY(QString exec READ exec NOTIFY execChanged)
  Q_PROPERTY(qint64 handle READ handle NOTIFY handleChanged)
  Q_PROPERTY(State state READ state NOTIFY stateChanged)

 public:
  enum State { Starting, Running };

  Application(DesktopData* desktopData, qint64 pid, State state, int timerId);
  ~Application();

  QString desktopFile() const;
  QString name() const;
  QString comment() const;
  QString icon() const;
  QString exec() const;
  qint64 handle() const;
  State state() const;

 Q_SIGNALS:
  void desktopFileChanged();
  void nameChanged();
  void commentChanged();
  void iconChanged();
  void execChanged();
  void handleChanged();
  void stateChanged();

 private:
  void setState(State state);
  int timerId() const { return timerId_; }

  DesktopData* desktopData_;
  qint64 pid_;
  State state_;
  int timerId_;

  friend class ApplicationManager;
  friend class ApplicationListModel;
};

Q_DECLARE_METATYPE(Application*)

#endif  // APPLICATION_H
