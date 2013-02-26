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

#ifndef QUBUNTUBASEBACKINGSTORE_H
#define QUBUNTUBASEBACKINGSTORE_H

#include <qpa/qplatformbackingstore.h>

class QOpenGLContext;
class QOpenGLPaintDevice;

class QUbuntuBaseBackingStore : public QPlatformBackingStore {
 public:
  QUbuntuBaseBackingStore(QWindow* window);
  ~QUbuntuBaseBackingStore();

  // QPlatformBackingStore methods.
  void beginPaint(const QRegion&);
  void endPaint();
  void flush(QWindow* window, const QRegion& region, const QPoint& offset);
  void resize(const QSize& size, const QRegion& staticContents);
  QPaintDevice* paintDevice() { return reinterpret_cast<QPaintDevice*>(device_); }

 private:
  QOpenGLContext* context_;
  QOpenGLPaintDevice* device_;
};

#endif  // QUBUNTUBASEBACKINGSTORE_H
