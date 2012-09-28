// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "qhybrisscreen.h"
#include "qhybriswindow.h"
#include <QtPlatformSupport/private/qeglconvenience_p.h>
#include <QtPlatformSupport/private/qeglplatformcontext_p.h>
#include <surface_flinger/surface_flinger_compatibility_layer.h>

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

QHybrisScreen::QHybrisScreen()
    : m_depth(32)
    , m_format(QImage::Format_Invalid)
    , m_sfClient(0)
    , m_sfSurface(0)
    , m_platformContext(0)
    , m_surface(0) {
#ifdef QHYBRIS_DEBUG
  qWarning("creating QEglScreen %p\n", this);
#endif

  // We don't need EGL support from the compat lib.
  m_sfClient = sf_client_create_full(false);
  if (m_sfClient == NULL) {
    qWarning("Could not create SF compat client\n");
    qFatal("SF compat error");
  }

  if (!eglBindAPI(EGL_OPENGL_ES_API)) {
    qWarning("Could not bind GL_ES API\n");
    qFatal("EGL error");
  }

  m_dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (m_dpy == EGL_NO_DISPLAY) {
    qWarning("Could not open egl display\n");
    qFatal("EGL error");
  }
  qWarning("Opened display %p\n", m_dpy);

  EGLint major, minor;
  if (!eglInitialize(m_dpy, &major, &minor)) {
    qWarning("Could not initialize egl display\n");
    qFatal("EGL error");
  }
  qWarning("Initialized display (EGL %d.%d)\n", major, minor);

  int swapInterval = 1;
  QByteArray swapIntervalString = qgetenv("QT_QPA_HYBRIS_SWAPINTERVAL");
  if (!swapIntervalString.isEmpty()) {
    bool ok;
    swapInterval = swapIntervalString.toInt(&ok);
    if (!ok)
      swapInterval = 1;
  }
  eglSwapInterval(m_dpy, swapInterval);

#ifdef QHYBRIS_DEBUG
  qWarning("created QEglScreen %p\n", this);
#endif
}

QHybrisScreen::~QHybrisScreen() {
  if (m_surface)
    eglDestroySurface(m_dpy, m_surface);
  eglTerminate(m_dpy);
  delete m_sfSurface;
  delete m_sfClient;
}

void QHybrisScreen::createAndSetPlatformContext() const {
  const_cast<QHybrisScreen*>(this)->createAndSetPlatformContext();
}

void QHybrisScreen::createAndSetPlatformContext() {
  QSurfaceFormat platformFormat;
  int w, h;

  // FIXME(loicm) SF compat only creates 32-bit ARGB surfaces for now. For
  //     performance reasons, it might be useful to add support for lower
  //     framebuffer color depths.
  platformFormat.setDepthBufferSize(24);
  platformFormat.setStencilBufferSize(8);
  platformFormat.setRedBufferSize(8);
  platformFormat.setGreenBufferSize(8);
  platformFormat.setBlueBufferSize(8);
  m_depth = 32;
  m_format = QImage::Format_RGB32;

  if (!qEnvironmentVariableIsEmpty("QT_QPA_HYBRIS_MULTISAMPLE"))
    platformFormat.setSamples(4);

  EGLConfig config = q_configFromGLFormat(m_dpy, platformFormat);
#ifdef QHYBRIS_DEBUG
  q_printEglConfig(m_dpy, config);
#endif

  w = sf_get_display_width(SURFACE_FLINGER_DEFAULT_DISPLAY_ID);
  h = sf_get_display_height(SURFACE_FLINGER_DEFAULT_DISPLAY_ID);
  SfSurfaceCreationParameters parameters = { 0, 0, w, h, -1, INT_MAX, 1.0f, false, "qthybris" };
  m_sfSurface = sf_surface_create(m_sfClient, &parameters);
  EGLNativeWindowType nativeWindow = sf_surface_get_egl_native_window(m_sfSurface);
  m_surface = eglCreateWindowSurface(m_dpy, config, nativeWindow, NULL);
  if (m_surface == EGL_NO_SURFACE) {
    qWarning("Could not create the egl surface: error = 0x%x\n", eglGetError());
    eglTerminate(m_dpy);
    qFatal("EGL error");
  }
  qWarning("Created surface %dx%d\n", w, h);

  QEGLPlatformContext* platformContext = new QHybrisContext(platformFormat, 0, m_dpy);
  m_platformContext = platformContext;

  eglQuerySurface(m_dpy, m_surface, EGL_WIDTH, &w);
  eglQuerySurface(m_dpy, m_surface, EGL_HEIGHT, &h);
  m_geometry = QRect(0, 0, w, h);
  qWarning("EGL surface size %dx%d\n", w, h);
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
