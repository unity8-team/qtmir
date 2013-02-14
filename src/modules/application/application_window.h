// This file is part of QtUbuntu, a set of Qt components for Ubuntu.
// Copyright © 2013 Canonical Ltd.
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 3, as published by
// the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
// SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
