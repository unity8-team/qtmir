// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef APPLICATION_LIST_MODEL_H
#define APPLICATION_LIST_MODEL_H

#include <QtCore/QAbstractListModel>

class Application;

class ApplicationListModel : public QAbstractListModel {
  Q_OBJECT

 public:
  explicit ApplicationListModel(QObject* parent = 0);
  ~ApplicationListModel();

  // QAbstractItemModel methods.
  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  QVariant data(const QModelIndex& index, int role) const;
  QHash<int,QByteArray> roleNames() const { return roleNames_; }
  Q_INVOKABLE QVariant get(int index) const;

 private:
  Q_DISABLE_COPY(ApplicationListModel)

  void add(Application* application);
  void remove(Application* application);

  QHash<int,QByteArray> roleNames_;
  QVector<Application*> applications_;

  friend class ApplicationManager;
};

Q_DECLARE_METATYPE(ApplicationListModel*)

#endif  // APPLICATION_LIST_MODEL_H
