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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QtCore/QtCore>

// unity-api
#include <unity/shell/application/ApplicationInfoInterface.h>

class DesktopData;

using namespace unity::shell::application;

class Application : public ApplicationInfoInterface {
  Q_OBJECT

  Q_PROPERTY(bool fullscreen READ fullscreen NOTIFY fullscreenChanged)

 public:
  Application(DesktopData* desktopData, qint64 pid, Stage stage, State state, int timerId);
  ~Application();

  QString appId() const;
  QString name() const;
  QString comment() const;
  QUrl icon() const;
  Stage stage() const;
  State state() const;
  bool focused() const;
  bool fullscreen() const;

  // used internally, not for QML
  QString exec() const;
  qint64 pid() const;

 Q_SIGNALS:
  void fullscreenChanged(bool fullscreen);

 private:
  void setStage(Stage stage);
  void setState(State state);
  void setFullscreen(bool fullscreen);
  int timerId() const { return timerId_; }

  DesktopData* desktopData_;
  qint64 pid_;
  Stage stage_;
  State state_;
  bool fullscreen_;
  int timerId_;

  friend class ApplicationManager;
  friend class ApplicationListModel;
};

Q_DECLARE_METATYPE(Application*)

#endif  // APPLICATION_H
