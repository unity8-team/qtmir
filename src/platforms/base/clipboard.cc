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

#include "clipboard.h"
#include "logging.h"

QUbuntuBaseClipboard::QUbuntuBaseClipboard() {
  DLOG("QUbuntuBaseClipboard::QUbuntuBaseClipboard (this=%p)", this);
}

QUbuntuBaseClipboard::~QUbuntuBaseClipboard() {
  DLOG("QUbuntuBaseClipboard::~QUbuntuBaseClipboard");
}

QMimeData* QUbuntuBaseClipboard::mimeData(QClipboard::Mode mode) {
  DLOG("QUbuntuBaseClipboard::mimeData (this=%p, mode=%d)", this, static_cast<int>(mode));
  return QPlatformClipboard::mimeData(mode);
}

void QUbuntuBaseClipboard::setMimeData(QMimeData* data, QClipboard::Mode mode) {
  DLOG("QUbuntuBaseClipboard::setMimeData (this=%p, data=%p, mode=%d)", this, data,
       static_cast<int>(mode));
  QPlatformClipboard::setMimeData(data, mode);
}
