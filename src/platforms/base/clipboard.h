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

#ifndef QUBUNTUBASECLIPBOARD_H
#define QUBUNTUBASECLIPBOARD_H

#include <qpa/qplatformclipboard.h>

class QUbuntuBaseClipboard : public QPlatformClipboard {
 public:
  QUbuntuBaseClipboard();
  ~QUbuntuBaseClipboard();

  // QPlatformClipboard methods.
  virtual QMimeData* mimeData(QClipboard::Mode mode = QClipboard::Clipboard);
  virtual void setMimeData(QMimeData* data, QClipboard::Mode mode = QClipboard::Clipboard);
  virtual bool supportsMode(QClipboard::Mode mode) const { return mode == QClipboard::Clipboard; }
  virtual bool ownsMode(QClipboard::Mode mode) const { Q_UNUSED(mode); return false; }
};

#endif  // QUBUNTUBASECLIPBOARD_H
