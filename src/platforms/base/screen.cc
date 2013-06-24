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

#include "screen.h"
#include "logging.h"
#include <QtPlatformSupport/private/qeglconvenience_p.h>

#include <ubuntu/application/ui/display.h>

static const int kSwapInterval = 1;

#if !defined(QT_NO_DEBUG)
static void printEglConfig(EGLDisplay display, EGLConfig config) {
  DASSERT(display != EGL_NO_DISPLAY);
  DASSERT(config != NULL);
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
  const char* string = eglQueryString(display, EGL_VENDOR);
  LOG("EGL vendor: %s", string);
  string = eglQueryString(display, EGL_VERSION);
  LOG("EGL version: %s", string);
  string = eglQueryString(display, EGL_EXTENSIONS);
  LOG("EGL extensions: %s", string);
  LOG("EGL configuration attibutes:");
  for (int index = 0; kAttribs[index].attrib != -1; index++) {
    EGLint value;
    if (eglGetConfigAttrib(display, config, kAttribs[index].attrib, &value))
      LOG("  %s: %d", kAttribs[index].name, static_cast<int>(value));
  }
}
#endif

QUbuntuBaseScreen::QUbuntuBaseScreen()
    : format_(QImage::Format_RGB32)
    , depth_(32)
    , surfaceFormat_()
    , eglDisplay_(EGL_NO_DISPLAY)
    , eglConfig_(NULL) {
  // Initialize EGL.
  ASSERT(eglBindAPI(EGL_OPENGL_ES_API) == EGL_TRUE);

  UAUiDisplay* u_display = ua_ui_display_new_with_index(0);
  ASSERT((eglDisplay_ = eglGetDisplay(ua_ui_display_get_native_type(u_display))) != EGL_NO_DISPLAY);
  ua_ui_display_destroy(u_display);
  ASSERT(eglInitialize(eglDisplay_, NULL, NULL) == EGL_TRUE);

  // Configure EGL buffers format.
  surfaceFormat_.setRedBufferSize(8);
  surfaceFormat_.setGreenBufferSize(8);
  surfaceFormat_.setBlueBufferSize(8);
  surfaceFormat_.setAlphaBufferSize(8);
  surfaceFormat_.setDepthBufferSize(24);
  surfaceFormat_.setStencilBufferSize(8);
  if (!qEnvironmentVariableIsEmpty("QTUBUNTU_MULTISAMPLE")) {
    surfaceFormat_.setSamples(4);
    DLOG("setting MSAA to 4 samples");
  }
  eglConfig_ = q_configFromGLFormat(eglDisplay_, surfaceFormat_, true);
#if !defined(QT_NO_DEBUG)
  printEglConfig(eglDisplay_, eglConfig_);
#endif

  // Set vblank swap interval.
  int swapInterval = kSwapInterval;
  QByteArray swapIntervalString = qgetenv("QTUBUNTU_SWAPINTERVAL");
  if (!swapIntervalString.isEmpty()) {
    bool ok;
    swapInterval = swapIntervalString.toInt(&ok);
    if (!ok)
      swapInterval = kSwapInterval;
  }
  DLOG("setting swap interval to %d", swapInterval);
  eglSwapInterval(eglDisplay_, swapInterval);

  DLOG("QUbuntuBaseScreen::QUbuntuBaseScreen (this=%p)", this);
}

QUbuntuBaseScreen::~QUbuntuBaseScreen() {
  eglTerminate(eglDisplay_);
  DLOG("QUbuntuBaseScreen::~QUbuntuBaseScreen");
}
