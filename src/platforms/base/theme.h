// Copyright © 2012 Canonical Ltd
// FIXME Add copyright notice here.

#ifndef QHYBRISTHEME_H
#define QHYBRISTHEME_H

#include <QtPlatformSupport/private/qgenericunixthemes_p.h>

class QHybrisTheme : public QGenericUnixTheme {
 public:
  QHybrisTheme();

  static const char* name();
  virtual QVariant themeHint(ThemeHint hint) const;
};

#endif  //QHYBRISBASECONTEXT_H
