// Copyright © 2012 Canonical Ltd
// FIXME Add copyright notice here.

#include "theme.h"
#include "logging.h"
#include <QtCore/QVariant>

const char *QHybrisTheme::name = "hybris";

QHybrisTheme::QHybrisTheme() {
}

QVariant QHybrisTheme::themeHint(ThemeHint hint) const {
  DLOG("QHybrisTheme::themehint (this=%p, hint=%d)", this, hint);
  if (hint == QPlatformTheme::SystemIconThemeName)
    return QVariant(QString(QStringLiteral("ubuntu-mono-dark")));
  else
    return QGenericUnixTheme::themeHint(hint);
}

