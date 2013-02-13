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
}

InputFilterArea::~InputFilterArea() {
  DLOG("InputFilterArea::~InputFilterArea");
}

void InputFilterArea::setBlockInput(bool blockInput) {
  DLOG("InputFilterArea::setBlockInput (this=%p, blockInput=%d)", this, blockInput);
  if (blockInput_ != blockInput) {
    blockInput_ = blockInput;
    setInputTrap(QRect(x(), y(), width(), height()));
    emit blockInputChanged();
  }
}

void InputFilterArea::geometryChanged(const QRectF & newGeometry, const QRectF & oldGeometry) {
  DLOG("InputFilterArea::geometryChanged (this=%p)", this);
  qDebug() << newGeometry;
  if (blockInput_) {
    setInputTrap(newGeometry.toRect());
  }
  QQuickItem::geometryChanged(newGeometry, oldGeometry);
}


void InputFilterArea::setInputTrap(const QRect & geometry) {
  DLOG("InputFilterArea::setInputTrap (this=%p)", this);
  qDebug() << geometry;

  if (trapHandle_ != 0) {
    ubuntu_ui_unset_surface_trap(trapHandle_);
  }

  if (blockInput_) {
    QRect sceneGeometry = mapRectToScene(geometry).toRect();
    qDebug() << sceneGeometry;
    trapHandle_ = ubuntu_ui_set_surface_trap(sceneGeometry.x(), sceneGeometry.y(), sceneGeometry.width(), sceneGeometry.height());
  }
}
