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

// FIXME(loicm) "nativeorientation" should be implemented in the Ubuntu plugin
//     and not include headers from it, base isn't meant to know about plugins.
// FIXME(loicm) Why return a pointer to the screen orientation enum? The enum
//     can just fit in the returned pointer.

#include "native_interface.h"
#include "screen.h"
#include "ubuntu/ubuntucommon/screen.h"
#include "context.h"
#include "logging.h"
#include <private/qguiapplication_p.h>
#include <QtGui/qopenglcontext.h>
#include <QtGui/qscreen.h>
#include <QtCore/QMap>

class QUbuntuBaseResourceMap : public QMap<QByteArray, QUbuntuBaseNativeInterface::ResourceType> {
 public:
  QUbuntuBaseResourceMap()
      : QMap<QByteArray, QUbuntuBaseNativeInterface::ResourceType>() {
    insert("egldisplay", QUbuntuBaseNativeInterface::EglDisplay);
    insert("eglcontext", QUbuntuBaseNativeInterface::EglContext);
    insert("nativeorientation", QUbuntuBaseNativeInterface::NativeOrientation);
    insert("display", QUbuntuBaseNativeInterface::Display);
  }
};

Q_GLOBAL_STATIC(QUbuntuBaseResourceMap, ubuntuResourceMap)

QUbuntuBaseNativeInterface::QUbuntuBaseNativeInterface()
    : genericEventFilterType_(QByteArrayLiteral("Event"))
    , nativeOrientation_(NULL) {
  DLOG("QUbuntuBaseNativeInterface::QUbuntuBaseNativeInterface (this=%p)", this);
}

QUbuntuBaseNativeInterface::~QUbuntuBaseNativeInterface() {
  DLOG("QUbuntuBaseNativeInterface::~QUbuntuBaseNativeInterface");
  if (nativeOrientation_)
    delete nativeOrientation_;
}

void* QUbuntuBaseNativeInterface::nativeResourceForContext(
    const QByteArray& resourceString, QOpenGLContext* context) {
  DLOG("QUbuntuBaseNativeInterface::nativeResourceForContext (this=%p, resourceString=%s, "
       "context=%p)", this, resourceString.constData(), context);
  if (!context)
    return NULL;
  const QByteArray kLowerCaseResource = resourceString.toLower();
  if (!ubuntuResourceMap()->contains(kLowerCaseResource))
    return NULL;
  const ResourceType kResourceType = ubuntuResourceMap()->value(kLowerCaseResource);
  if (kResourceType == QUbuntuBaseNativeInterface::EglContext)
    return static_cast<QUbuntuBaseContext*>(context->handle())->eglContext();
  else
    return NULL;
}

void* QUbuntuBaseNativeInterface::nativeResourceForScreen(
    const QByteArray& resourceString, QScreen* screen) {
  DLOG("QUbuntuBaseNativeInterface::nativeResourceForScreen (this=%p, resourceString=%s, "
       "screen=%p)", this, resourceString.constData(), screen);
  const QByteArray kLowerCaseResource = resourceString.toLower();
  if (!ubuntuResourceMap()->contains(kLowerCaseResource))
    return NULL;
  const ResourceType kResourceType = ubuntuResourceMap()->value(kLowerCaseResource);
  if (kResourceType == QUbuntuBaseNativeInterface::Display) {
    if (!screen)
      screen = QGuiApplication::primaryScreen();
    return static_cast<QUbuntuBaseScreen*>(screen->handle())->nativeDisplay();
  } else
    return NULL;
}

void* QUbuntuBaseNativeInterface::nativeResourceForWindow(
    const QByteArray& resourceString, QWindow* window) {
  DLOG("QUbuntuBaseNativeInterface::nativeResourceForWindow (this=%p, resourceString=%s, "
       "window=%p)", this, resourceString.constData(), window);
  const QByteArray kLowerCaseResource = resourceString.toLower();
  if (!ubuntuResourceMap()->contains(kLowerCaseResource))
    return NULL;
  const ResourceType kResourceType = ubuntuResourceMap()->value(kLowerCaseResource);
  if (kResourceType == QUbuntuBaseNativeInterface::EglDisplay) {
    if (window) {
      return static_cast<QUbuntuBaseScreen*>(window->screen()->handle())->eglDisplay();
    } else {
      return static_cast<QUbuntuBaseScreen*>(
          QGuiApplication::primaryScreen()->handle())->eglDisplay();
    }
  } else if (kResourceType == QUbuntuBaseNativeInterface::NativeOrientation) {
    // Return the device's native screen orientation.
    if (window) {
      nativeOrientation_ = new Qt::ScreenOrientation(static_cast<QUbuntuScreen*>(window->screen()->handle())->nativeOrientation());
    } else {
      nativeOrientation_ = new Qt::ScreenOrientation(QGuiApplication::primaryScreen()->handle()->nativeOrientation());
    }
    return nativeOrientation_;
  } else {
    return NULL;
  }
}
