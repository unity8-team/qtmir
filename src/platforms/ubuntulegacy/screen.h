// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QUBUNTULEGACYSCREEN_H
#define QUBUNTULEGACYSCREEN_H

#include "base/screen.h"

struct SfClient;

class QUbuntuLegacyScreen : public QUbuntuBaseScreen {
 public:
  QUbuntuLegacyScreen();
  ~QUbuntuLegacyScreen();

  // QPlatformScreen methods.
  QRect geometry() const { return geometry_; }

  // New methods.
  SfClient* sfClient() const { return sfClient_; }

 private:
  QRect geometry_;
  SfClient* sfClient_;
};

#endif  // QUBUNTULEGACYSCREEN_H
