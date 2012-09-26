// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "qhybrisscreen.h"
#include "qhybriswindow.h"
#include <QtPlatformSupport/private/qeglconvenience_p.h>
#include <QtPlatformSupport/private/qeglplatformcontext_p.h>

QT_BEGIN_NAMESPACE

class QHybrisContext : public QEGLPlatformContext {
 public:
  QHybrisContext(const QSurfaceFormat& format, QPlatformOpenGLContext* share, EGLDisplay display,
                 EGLenum eglApi = EGL_OPENGL_ES_API)
      : QEGLPlatformContext(format, share, display, eglApi) {
  }

  EGLSurface eglSurfaceForPlatformSurface(QPlatformSurface* surface) {
    QHybrisWindow* window = static_cast<QHybrisWindow*>(surface);
    QHybrisScreen* screen = static_cast<QHybrisScreen*>(window->screen());
    return screen->surface();
  }
};

QHybrisScreen::QHybrisScreen(EGLNativeDisplayType display)
    : m_depth(32)
    , m_format(QImage::Format_Invalid)
    , m_platformContext(0)
    , m_surface(0) {
#ifdef QHYBRIS_DEBUG
  qWarning("QEglScreen %p\n", this);
#endif
  EGLint major, minor;

  if (!eglBindAPI(EGL_OPENGL_ES_API)) {
    qWarning("Could not bind GL_ES API\n");
    qFatal("EGL error");
  }

  m_dpy = eglGetDisplay(display);
  if (m_dpy == EGL_NO_DISPLAY) {
    qWarning("Could not open egl display\n");
    qFatal("EGL error");
  }
  qWarning("Opened display %p\n", m_dpy);

  if (!eglInitialize(m_dpy, &major, &minor)) {
    qWarning("Could not initialize egl display\n");
    qFatal("EGL error");
  }

  qWarning("Initialized display %d %d\n", major, minor);

  int swapInterval = 1;
  QByteArray swapIntervalString = qgetenv("QT_QPA_EGLFS_SWAPINTERVAL");
  if (!swapIntervalString.isEmpty()) {
    bool ok;
    swapInterval = swapIntervalString.toInt(&ok);
    if (!ok)
      swapInterval = 1;
  }
  eglSwapInterval(m_dpy, swapInterval);
}

QHybrisScreen::~QHybrisScreen() {
  if (m_surface)
    eglDestroySurface(m_dpy, m_surface);
  eglTerminate(m_dpy);
}

void QHybrisScreen::createAndSetPlatformContext() const {
  const_cast<QHybrisScreen*>(this)->createAndSetPlatformContext();
}

void QHybrisScreen::createAndSetPlatformContext() {
  QSurfaceFormat platformFormat;

  QByteArray depthString = qgetenv("QT_QPA_EGLFS_DEPTH");
  if (depthString.toInt() == 16) {
    platformFormat.setDepthBufferSize(16);
    platformFormat.setRedBufferSize(5);
    platformFormat.setGreenBufferSize(6);
    platformFormat.setBlueBufferSize(5);
    m_depth = 16;
    m_format = QImage::Format_RGB16;
  } else {
    platformFormat.setDepthBufferSize(24);
    platformFormat.setStencilBufferSize(8);
    platformFormat.setRedBufferSize(8);
    platformFormat.setGreenBufferSize(8);
    platformFormat.setBlueBufferSize(8);
    m_depth = 32;
    m_format = QImage::Format_RGB32;
  }

  if (!qEnvironmentVariableIsEmpty("QT_QPA_EGLFS_MULTISAMPLE"))
    platformFormat.setSamples(4);

  EGLConfig config = q_configFromGLFormat(m_dpy, platformFormat);

  EGLNativeWindowType eglWindow = 0;

#ifdef QHYBRIS_DEBUG
  q_printEglConfig(m_dpy, config);
#endif

  m_surface = eglCreateWindowSurface(m_dpy, config, eglWindow, NULL);
  if (m_surface == EGL_NO_SURFACE) {
    qWarning("Could not create the egl surface: error = 0x%x\n", eglGetError());
    eglTerminate(m_dpy);
    qFatal("EGL error");
  }

  QEGLPlatformContext* platformContext = new QHybrisContext(platformFormat, 0, m_dpy);
  m_platformContext = platformContext;

  EGLint w, h;
  eglQuerySurface(m_dpy, m_surface, EGL_WIDTH, &w);
  eglQuerySurface(m_dpy, m_surface, EGL_HEIGHT, &h);

  m_geometry = QRect(0,0,w,h);
}

QRect QHybrisScreen::geometry() const {
  if (m_geometry.isNull()) {
    createAndSetPlatformContext();
  }
  return m_geometry;
}

int QHybrisScreen::depth() const {
  return m_depth;
}

QImage::Format QHybrisScreen::format() const {
  if (m_format == QImage::Format_Invalid)
    createAndSetPlatformContext();
  return m_format;
}

QPlatformOpenGLContext *QHybrisScreen::platformContext() const {
  if (!m_platformContext) {
    QHybrisScreen* screen = const_cast<QHybrisScreen*>(this);
    screen->createAndSetPlatformContext();
  }
  return m_platformContext;
}

QT_END_NAMESPACE
