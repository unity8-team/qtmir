TARGET = qhybris
load(qt_plugin)

QT += core-private gui-private platformsupport-private

DESTDIR = ../../../../plugins/platforms

# Avoid X11 header collision (from the minimalegl plugin)
DEFINES += MESA_EGL_NO_X11_HEADERS

# Comment for debugging logs.
# DEFINES += QHYBRIS_DEBUG

SOURCES = main.cpp \
          qhybrisintegration.cpp \
          qhybriswindow.cpp \
          qhybrisbackingstore.cpp \
          qhybrisscreen.cpp \
          qhybrisinput.cpp

HEADERS = qhybrislogging.h \
          qhybrisintegration.h \
          qhybriswindow.h \
          qhybrisbackingstore.h \
          qhybrisscreen.h \
          qhybrisinput.h

CONFIG += egl qpa/genericunixfontdatabase

# FIXME(loicm) Remove hard-coded paths by adding pkg-config support to aal+.
INCLUDEPATH += /media/data/dev/projects/display_server/chroot/aal+/compat
LIBS += -L/media/data/dev/projects/display_server/chroot/aal+/hybris -lsf -lis

OTHER_FILES += hybris.json

target.path += $$[QT_INSTALL_PLUGINS]/platforms
INSTALLS += target
