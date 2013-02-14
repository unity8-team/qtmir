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

#include "backing_store.h"
#include "logging.h"
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLPaintDevice>

QUbuntuBaseBackingStore::QUbuntuBaseBackingStore(QWindow* window)
    : QPlatformBackingStore(window)
    , context_(new QOpenGLContext) {
  context_->setFormat(window->requestedFormat());
  context_->setScreen(window->screen());
  context_->create();
  DLOG("QUbuntuBaseBackingStore::QUbuntuBaseBackingStore (this=%p, window=%p)", this, window);
}

QUbuntuBaseBackingStore::~QUbuntuBaseBackingStore() {
  DLOG("QUbuntuBaseBackingStore::~QUbuntuBaseBackingStore");
  delete context_;
}

void QUbuntuBaseBackingStore::flush(QWindow* window, const QRegion& region, const QPoint& offset) {
  Q_UNUSED(region);
  Q_UNUSED(offset);
  DLOG("QUbuntuBaseBackingStore::flush (this=%p, window=%p)", this, window);
  context_->swapBuffers(window);
}

void QUbuntuBaseBackingStore::beginPaint(const QRegion& region) {
  Q_UNUSED(region);
  DLOG("QUbuntuBaseBackingStore::beginPaint (this=%p)", this);
  window()->setSurfaceType(QSurface::OpenGLSurface);
  context_->makeCurrent(window());
  device_ = new QOpenGLPaintDevice(window()->size());
}

void QUbuntuBaseBackingStore::endPaint() {
  DLOG("QUbuntuBaseBackingStore::endPaint (this=%p)", this);
  delete device_;
}

void QUbuntuBaseBackingStore::resize(const QSize& size, const QRegion& staticContents) {
  Q_UNUSED(size);
  Q_UNUSED(staticContents);
  DLOG("QUbuntuBaseBackingStore::resize (this=%p)", this);
}
