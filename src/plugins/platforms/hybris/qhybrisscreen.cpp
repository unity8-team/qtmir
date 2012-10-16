// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include "qhybrisscreen.h"
#include "qhybrislogging.h"
#include <QtPlatformSupport/private/qeglconvenience_p.h>
#include <surface_flinger/surface_flinger_compatibility_layer.h>

static const int kSwapInterval = 1;

#if defined(QHYBRIS_DEBUG)
static void printEglConfig(EGLDisplay display, EGLConfig config) {
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

QHybrisScreen::QHybrisScreen()
    : format_(QImage::Format_RGB32)
    , depth_(32) {
  // Initialize SF compat library and store the screen size.
  ASSERT((sfClient_ = sf_client_create_full(false)) != NULL);
  geometry_ = QRect(0, 0, sf_get_display_width(SURFACE_FLINGER_DEFAULT_DISPLAY_ID),
                    sf_get_display_height(SURFACE_FLINGER_DEFAULT_DISPLAY_ID));

  // Initialize EGL.
  EGLint major, minor;
  ASSERT(eglBindAPI(EGL_OPENGL_ES_API) == EGL_TRUE);
  ASSERT((eglDisplay_ = eglGetDisplay(EGL_DEFAULT_DISPLAY)) != EGL_NO_DISPLAY);
  ASSERT(eglInitialize(eglDisplay_, &major, &minor) == EGL_TRUE);
  DLOG("EGL version: %d.%d", major, minor);

  // Configure EGL buffers format.
  surfaceFormat_.setRedBufferSize(8);
  surfaceFormat_.setGreenBufferSize(8);
  surfaceFormat_.setBlueBufferSize(8);
  surfaceFormat_.setDepthBufferSize(24);
  surfaceFormat_.setStencilBufferSize(8);
  if (!qEnvironmentVariableIsEmpty("QTHYBRIS_MULTISAMPLE")) {
    surfaceFormat_.setSamples(4);
    DLOG("setting MSAA to 4 samples");
  }
  eglConfig_ = q_configFromGLFormat(eglDisplay_, surfaceFormat_, true);
#if defined(QHYBRIS_DEBUG)
  printEglConfig(eglDisplay_, eglConfig_);
#endif

  // Set vblank swap interval.
  int swapInterval = kSwapInterval;
  QByteArray swapIntervalString = qgetenv("QTHYBRIS_SWAPINTERVAL");
  if (!swapIntervalString.isEmpty()) {
    bool ok;
    swapInterval = swapIntervalString.toInt(&ok);
    if (!ok)
      swapInterval = kSwapInterval;
  }
  DLOG("setting swap interval to %d", swapInterval);
  eglSwapInterval(eglDisplay_, swapInterval);

  DLOG("QHybrisScreen::QHybrisScreen (this=%p)", this);
}

QHybrisScreen::~QHybrisScreen() {
  DLOG("QHybrisScreen::~QHybrisScreen");
  eglTerminate(eglDisplay_);
  // FIXME(loicm) Invalid because the struct is forward declarated, we need a way to clean the
  //     handle correctly.
  // delete sfClient_;
}

QRect QHybrisScreen::geometry() const {
  DLOG("QHybrisScreen::geometry (this=%p)", this);
  return geometry_;
}

int QHybrisScreen::depth() const {
  DLOG("QHybrisScreen::depth (this=%p)", this);
  return depth_;
}

QImage::Format QHybrisScreen::format() const {
  DLOG("QHybrisScreen::format (this=%p)", this);
  return format_;
}
