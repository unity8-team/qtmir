// Copyright © 2012 Canonical Ltd
// FIXME Add copyright notice here.

#include "theme.h"
#include "logging.h"
#include <QtCore/QVariant>

const char *QHybrisTheme::name = "hybris";

QHybrisTheme::QHybrisTheme() {
  DLOG("QHybrisTheme::QHybrisTheme()");
}

QHybrisTheme::~QHybrisTheme() {
  DLOG("QHybrisTheme::~QHybrisTheme");
}

QVariant QHybrisTheme::themeHint(ThemeHint hint) const {
  DLOG("QHybrisTheme::themehint (this=%p, hint=%d)", this, hint);
  if (hint == QPlatformTheme::SystemIconThemeName) {
    QByteArray iconTheme = qgetenv("QTHYBRIS_ICON_THEME");
    if (iconTheme.isEmpty()) {
      return QVariant(QStringLiteral("ubuntu-mobile"));
    } else {
      return QVariant(QString(iconTheme));
    }
  } else {
    return QGenericUnixTheme::themeHint(hint);
  }
}

