
// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef APPLICATION_H
#define APPLICATION_H

#include <Qt/QtCore>

class DesktopData;

class Application : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString name READ name NOTIFY nameChanged)
  Q_PROPERTY(QString comment READ comment NOTIFY commentChanged)
  Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
  Q_PROPERTY(int handle READ handle NOTIFY handleChanged)
  Q_PROPERTY(bool focused READ focused NOTIFY focusedChanged)

 public:
  Application(DesktopData* desktopData, QProcess* process, int handle);
  ~Application();

  QString name() const;
  QString comment() const;
  QString icon() const;
  int handle() const;
  bool focused() const;

 Q_SIGNALS:
  void nameChanged();
  void commentChanged();
  void iconChanged();
  void handleChanged();
  void focusedChanged();

 private:
  QProcess* process() const { return process_; }
  void setFocused(bool focused);

  DesktopData* desktopData_;
  QProcess* process_;
  int handle_;
  bool focused_;

  friend class ApplicationManager;
};

Q_DECLARE_METATYPE(Application*)

#endif  // APPLICATION_H
