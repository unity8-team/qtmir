TARGET = HybrisBase
TEMPLATE = lib

QT += core-private gui-private platformsupport-private

# Uncomment for debugging logs.
# DEFINES += QHYBRIS_DEBUG

DEFINES += MESA_EGL_NO_X11_HEADERS

SOURCES = integration.cc \
          backing_store.cc \
          native_interface.cc \
          context.cc \
          screen.cc \
          window.cc \
          input.cc

HEADERS = integration.h \
          backing_store.h \
          native_interface.h \
          context.h \
          screen.h \
          window.h \
          input.h \
          logging.h

CONFIG += static egl

INCLUDEPATH += /usr/include/hybris
