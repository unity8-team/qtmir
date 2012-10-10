// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "qhybrisscreen.h"
#include "qhybriscontext.h"
#include "qhybrislogging.h"
#include <qpa/qplatformwindow.h>
#include <QtPlatformSupport/private/qeglconvenience_p.h>
#include <surface_flinger/surface_flinger_compatibility_layer.h>

namespace {

#if defined(QHYBRIS_DEBUG)
void printEglConfig(EGLDisplay display, EGLConfig config) {
  static const struct { const EGLint attrib; const char* name; } kAttribs[] = {
    { EGL_BUFFER_SIZE, "EGL_BUFFER_SIZE" },
    { EGL_ALPHA_SIZE, "EGL_ALPHA_SIZE" },
    { EGL_BLUE_SIZE, "EGL_BLUE_SIZE" },
    { EGL_GREEN_SIZE, "EGL_GREEN_SIZE" },
    { EGL_RED_SIZE, "EGL_RED_SIZE" },
    { EGL_DEPTH_SIZE, "EGL_DEPTH_SIZE" },
    { EGL_STENCIL_SIZE, "EGL_STENCIL_SIZE" },
    { EGL_CONFIG_CAVEAT, "EGL_CONFIG_CAVEAT" },
    { EGL_CONFIG_ID, "EGL_CONFIG_ID" },
    { EGL_LEVEL, "EGL_LEVEL" },
    { EGL_MAX_PBUFFER_HEIGHT, "EGL_MAX_PBUFFER_HEIGHT" },
    { EGL_MAX_PBUFFER_PIXELS, "EGL_MAX_PBUFFER_PIXELS" },
    { EGL_MAX_PBUFFER_WIDTH, "EGL_MAX_PBUFFER_WIDTH" },
    { EGL_NATIVE_RENDERABLE, "EGL_NATIVE_RENDERABLE" },
    { EGL_NATIVE_VISUAL_ID, "EGL_NATIVE_VISUAL_ID" },
    { EGL_NATIVE_VISUAL_TYPE, "EGL_NATIVE_VISUAL_TYPE" },
    { EGL_SAMPLES, "EGL_SAMPLES" },
    { EGL_SAMPLE_BUFFERS, "EGL_SAMPLE_BUFFERS" },
    { EGL_SURFACE_TYPE, "EGL_SURFACE_TYPE" },
    { EGL_TRANSPARENT_TYPE, "EGL_TRANSPARENT_TYPE" },
    { EGL_TRANSPARENT_BLUE_VALUE, "EGL_TRANSPARENT_BLUE_VALUE" },
    { EGL_TRANSPARENT_GREEN_VALUE, "EGL_TRANSPARENT_GREEN_VALUE" },
    { EGL_TRANSPARENT_RED_VALUE, "EGL_TRANSPARENT_RED_VALUE" },
    { EGL_BIND_TO_TEXTURE_RGB, "EGL_BIND_TO_TEXTURE_RGB" },
    { EGL_BIND_TO_TEXTURE_RGBA, "EGL_BIND_TO_TEXTURE_RGBA" },
    { EGL_MIN_SWAP_INTERVAL, "EGL_MIN_SWAP_INTERVAL" },
    { EGL_MAX_SWAP_INTERVAL, "EGL_MAX_SWAP_INTERVAL" },
    { -1, NULL }
  };
  LOG("EGL configuration attibutes:");
  for (int index = 0; kAttribs[index].attrib != -1; index++) {
    EGLint value;
    if (eglGetConfigAttrib(display, config, kAttribs[index].attrib, &value))
      LOG("  %s: %d", kAttribs[index].name, static_cast<int>(value));
  }
}
#endif

}  // Anonymous namespace.

QHybrisScreen::QHybrisScreen()
    : depth_(32)
    , format_(QImage::Format_Invalid)
    , sfClient_(NULL)
    , sfSurface_(NULL)
    , platformContext_(NULL)
    , eglSurface_(NULL) {
  // Initialize surface flinger compatibility library and EGL.
  EGLint major, minor;
  sfClient_ = sf_client_create_full(false);
  ASSERT(sfClient_ != NULL);
  ASSERT(eglBindAPI(EGL_OPENGL_ES_API) == EGL_TRUE);
  eglDisplay_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  ASSERT(eglDisplay_ != EGL_NO_DISPLAY);
  ASSERT(eglInitialize(eglDisplay_, &major, &minor) == EGL_TRUE);
  DLOG("Initialized EGL version %d.%d", major, minor);

  // Set swap interval.
  int swapInterval = 1;
  QByteArray swapIntervalString = qgetenv("QTHYBRIS_SWAPINTERVAL");
  if (!swapIntervalString.isEmpty()) {
    bool ok;
    swapInterval = swapIntervalString.toInt(&ok);
    if (!ok)
      swapInterval = 1;
  }
  DLOG("setting swap interval to %d", swapInterval);
  eglSwapInterval(eglDisplay_, swapInterval);

  DLOG("QEglScreen::QEglScreen (this=%p)", this);
}

QHybrisScreen::~QHybrisScreen() {
  DLOG("QHybrisScreen::~QHybrisScreen");
  if (eglSurface_)
    eglDestroySurface(eglDisplay_, eglSurface_);
  eglTerminate(eglDisplay_);
  // FIXME(loicm) These are invalid since the structs are forward declarated, we need a way to clean
  //     these handles correctly.
  // delete sfSurface_;
  // delete sfClient_;
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
  depth_ = 32;
  format_ = QImage::Format_RGB32;
  if (!qEnvironmentVariableIsEmpty("QTHYBRIS_MULTISAMPLE")) {
    platformFormat.setSamples(4);
    DLOG("setting MSAA to 4 samples");
  }

  const EGLConfig kConfig = q_configFromGLFormat(eglDisplay_, platformFormat);
#if defined(QHYBRIS_DEBUG)
  printEglConfig(eglDisplay_, kConfig);
#endif
  w = sf_get_display_width(SURFACE_FLINGER_DEFAULT_DISPLAY_ID);
  h = sf_get_display_height(SURFACE_FLINGER_DEFAULT_DISPLAY_ID);
  SfSurfaceCreationParameters parameters = { 0, 0, w, h, -1, INT_MAX, 1.0f, false, "qthybris" };
  sfSurface_ = sf_surface_create(sfClient_, &parameters);
  ASSERT(sfSurface_ != NULL);
  const EGLNativeWindowType kNativeWindow = sf_surface_get_egl_native_window(sfSurface_);
  eglSurface_ = eglCreateWindowSurface(eglDisplay_, kConfig, kNativeWindow, NULL);
  ASSERT(eglSurface_ != EGL_NO_SURFACE);
  DLOG("created EGL surface %p", eglSurface_);

  platformContext_ = new QHybrisContext(platformFormat, eglDisplay_);

  eglQuerySurface(eglDisplay_, eglSurface_, EGL_WIDTH, &w);
  eglQuerySurface(eglDisplay_, eglSurface_, EGL_HEIGHT, &h);
  geometry_ = QRect(0, 0, w, h);
}

QRect QHybrisScreen::geometry() const {
  DLOG("QHybrisScreen::geometry (this=%p)", this);
  if (geometry_.isNull())
    createAndSetPlatformContext();
  return geometry_;
}

int QHybrisScreen::depth() const {
  DLOG("QHybrisScreen::depth (this=%p)", this);
  return depth_;
}

QImage::Format QHybrisScreen::format() const {
  DLOG("QHybrisScreen::format (this=%p)", this);
  if (format_ == QImage::Format_Invalid)
    createAndSetPlatformContext();
  return format_;
}

QPlatformOpenGLContext *QHybrisScreen::platformContext() const {
  DLOG("QHybrisScreen::platformContext (this=%p)", this);
  if (!platformContext_) {
    QHybrisScreen* screen = const_cast<QHybrisScreen*>(this);
    screen->createAndSetPlatformContext();
  }
  return platformContext_;
}
