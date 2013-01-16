// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef APPLICATION_WINDOW_H
#define APPLICATION_WINDOW_H

#include <QtCore/QtCore>

class ApplicationWindow : public QObject {
  Q_OBJECT
  Q_PROPERTY(int role READ role WRITE setRole NOTIFY roleChanged)
  Q_PROPERTY(int opaque READ opaque WRITE setOpaque NOTIFY opaqueChanged)

 public:
  explicit ApplicationWindow(QObject* parent);
  ~ApplicationWindow();

  int role() const { return role_; }
  void setRole(int role);
  bool opaque() const { return static_cast<bool>(opaque_); }
  void setOpaque(bool opaque);

 Q_SIGNALS:
  void roleChanged();
  void opaqueChanged();

 private:
  int role_;
  int opaque_;
};

#endif  // APPLICATION_WINDOW_H
