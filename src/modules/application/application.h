
// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef APPLICATION_H
#define APPLICATION_H

#include <Qt/QtCore>

class DesktopData;

class Application : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString desktopFile READ desktopFile NOTIFY desktopFileChanged)
  Q_PROPERTY(QString name READ name NOTIFY nameChanged)
  Q_PROPERTY(QString comment READ comment NOTIFY commentChanged)
  Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
  Q_PROPERTY(QString exec READ exec NOTIFY execChanged)
  Q_PROPERTY(qint64 handle READ handle NOTIFY handleChanged)
  Q_PROPERTY(bool focused READ focused NOTIFY focusedChanged)

 public:
  Application(DesktopData* desktopData, qint64 pid);
  ~Application();

  QString desktopFile() const;
  QString name() const;
  QString comment() const;
  QString icon() const;
  QString exec() const;
  qint64 handle() const;
  bool focused() const;

 Q_SIGNALS:
  void desktopFileChanged();
  void nameChanged();
  void commentChanged();
  void iconChanged();
  void execChanged();
  void handleChanged();
  void focusedChanged();

 private:
  void setFocused(bool focused);

  DesktopData* desktopData_;
  qint64 pid_;
  bool focused_;

  friend class ApplicationManager;
};

Q_DECLARE_METATYPE(Application*)

#endif  // APPLICATION_H
