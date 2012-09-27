QT += core quick gui

TARGET = hybris-qmlscene
TEMPLATE = app

HEADERS += hybrisqmlscene.h
SOURCES += hybrisqmlscene.cpp

# FIXME(loicm) Remove hard-coded paths by adding pkg-config support to aal+.
LIBS += -L/media/data/dev/projects/display_server/chroot/aal+/hybris -lhybris_ics
LIBS += -lrt

OTHER_FILES += *.qml logo.png

target.path = $$[QT_INSTALL_EXAMPLES]/qthtbris
sources.path = $$[QT_INSTALL_EXAMPLES]/qthybris
sources.files = $$OTHER_FILES $$HEADERS $$SOURCES examples.pro
INSTALLS += target
