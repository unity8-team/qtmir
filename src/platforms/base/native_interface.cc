// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "native_interface.h"
#include "screen.h"
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
  }
};

Q_GLOBAL_STATIC(QUbuntuBaseResourceMap, ubuntuResourceMap)

QUbuntuBaseNativeInterface::QUbuntuBaseNativeInterface()
    : genericEventFilterType_(QByteArrayLiteral("Event")) {
  DLOG("QUbuntuBaseNativeInterface::QUbuntuBaseNativeInterface (this=%p)", this);
}

QUbuntuBaseNativeInterface::~QUbuntuBaseNativeInterface() {
  DLOG("QUbuntuBaseNativeInterface::~QUbuntuBaseNativeInterface");
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
  } else {
    return NULL;
  }
}
