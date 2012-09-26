// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISWINDOW_H
#define QHYBRISWINDOW_H

#include "qhybrisintegration.h"
#include "qhybrisscreen.h"
#include <qpa/qplatformwindow.h>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class QHybrisWindow : public QPlatformWindow {
 public:
  QHybrisWindow(QWindow* w);

  void setGeometry(const QRect&);
  WId winId() const;

 private:
  WId m_winid;
};

QT_END_NAMESPACE

#endif  // QHYBRISWINDOW_H
