// Copyright © 2012 Canonical Ltd
// FIXME Add copyright notice here.

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

