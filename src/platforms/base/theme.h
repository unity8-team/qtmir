// Copyright © 2012 Canonical Ltd
// FIXME Add copyright notice here.

#ifndef QUBUNTUTHEME_H
#define QUBUNTUTHEME_H

#include <QtPlatformSupport/private/qgenericunixthemes_p.h>

class QUbuntuTheme : public QGenericUnixTheme {
 public:
  static const char* name;
  QUbuntuTheme();
  ~QUbuntuTheme();

  virtual QVariant themeHint(ThemeHint hint) const;
};

#endif  //QUBUNTUBASECONTEXT_H
