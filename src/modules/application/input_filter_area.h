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

#ifndef INPUT_FILTER_AREA_H
#define INPUT_FILTER_AREA_H

#include <QtQuick/QQuickItem>
#include <QtCore/QLinkedList>
#include <QtCore/QMetaObject>

class InputFilterArea : public QQuickItem {
  Q_OBJECT
  Q_PROPERTY(bool blockInput READ blockInput WRITE setBlockInput NOTIFY blockInputChanged)

 public:
  explicit InputFilterArea(QQuickItem* parent = 0);
  ~InputFilterArea();

  bool blockInput() const { return blockInput_; }
  void setBlockInput(bool blockInput);

 Q_SIGNALS:
  void blockInputChanged();

 protected:
  virtual void geometryChanged(const QRectF & newGeometry, const QRectF & oldGeometry);

 private Q_SLOTS:
  void onAscendantChanged();
  void onAscendantGeometryChanged();

 private:
  void listenToAscendantsChanges();
  void disconnectFromAscendantsChanges();
  void setInputTrap(const QRect & geometry);
  void enableInputTrap();
  void disableInputTrap();
  QRect relativeToAbsoluteGeometry(QRectF relativeGeometry);

  bool blockInput_;
  unsigned int trapHandle_;
  QRectF geometry_;
  QRect trapGeometry_;
  QLinkedList<QMetaObject::Connection> connections_;
};

#endif  // INPUT_FILTER_AREA_H
