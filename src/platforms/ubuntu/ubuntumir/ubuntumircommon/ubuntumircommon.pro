TARGET = qubuntumircommon
TEMPLATE = lib

QT += core-private gui-private platformsupport-private sensors-private

DEFINES += MESA_EGL_NO_X11_HEADERS
QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_LFLAGS += -Wl,-no-undefined -L../ubuntucommon

CONFIG(debug) {
  QMAKE_CXXFLAGS_DEBUG += -Werror
}

SOURCES = input.cc integration.cc

HEADERS = input.h integration.h

CONFIG += static plugin create_prl link_prl

INCLUDEPATH += ../../../ ../../
LIBS += -Wl,--whole-archive -lqubuntucommon -Wl,--no-whole-archive
