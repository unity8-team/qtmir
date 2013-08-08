TARGET = qubuntumirserver
TEMPLATE = lib

QT += core-private gui-private platformsupport-private sensors-private

DEFINES += MESA_EGL_NO_X11_HEADERS
QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_LFLAGS += -Wl,-no-undefined

CONFIG(debug) {
  QMAKE_CXXFLAGS_DEBUG += -Werror
}

SOURCES = main.cc 

CONFIG += plugin link_prl

INCLUDEPATH += ../../../ ../../ ../
LIBS += -Wl,--whole-archive -L../../../base -lubuntubase -L../../ubuntucommon -lqubuntucommon  -L../ubuntumircommon -lqubuntumircommon -Wl,--no-whole-archive -lubuntu_application_api_mirserver

OTHER_FILES += ubuntu.json

target.path += $$[QT_INSTALL_PLUGINS]/platforms
INSTALLS += target
