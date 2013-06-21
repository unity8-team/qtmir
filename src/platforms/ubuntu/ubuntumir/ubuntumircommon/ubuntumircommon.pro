TARGET = qubuntucommon
TEMPLATE = lib

QT += core-private gui-private platformsupport-private sensors-private

DEFINES += MESA_EGL_NO_X11_HEADERS
QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_LFLAGS += -Wl,-no-undefined

CONFIG(debug) {
  QMAKE_CXXFLAGS_DEBUG += -Werror
}

SOURCES = input.cc

HEADERS = integration.h

CONFIG += static plugin link_prl

INCLUDEPATH += ../../../ ../../
LIBS += -L../../../base -Wl,--whole-archive -lubuntubase -Wl,--no-whole-archive
