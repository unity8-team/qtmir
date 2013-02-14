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

#include "application_list_model.h"
#include "application.h"
#include "logging.h"

ApplicationListModel::ApplicationListModel(QObject* parent)
    : QAbstractListModel(parent)
    , applications_() {
  roleNames_.insert(0, "application");
  DLOG("ApplicationListModel::ApplicationListModel (this=%p, parent=%p)", this, parent);
}

ApplicationListModel::~ApplicationListModel() {
  DLOG("ApplicationListModel::~ApplicationListModel");
  const int kSize = applications_.size();
  for (int i = 0; i < kSize; i++)
    delete applications_.at(i);
  applications_.clear();
}

int ApplicationListModel::rowCount(const QModelIndex& parent) const {
  DLOG("ApplicationListModel::rowCount (this=%p)", this);
  return !parent.isValid() ? applications_.size() : 0;
}

QVariant ApplicationListModel::data(const QModelIndex& index, int role) const {
  DLOG("ApplicationListModel::data (this=%p, role=%d)", this, role);
  if (index.row() >= 0 && index.row() < applications_.size() && role == 0)
    return QVariant::fromValue(applications_.at(index.row()));
  else
    return QVariant();
}

QVariant ApplicationListModel::get(int row) const {
  DLOG("ApplicationListModel::get (this=%p, row=%d)", this, row);
  return data(index(row), 0);
}

void ApplicationListModel::move(int from, int to) {
  DLOG("ApplicationListModel::move (this=%p, from=%d, to=%d)", this, from, to);
  if (from >= 0 && from < applications_.size() && to >= 0 && to < applications_.size()) {
      QModelIndex parent;
    /* When moving an item down, the destination index needs to be incremented
       by one, as explained in the documentation:
       http://qt-project.org/doc/qt-5.0/qtcore/qabstractitemmodel.html#beginMoveRows */
    beginMoveRows(parent, from, from, parent, to + (to > from ? 1 : 0));
    applications_.move(from, to);
    endMoveRows();
  }
}

void ApplicationListModel::add(Application* application) {
  DASSERT(application != NULL);
  DLOG("ApplicationListModel::add (this=%p, application='%s')", this,
       application->name().toLatin1().data());
#if !defined(QT_NO_DEBUG)
  for (int i = 0; i < applications_.size(); i++)
    ASSERT(applications_.at(i) != application);
#endif
  beginInsertRows(QModelIndex(), applications_.size(), applications_.size());
  applications_.append(application);
  endInsertRows();
  emit countChanged();
}

void ApplicationListModel::remove(Application* application) {
  DASSERT(application != NULL);
  DLOG("ApplicationListModel::remove (this=%p, application='%s')", this,
       application->name().toLatin1().data());
  int i = applications_.indexOf(application);
  if (i != -1) {
    beginRemoveRows(QModelIndex(), i, i);
    applications_.removeAt(i);
    endRemoveRows();
    emit countChanged();
  }
}

Application* ApplicationListModel::findFromTimerId(int timerId) {
  DLOG("ApplicationListModel::findFromTimerId (this=%p, timerId=%d)", this, timerId);
  const int kSize = applications_.size();
  for (int i = 0; i < kSize; i++)
    if (applications_[i]->timerId() == timerId)
      return applications_[i];
  return NULL;
}
