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

#ifndef APPLICATION_IMAGE_H
#define APPLICATION_IMAGE_H

#include <QtQuick/QQuickPaintedItem>
#include <QtGui/QImage>

class Application;

class ApplicationImage : public QQuickPaintedItem {
  Q_OBJECT
  Q_ENUMS(FillMode)
  Q_PROPERTY(Application* source READ source WRITE setSource NOTIFY sourceChanged)
  Q_PROPERTY(FillMode fillMode READ fillMode WRITE setFillMode NOTIFY fillModeChanged)
  Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)

 public:
  explicit ApplicationImage(QQuickPaintedItem* parent = 0);
  ~ApplicationImage();

  enum FillMode { Stretch, PreserveAspectCrop };

  // QObject methods.
  void customEvent(QEvent* event);

  // QQuickPaintedItem methods.
  void paint(QPainter* painter);

  // ApplicationImage methods.
  Application* source() const { return source_; }
  void setSource(Application* source);
  FillMode fillMode() const  { return fillMode_; }
  void setFillMode(FillMode);
  bool ready() const { return ready_; }
  Q_INVOKABLE void scheduleUpdate();
  Q_INVOKABLE void updateFromCache();

 Q_SIGNALS:
  void sourceChanged();
  void fillModeChanged();
  void readyChanged();

 private Q_SLOTS:
  void onSourceDestroyed();

 private:
  QImage image_;
  Application* source_;
  FillMode fillMode_;
  bool ready_;
  QRect sourceRect_;
  // cache of application screenshots keeping pairs of image and source rectangle
  // for applications that are alive
  static QHash<Application*, QPair<QImage, QRect> > imageCache_;
};

#endif  // APPLICATION_IMAGE_H
