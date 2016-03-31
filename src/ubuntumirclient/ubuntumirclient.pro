TARGET = qpa-ubuntumirclient
TEMPLATE = lib

QT -= gui
QT += core-private platformsupport-private dbus

CONFIG += plugin no_keywords qpa/genericunixfontdatabase

DEFINES += MESA_EGL_NO_X11_HEADERS
# CONFIG += c++11 # only enables C++0x
QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden -std=c++11 -Werror -Wall
QMAKE_LFLAGS += -std=c++11 -Wl,-no-undefined

CONFIG += link_pkgconfig
PKGCONFIG += egl mirclient ubuntu-platform-api xkbcommon

SOURCES = \
    backingstore.cpp \
    clipboard.cpp \
    cursor.cpp \
    glcontext.cpp \
    input.cpp \
    integration.cpp \
    nativeinterface.cpp \
    offscreensurface.cpp \
    platformservices.cpp \
    plugin.cpp \
    screen.cpp \
    screenobserver.cpp \
    theme.cpp \
    window.cpp

HEADERS = \
    backingstore.h \
    clipboard.h \
    cursor.h \
    glcontext.h \
    input.h \
    integration.h \
    logging.h \
    nativeinterface.h \
    offscreensurface.h \
    orientationchangeevent_p.h \
    platformservices.h \
    plugin.h \
    screenobserver.h \
    screen.h \
    theme.h \
    window.h \
    utils.h

# Installation path
target.path +=  $$[QT_INSTALL_PLUGINS]/platforms

INSTALLS += target
