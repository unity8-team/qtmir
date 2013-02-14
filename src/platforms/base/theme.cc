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

#include "theme.h"
#include "logging.h"
#include <QtCore/QVariant>

const char *QUbuntuTheme::name = "ubuntu";

QUbuntuTheme::QUbuntuTheme() {
  DLOG("QUbuntuTheme::QUbuntuTheme()");
}

QUbuntuTheme::~QUbuntuTheme() {
  DLOG("QUbuntuTheme::~QUbuntuTheme");
}

QVariant QUbuntuTheme::themeHint(ThemeHint hint) const {
  DLOG("QUbuntuTheme::themehint (this=%p, hint=%d)", this, hint);
  if (hint == QPlatformTheme::SystemIconThemeName) {
    QByteArray iconTheme = qgetenv("QTUBUNTU_ICON_THEME");
    if (iconTheme.isEmpty()) {
      return QVariant(QStringLiteral("ubuntu-mobile"));
    } else {
      return QVariant(QString(iconTheme));
    }
  } else {
    return QGenericUnixTheme::themeHint(hint);
  }
}

