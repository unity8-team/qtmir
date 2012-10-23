TARGET = qhybris
TEMPLATE = lib

QT += core-private gui-private platformsupport-private

# Uncomment for debugging logs.
# DEFINES += QHYBRIS_DEBUG

DESTDIR = ../../plugins/platforms

DEFINES += MESA_EGL_NO_X11_HEADERS

SOURCES = main.cc \
          integration.cc \
          window.cc \
          screen.cc \
          input.cc

HEADERS = integration.h \
          window.h \
          screen.h \
          input.h

CONFIG += plugin egl

INCLUDEPATH += .. /usr/include/hybris
LIBS += -L../base -lHybrisBase -lhybris_ics -lubuntu_application_api

OTHER_FILES += hybris.json

target.path += $$[QT_INSTALL_PLUGINS]/platforms
INSTALLS += target
