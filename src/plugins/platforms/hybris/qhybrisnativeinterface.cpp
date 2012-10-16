// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "qhybrisnativeinterface.h"
#include "qhybrisscreen.h"
#include "qhybriscontext.h"
#include "qhybrislogging.h"
#include <private/qguiapplication_p.h>
#include <QtGui/qopenglcontext.h>
#include <QtGui/qscreen.h>
#include <QtCore/QMap>

class QHybrisResourceMap : public QMap<QByteArray, QHybrisNativeInterface::ResourceType> {
 public:
  QHybrisResourceMap()
      : QMap<QByteArray, QHybrisNativeInterface::ResourceType>() {
    insert("sfclient", QHybrisNativeInterface::SfClient);
    insert("egldisplay", QHybrisNativeInterface::EglDisplay);
    insert("eglcontext", QHybrisNativeInterface::EglContext);
  }
};

Q_GLOBAL_STATIC(QHybrisResourceMap, hybrisResourceMap)

QHybrisNativeInterface::QHybrisNativeInterface()
    : genericEventFilterType_(QByteArrayLiteral("Event")) {
  DLOG("QHybrisNativeInterface::QHybrisNativeInterface (this=%p)", this);
}

QHybrisNativeInterface::~QHybrisNativeInterface() {
  DLOG("QHybrisNativeInterface::~QHybrisNativeInterface");
}

void* QHybrisNativeInterface::nativeResourceForContext(
    const QByteArray& resourceString, QOpenGLContext* context) {
  DLOG("QHybrisNativeInterface::nativeResourceForContext (this=%p, resourceString=%s, "
       "context=%p)", this, resourceString, context);
  if (!context)
    return NULL;
  const QByteArray kLowerCaseResource = resourceString.toLower();
  if (!hybrisResourceMap()->contains(kLowerCaseResource))
    return NULL;
  const ResourceType kResourceType = hybrisResourceMap()->value(kLowerCaseResource);
  if (kResourceType == QHybrisNativeInterface::EglContext)
    return static_cast<QHybrisContext*>(context->handle())->eglContext();
  else
    return NULL;
}

void* QHybrisNativeInterface::nativeResourceForWindow(
    const QByteArray& resourceString, QWindow* window) {
  DLOG("QHybrisNativeInterface::nativeResourceForWindow (this=%p, resourceString=%s, "
       "window=%p)", this, resourceString, window);
  const QByteArray kLowerCaseResource = resourceString.toLower();
  if (!hybrisResourceMap()->contains(kLowerCaseResource))
    return NULL;
  const ResourceType kResourceType = hybrisResourceMap()->value(kLowerCaseResource);
  if (kResourceType == QHybrisNativeInterface::SfClient) {
    if (window) {
      return static_cast<QHybrisScreen*>(window->screen()->handle())->sfClient();
    } else {
      return static_cast<QHybrisScreen*>(QGuiApplication::primaryScreen()->handle())->sfClient();
    }
  } else if (kResourceType == QHybrisNativeInterface::EglDisplay) {
    if (window) {
      return static_cast<QHybrisScreen*>(window->screen()->handle())->eglDisplay();
    } else {
      return static_cast<QHybrisScreen*>(QGuiApplication::primaryScreen()->handle())->eglDisplay();
    }
  } else {
    return NULL;
  }
}
