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

#ifndef QUBUNTUINTEGRATION_H
#define QUBUNTUINTEGRATION_H

#include "base/integration.h"
#include "screen.h"
#include <ubuntu/application/instance.h>
#include <ubuntu/application/ui/session.h>

class QUbuntuInputAdaptorFactory;
class QUbuntuInput;

class QUbuntuIntegration : public QUbuntuBaseIntegration {
 public:
  QUbuntuIntegration(QUbuntuInputAdaptorFactory* input_factory);
  ~QUbuntuIntegration();

  // QPlatformIntegration methods.
  QPlatformWindow* createPlatformWindow(QWindow* window) const;
  QPlatformWindow* createPlatformWindow(QWindow* window);
  QPlatformInputContext* inputContext() const { return inputContext_; }
  QPlatformClipboard* clipboard() const { return clipboard_; }
  QUbuntuScreen* screen() const { return screen_; }

 private:
  int argc_;
  char** argv_;
  QUbuntuScreen* screen_;
  QUbuntuInput* input_;
  QPlatformInputContext* inputContext_;
  QPlatformClipboard* clipboard_;
  bool isShell_;

  UApplicationOptions* options_;
  UApplicationDescription* desc_;
  UApplicationInstance* instance_;
  UAUiSessionProperties* props_;
  UAUiSession* session_;
};

#endif  // QUBUNTUINTEGRATION_H
