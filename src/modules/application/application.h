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

class DesktopData;

class Application : public QObject {
  Q_OBJECT
  Q_ENUMS(Stage)
  Q_ENUMS(State)
  Q_PROPERTY(QString desktopFile READ desktopFile NOTIFY desktopFileChanged)
  Q_PROPERTY(QString name READ name NOTIFY nameChanged)
  Q_PROPERTY(QString comment READ comment NOTIFY commentChanged)
  Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
  Q_PROPERTY(QString exec READ exec NOTIFY execChanged)
  Q_PROPERTY(qint64 handle READ handle NOTIFY handleChanged)
  Q_PROPERTY(Stage stage READ stage NOTIFY stageChanged)
  Q_PROPERTY(State state READ state NOTIFY stateChanged)
  Q_PROPERTY(bool fullscreen READ fullscreen NOTIFY fullscreenChanged)

 public:
  enum Stage { MainStage, SideStage };
  enum State { Starting, Running };

  Application(DesktopData* desktopData, qint64 pid, Stage stage, State state, int timerId);
  ~Application();

  QString desktopFile() const;
  QString name() const;
  QString comment() const;
  QString icon() const;
  QString exec() const;
  qint64 handle() const;
  Stage stage() const;
  State state() const;
  bool fullscreen() const;

 Q_SIGNALS:
  void desktopFileChanged();
  void nameChanged();
  void commentChanged();
  void iconChanged();
  void execChanged();
  void handleChanged();
  void stageChanged();
  void stateChanged();
  void fullscreenChanged();

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
