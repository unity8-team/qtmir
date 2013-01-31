TARGET = qubuntulegacy
TEMPLATE = lib

QT += core-private gui-private platformsupport-private

DEFINES += MESA_EGL_NO_X11_HEADERS
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

PRE_TARGETDEPS = ../base/libubuntubase.a

INCLUDEPATH += .. /usr/include/hybris
LIBS += -L../base -lubuntubase -lhybris_ics -lsf -lis

OTHER_FILES += ubuntulegacy.json

target.path += $$[QT_INSTALL_PLUGINS]/platforms
INSTALLS += target
