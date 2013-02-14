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

#ifndef APPLICATION_LIST_MODEL_H
#define APPLICATION_LIST_MODEL_H

#include <QtCore/QAbstractListModel>

class Application;

class ApplicationListModel : public QAbstractListModel {
  Q_OBJECT

  Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

 public:
  explicit ApplicationListModel(QObject* parent = 0);
  ~ApplicationListModel();

  // QAbstractItemModel methods.
  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  QVariant data(const QModelIndex& index, int role) const;
  QHash<int,QByteArray> roleNames() const { return roleNames_; }
  Q_INVOKABLE QVariant get(int index) const;
  Q_INVOKABLE void move(int from, int to);

 Q_SIGNALS:
  void countChanged();

 private:
  Q_DISABLE_COPY(ApplicationListModel)

  void add(Application* application);
  void remove(Application* application);
  Application* findFromTimerId(int timerId);
#if !defined(QT_NO_DEBUG)
  bool contains(Application* application) const { return applications_.contains(application); }
#endif

  QHash<int,QByteArray> roleNames_;
  QList<Application*> applications_;

  friend class ApplicationManager;
};

Q_DECLARE_METATYPE(ApplicationListModel*)

#endif  // APPLICATION_LIST_MODEL_H
