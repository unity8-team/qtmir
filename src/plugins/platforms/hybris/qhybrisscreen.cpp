// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "qhybrisscreen.h"
#include "qhybriswindow.h"
#include "qhybrislogging.h"
#include <QtPlatformSupport/private/qeglconvenience_p.h>
#include <QtPlatformSupport/private/qeglplatformcontext_p.h>
#include <surface_flinger/surface_flinger_compatibility_layer.h>

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
  // Initialize and surface flinger compatibility library and EGL.
  m_sfClient = sf_client_create_full(false);
  ASSERT(m_sfClient != NULL);
  bool eglBindApiResult = eglBindAPI(EGL_OPENGL_ES_API);
  ASSERT(eglBindApiResult == true);
  m_dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  ASSERT(m_dpy != EGL_NO_DISPLAY);
  EGLint major, minor;
  bool eglInitializeResult = eglInitialize(m_dpy, &major, &minor);
  ASSERT(eglInitializeResult == true);
  DLOG("Initialized EGL version %d.%d", major, minor);

  // Set swap interval.
  int swapInterval = 1;
  QByteArray swapIntervalString = qgetenv("QT_QPA_HYBRIS_SWAPINTERVAL");
  if (!swapIntervalString.isEmpty()) {
    bool ok;
    swapInterval = swapIntervalString.toInt(&ok);
    if (!ok)
      swapInterval = 1;
  }
  DLOG("setting swap interval to %d", swapInterval);
  eglSwapInterval(m_dpy, swapInterval);

  DLOG("created QEglScreen (this=%p)", this);
}

QHybrisScreen::~QHybrisScreen() {
  if (m_surface)
    eglDestroySurface(m_dpy, m_surface);
  eglTerminate(m_dpy);
  // FIXME(loicm) These are invalid since the structs are forward declarated, we need a way to clean
  //     these handles correctly.
  // delete m_sfSurface;
  // delete m_sfClient;
  DLOG("deleted QHybrisScreen (this=%p)", this);
}

void QHybrisScreen::createAndSetPlatformContext() const {
  DLOG("QHybrisScreen::createAndSetPlatformContext const (this=%p)", this);
  const_cast<QHybrisScreen*>(this)->createAndSetPlatformContext();
}

void QHybrisScreen::createAndSetPlatformContext() {
  DLOG("QHybrisScreen::createAndSetPlatformContext (this=%p)", this);
  QSurfaceFormat platformFormat;
  int w, h;

  // FIXME(loicm) SF compat only creates 32-bit ARGB surfaces for now. For performance reasons, it
  //     might be useful to add support for lower framebuffer color depths.
  platformFormat.setDepthBufferSize(24);
  platformFormat.setStencilBufferSize(8);
  platformFormat.setRedBufferSize(8);
  platformFormat.setGreenBufferSize(8);
  platformFormat.setBlueBufferSize(8);
  m_depth = 32;
  m_format = QImage::Format_RGB32;
  if (!qEnvironmentVariableIsEmpty("QT_QPA_HYBRIS_MULTISAMPLE")) {
    platformFormat.setSamples(4);
    DLOG("setting MSAA to 4 samples");
  }

  EGLConfig config = q_configFromGLFormat(m_dpy, platformFormat);
  // q_printEglConfig(m_dpy, config);
  w = sf_get_display_width(SURFACE_FLINGER_DEFAULT_DISPLAY_ID);
  h = sf_get_display_height(SURFACE_FLINGER_DEFAULT_DISPLAY_ID);
  SfSurfaceCreationParameters parameters = { 0, 0, w, h, -1, INT_MAX, 1.0f, false, "qthybris" };
  m_sfSurface = sf_surface_create(m_sfClient, &parameters);
  ASSERT(m_sfSurface != NULL);
  EGLNativeWindowType nativeWindow = sf_surface_get_egl_native_window(m_sfSurface);
  m_surface = eglCreateWindowSurface(m_dpy, config, nativeWindow, NULL);
  ASSERT(m_surface != EGL_NO_SURFACE);
  DLOG("created EGL surface %p", m_surface);

  m_platformContext = new QHybrisContext(platformFormat, 0, m_dpy);

  eglQuerySurface(m_dpy, m_surface, EGL_WIDTH, &w);
  eglQuerySurface(m_dpy, m_surface, EGL_HEIGHT, &h);
  m_geometry = QRect(0, 0, w, h);
}

QRect QHybrisScreen::geometry() const {
  DLOG("QHybrisScreen::geometry (this=%p)", this);
  if (m_geometry.isNull())
    createAndSetPlatformContext();
  return m_geometry;
}

int QHybrisScreen::depth() const {
  DLOG("QHybrisScreen::depth (this=%p)", this);
  return m_depth;
}

QImage::Format QHybrisScreen::format() const {
  DLOG("QHybrisScreen::format (this=%p)", this);
  if (m_format == QImage::Format_Invalid)
    createAndSetPlatformContext();
  return m_format;
}

QPlatformOpenGLContext *QHybrisScreen::platformContext() const {
  DLOG("QHybrisScreen::platformContext (this=%p)", this);
  if (!m_platformContext) {
    QHybrisScreen* screen = const_cast<QHybrisScreen*>(this);
    screen->createAndSetPlatformContext();
  }
  return m_platformContext;
}
