// This file is part of QtUbuntu, a set of Qt components for Ubuntu.
// Copyright © 2013 Canonical Ltd.
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 3, as published by the
// Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
// SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.

#include <QObject>

class Scene : public QObject {
  Q_OBJECT

 public:
  Scene(QObject* parent = 0);
  void getStats(int* frames, double* min, double* max, double* average);

 private slots:
  void beforeRendering();
  void afterRendering();

 private:
  struct timespec t1_;
  struct timespec t2_;
  qint64 frames_;
  double sum_;
  double min_;
  double max_;
};
