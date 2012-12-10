// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

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
    beginMoveRows(QModelIndex(), from, from, QModelIndex(), to);
    applications_.move(from, to);
    endMoveRows();
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
  Q_EMIT countChanged();
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
      Q_EMIT countChanged();
  }
  DNOT_REACHED();
}
