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

#ifndef QUBUNTUSCREEN_H
#define QUBUNTUSCREEN_H

#include "base/screen.h"
#include <ubuntu/application/ui/options.h>

#include <QObject>

class QOrientationSensor;

class QUbuntuScreen : public QObject, public QUbuntuBaseScreen {
  Q_OBJECT
 public:
  QUbuntuScreen(UApplicationOptions *options);
  ~QUbuntuScreen();

  // QPlatformScreen methods.
  QRect geometry() const { return geometry_; }
  QRect availableGeometry() const { return availableGeometry_; }

  Qt::ScreenOrientation nativeOrientation() const { return nativeOrientation_; }
  Qt::ScreenOrientation orientation() const { return currentOrientation_; }
  int gridUnitToPixel(int value) const;
  int densityPixelToPixel(int value) const;

  // QObject methods.
  void customEvent(QEvent* event);

 public Q_SLOTS:
    void onOrientationReadingChanged();

 private:
  QRect geometry_;
  QRect availableGeometry_;
  int gridUnit_;
  float densityPixelRatio_;
  Qt::ScreenOrientation nativeOrientation_;
  Qt::ScreenOrientation currentOrientation_;
  QOrientationSensor* orientationSensor_;
};

#endif  // QUBUNTUSCREEN_H
