// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef APPLICATION_H
#define APPLICATION_H

#include <Qt/QtCore>

class Application : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString desktopFile READ desktopFile NOTIFY desktopFileChanged)
  Q_PROPERTY(QString name READ name NOTIFY nameChanged)
  Q_PROPERTY(QString comment READ comment NOTIFY commentChanged)
  Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
  Q_PROPERTY(int handle READ handle NOTIFY handleChanged)
  Q_PROPERTY(bool focused READ focused NOTIFY focusedChanged)

 public:
  Application() {}
  Application(const char* desktopFile, const char* name, const char* comment, const char* icon, int handle);
  ~Application();

  QString desktopFile() const { return desktopFile_; }
  QString name() const { return name_; }
  QString comment() const { return comment_; }
  QString icon() const { return icon_; }
  int handle() const { return handle_; }
  bool focused() const { return focused_; }

 Q_SIGNALS:
  void desktopFileChanged();
  void nameChanged();
  void commentChanged();
  void iconChanged();
  void handleChanged();
  void focusedChanged();

 private:
  void setFocused(bool focused);

  QString desktopFile_;
  QString name_;
  QString comment_;
  QString icon_;
  int handle_;
  bool focused_;

  friend class ApplicationManager;
};

Q_DECLARE_METATYPE(Application*)

#endif  // APPLICATION_H
