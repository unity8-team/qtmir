// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef APPLICATION_IMAGE_H
#define APPLICATION_IMAGE_H

#include <QtQuick/QQuickPaintedItem>
#include <QtGui/QImage>

class Application;

class ApplicationImage : public QQuickPaintedItem {
  Q_OBJECT
  Q_PROPERTY(Application* source READ source WRITE setSource NOTIFY sourceChanged)

 public:
  explicit ApplicationImage(QQuickPaintedItem* parent = 0);
  ~ApplicationImage();

  // QObject methods.
  void customEvent(QEvent* event);

  // QQuickPaintedItem methods.
  void paint(QPainter* painter);

  // ApplicationImage methods.
  Application* source() const { return source_; }
  void setSource(Application* source);
  Q_INVOKABLE void scheduleUpdate();

 Q_SIGNALS:
  void sourceChanged();

 private Q_SLOTS:
  void onSourceDestroyed();

 private:
  QImage image_;
  Application* source_;
};

#endif  // APPLICATION_IMAGE_H
