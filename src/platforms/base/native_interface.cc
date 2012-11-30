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

class QHybrisBaseResourceMap : public QMap<QByteArray, QHybrisBaseNativeInterface::ResourceType> {
 public:
  QHybrisBaseResourceMap()
      : QMap<QByteArray, QHybrisBaseNativeInterface::ResourceType>() {
    insert("egldisplay", QHybrisBaseNativeInterface::EglDisplay);
    insert("eglcontext", QHybrisBaseNativeInterface::EglContext);
  }
};

Q_GLOBAL_STATIC(QHybrisBaseResourceMap, hybrisResourceMap)

QHybrisBaseNativeInterface::QHybrisBaseNativeInterface()
    : genericEventFilterType_(QByteArrayLiteral("Event")) {
  DLOG("QHybrisBaseNativeInterface::QHybrisBaseNativeInterface (this=%p)", this);
}

QHybrisBaseNativeInterface::~QHybrisBaseNativeInterface() {
  DLOG("QHybrisBaseNativeInterface::~QHybrisBaseNativeInterface");
}

void* QHybrisBaseNativeInterface::nativeResourceForContext(
    const QByteArray& resourceString, QOpenGLContext* context) {
  DLOG("QHybrisBaseNativeInterface::nativeResourceForContext (this=%p, resourceString=%s, "
       "context=%p)", this, resourceString.constData(), context);
  if (!context)
    return NULL;
  const QByteArray kLowerCaseResource = resourceString.toLower();
  if (!hybrisResourceMap()->contains(kLowerCaseResource))
    return NULL;
  const ResourceType kResourceType = hybrisResourceMap()->value(kLowerCaseResource);
  if (kResourceType == QHybrisBaseNativeInterface::EglContext)
    return static_cast<QHybrisBaseContext*>(context->handle())->eglContext();
  else
    return NULL;
}

void* QHybrisBaseNativeInterface::nativeResourceForWindow(
    const QByteArray& resourceString, QWindow* window) {
  DLOG("QHybrisBaseNativeInterface::nativeResourceForWindow (this=%p, resourceString=%s, "
       "window=%p)", this, resourceString.constData(), window);
  const QByteArray kLowerCaseResource = resourceString.toLower();
  if (!hybrisResourceMap()->contains(kLowerCaseResource))
    return NULL;
  const ResourceType kResourceType = hybrisResourceMap()->value(kLowerCaseResource);
  if (kResourceType == QHybrisBaseNativeInterface::EglDisplay) {
    if (window) {
      return static_cast<QHybrisBaseScreen*>(window->screen()->handle())->eglDisplay();
    } else {
      return static_cast<QHybrisBaseScreen*>(
          QGuiApplication::primaryScreen()->handle())->eglDisplay();
    }
  } else {
    return NULL;
  }
}
