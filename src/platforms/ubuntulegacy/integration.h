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

#ifndef QUBUNTULEGACYINTEGRATION_H
#define QUBUNTULEGACYINTEGRATION_H

#include "base/integration.h"
#include "screen.h"

class QUbuntuLegacyInput;

class QUbuntuLegacyIntegration : public QObject, public QUbuntuBaseIntegration {
  Q_OBJECT

 public:
  QUbuntuLegacyIntegration();
  ~QUbuntuLegacyIntegration();

  // QPlatformIntegration methods.
  QPlatformWindow* createPlatformWindow(QWindow* window) const;
  QPlatformWindow* createPlatformWindow(QWindow* window);
  QPlatformInputContext* inputContext() const { return inputContext_; }
  QPlatformClipboard* clipboard() const { return clipboard_; }

  // New methods.
  // FIXME(loicm) Only one window can be created for now, remove that function when adding support
  //     for multiple windows.
  QPlatformWindow* platformWindow() const { return window_; }

 private slots:
  void initInput();

 private:
  QPlatformWindow* window_;
  QPlatformScreen* screen_;
  QUbuntuLegacyInput* input_;
  QPlatformInputContext* inputContext_;
  QPlatformClipboard* clipboard_;
};

#endif  // QUBUNTULEGACYINTEGRATION_H
