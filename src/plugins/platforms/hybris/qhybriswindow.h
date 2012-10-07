// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISWINDOW_H
#define QHYBRISWINDOW_H

#include "qhybrisintegration.h"
#include "qhybrisscreen.h"
#include <qpa/qplatformwindow.h>
#include <QtWidgets/QWidget>

class QHybrisWindow : public QPlatformWindow {
 public:
  QHybrisWindow(QWindow* w);
  ~QHybrisWindow();

  void setGeometry(const QRect&);
  WId winId() const { return winId_; }

  // FIXME(loicm) Add opacity and stacking support.
  // void setOpacity(qreal level);
  // void raise();
  // void lower();

 private:
  WId winId_;
};

#endif  // QHYBRISWINDOW_H
