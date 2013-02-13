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

#include "input_filter_area.h"
#include "logging.h"
#include <QDebug>
#include <ubuntu/ui/ubuntu_ui_session_service.h>

InputFilterArea::InputFilterArea(QQuickItem* parent)
    : QQuickItem(parent)
    , blockInput_(false)
    , trapHandle_(0) {
  DLOG("InputFilterArea::InputFilterArea (this=%p, parent=%p)", this, parent);
  listenToAscendantsChanges();
  connect(this, &QQuickItem::parentChanged, this, &InputFilterArea::onAscendantChanged);
}

InputFilterArea::~InputFilterArea() {
  DLOG("InputFilterArea::~InputFilterArea");
  disableInputTrap();
}

void InputFilterArea::setBlockInput(bool blockInput) {
  DLOG("InputFilterArea::setBlockInput (this=%p, blockInput=%d)", this, blockInput);
  if (blockInput_ != blockInput) {
    blockInput_ = blockInput;
    if (blockInput) {
      enableInputTrap();
    } else {
      disableInputTrap();
    }
    emit blockInputChanged();
  }
}

void InputFilterArea::geometryChanged(const QRectF & newGeometry, const QRectF & oldGeometry) {
  DLOG("InputFilterArea::geometryChanged (this=%p)", this);
  qDebug() << newGeometry;
  if (blockInput_ && newGeometry != oldGeometry) {
    geometry_ = newGeometry.toRect();
    setInputTrap(newGeometry.toRect());
  }
  QQuickItem::geometryChanged(newGeometry, oldGeometry);
}

void InputFilterArea::onAscendantChanged() {
  DLOG("InputFilterArea::onAscendantChanged (this=%p)", this);
  listenToAscendantsChanges();
  updateTrap();
}

void InputFilterArea::onAscendantGeometryChanged() {
  DLOG("InputFilterArea::onAscendantGeometryChanged (this=%p)", this);
  updateTrap();
}

void InputFilterArea::listenToAscendantsChanges() {
  DLOG("InputFilterArea::listenToAscendantsChanges (this=%p)", this);

  // disconnect all previously connected signals
  Q_FOREACH (QMetaObject::Connection connection, connections_) {
    disconnect(connection);
  }
  connections_.clear();

  // listen to geometry changes and parent changes on all the ascendants
  QQuickItem* parent = parentItem();
  while (parent != NULL) {
    connections_.append(connect(parent, &QQuickItem::parentChanged, this, &InputFilterArea::onAscendantChanged));
    connections_.append(connect(parent, &QQuickItem::xChanged, this, &InputFilterArea::onAscendantGeometryChanged));
    connections_.append(connect(parent, &QQuickItem::yChanged, this, &InputFilterArea::onAscendantGeometryChanged));
    connections_.append(connect(parent, &QQuickItem::widthChanged, this, &InputFilterArea::onAscendantGeometryChanged));
    connections_.append(connect(parent, &QQuickItem::heightChanged, this, &InputFilterArea::onAscendantGeometryChanged));
    parent = parent->parentItem();
  }
}

void InputFilterArea::updateTrap() {
  DLOG("InputFilterArea::updateTrap (this=%p)", this);

  setInputTrap(geometry_);
}

void InputFilterArea::setInputTrap(const QRect & geometry) {
  DLOG("InputFilterArea::setInputTrap (this=%p)", this);
  qDebug() << geometry;

  disableInputTrap();

  if (blockInput_ && geometry.isValid()) {
    QRect sceneGeometry;
    if (parentItem()) {
      sceneGeometry = parentItem()->mapRectToScene(geometry).toRect();
    } else {
      sceneGeometry = geometry;
    }
    trapHandle_ = ubuntu_ui_set_surface_trap(sceneGeometry.x(), sceneGeometry.y(), sceneGeometry.width(), sceneGeometry.height());
  }
}

void InputFilterArea::enableInputTrap() {
  DLOG("InputFilterArea::enableInputTrap (this=%p)", this);
  setInputTrap(geometry_);
}

void InputFilterArea::disableInputTrap() {
  DLOG("InputFilterArea::disableInputTrap (this=%p)", this);
  if (trapHandle_ != 0) {
    ubuntu_ui_unset_surface_trap(trapHandle_);
    trapHandle_ = 0;
  }
}
