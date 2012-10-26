TARGET = qhybris
TEMPLATE = lib

QT += core-private gui-private platformsupport-private

DESTDIR = ../../plugins/platforms

DEFINES += MESA_EGL_NO_X11_HEADERS QT_COMPILES_IN_HARFBUZZ
QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_LFLAGS += -Wl,-no-undefined

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

CONFIG += plugin link_prl

PRE_TARGETDEPS = ../base/libhybrisbase.a

INCLUDEPATH += .. /usr/include/hybris
LIBS += -L../base -lhybrisbase -lhybris_ics -lubuntu_application_api

OTHER_FILES += hybris.json

target.path += $$[QT_INSTALL_PLUGINS]/platforms
INSTALLS += target
