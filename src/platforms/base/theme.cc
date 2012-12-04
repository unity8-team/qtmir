// Copyright © 2012 Canonical Ltd
// FIXME Add copyright notice here.

#include "theme.h"
#include "logging.h"
#include <QtCore/QVariant>

QHybrisTheme::QHybrisTheme() {
}

const char* QHybrisTheme::name() {
  return "hybris";
}

QVariant QHybrisTheme::themeHint(ThemeHint hint) const {
  DLOG("QHybrisTheme::themehint (this=%p, hint=%d)", this, hint);
  if (hint == QPlatformTheme::SystemIconThemeName) {
    QByteArray iconTheme = qgetenv("QHYBRIS_ICON_THEME");
    if (iconTheme.isEmpty()) {
      return QVariant(QStringLiteral("ubuntu-mobile"));
    } else {
      return QVariant(QString(iconTheme));
    }
  } else {
    return QGenericUnixTheme::themeHint(hint);
  }
}

