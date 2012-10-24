TARGET = qhybrislegacy
TEMPLATE = lib

QT += core-private gui-private platformsupport-private

DESTDIR = ../../plugins/platforms

DEFINES += MESA_EGL_NO_X11_HEADERS
QMAKE_LFLAGS += -no-undefined

CONFIG(debug) {
  QMAKE_CXXFLAGS_DEBUG += -Werror
}

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
LIBS += -L../base -lHybrisBase -lhybris_ics -lsf -lis

OTHER_FILES += hybrislegacy.json

target.path += $$[QT_INSTALL_PLUGINS]/platforms
INSTALLS += target
