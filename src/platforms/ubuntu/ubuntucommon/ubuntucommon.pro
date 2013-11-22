TARGET = qubuntucommon
TEMPLATE = lib

QT += gui-private

DEFINES += MESA_EGL_NO_X11_HEADERS
QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_LFLAGS += -Wl,-no-undefined

CONFIG(debug) {
  QMAKE_CXXFLAGS_DEBUG += -Werror
}

SOURCES = integration.cc \
          window.cc \
          screen.cc \
          input.cc \
          clipboard.cc

HEADERS = integration.h \
          window.h \
          screen.h \
          input.h \
          clipboard.h \
          input_adaptor_factory.h

CONFIG += static plugin create_prl link_prl

INCLUDEPATH += ../../
LIBS += -L../base -Wl,--whole-archive -lubuntubase -Wl,--no-whole-archive
