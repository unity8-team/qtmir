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

#ifndef QUBUNTUCLIPBOARD_H
#define QUBUNTUCLIPBOARD_H

#include "base/clipboard.h"

class QUbuntuClipboard : public QUbuntuBaseClipboard {
 public:
  QUbuntuClipboard();
  ~QUbuntuClipboard();

  // QUbuntuBaseClipboard methods.
  QMimeData* mimeData(QClipboard::Mode mode = QClipboard::Clipboard);
  void setMimeData(QMimeData* data, QClipboard::Mode mode = QClipboard::Clipboard);

 private:
  QMimeData* mimeData_;
};

#endif  // QUBUNTUCLIPBOARD_H
