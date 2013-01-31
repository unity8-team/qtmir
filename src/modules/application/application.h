// This file is part of QtUbuntu, a set of Qt components for Ubuntu.
// Copyright © 2013 Canonical Ltd.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; version 3.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QtCore/QtCore>

class DesktopData;

class Application : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString desktopFile READ desktopFile NOTIFY desktopFileChanged)
  Q_PROPERTY(QString name READ name NOTIFY nameChanged)
  Q_PROPERTY(QString comment READ comment NOTIFY commentChanged)
  Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
  Q_PROPERTY(QString exec READ exec NOTIFY execChanged)
  Q_PROPERTY(int handle READ handle NOTIFY handleChanged)

 public:
  Application(DesktopData* desktopData, QProcess* process, int handle);
  ~Application();

  QString desktopFile() const;
  QString name() const;
  QString comment() const;
  QString icon() const;
  QString exec() const;
  int handle() const;

 Q_SIGNALS:
  void desktopFileChanged();
  void nameChanged();
  void commentChanged();
  void iconChanged();
  void execChanged();
  void handleChanged();

 private:
  QProcess* process() const { return process_; }

  DesktopData* desktopData_;
  QProcess* process_;
  int handle_;

  friend class ApplicationManager;
};

Q_DECLARE_METATYPE(Application*)

#endif  // APPLICATION_H
