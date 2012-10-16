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
          qhybriscontext.cpp \
          qhybriswindow.cpp \
          qhybrisbackingstore.cpp \
          qhybrisscreen.cpp \
          qhybrisinput.cpp \
          qhybrisnativeinterface.cpp

HEADERS = qhybrislogging.h \
          qhybrisintegration.h \
          qhybriscontext.h \
          qhybriswindow.h \
          qhybrisbackingstore.h \
          qhybrisscreen.h \
          qhybrisinput.h \
          qhybrisnativeinterface.h

CONFIG += egl qpa/genericunixfontdatabase

INCLUDEPATH += /usr/include/hybris
LIBS += -lsf -lis -lhybris_ics

OTHER_FILES += hybris.json

target.path += $$[QT_INSTALL_PLUGINS]/platforms
INSTALLS += target
