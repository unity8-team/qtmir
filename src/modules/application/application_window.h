// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef APPLICATION_WINDOW_H
#define APPLICATION_WINDOW_H

#include <Qt/QtCore>

class ApplicationWindow : public QObject {
  Q_OBJECT
  Q_PROPERTY(int role READ role WRITE setRole NOTIFY roleChanged)

 public:
  explicit ApplicationWindow(QObject* parent);
  ~ApplicationWindow();

  int role() const { return role_; }
  void setRole(int role);

 Q_SIGNALS:
  void roleChanged();

 private:
  int role_;
};

#endif  // APPLICATION_WINDOW_H
