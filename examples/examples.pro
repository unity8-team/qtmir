QT += core quick gui

TARGET = hybris-qmlscene
TEMPLATE = app

HEADERS += hybrisqmlscene.h
SOURCES += hybrisqmlscene.cpp

# FIXME(loicm) Remove hard-coded paths by adding pkg-config support to aal+.
LIBS += -lhybris_ics -lrt

OTHER_FILES += *.qml logo.png noise.png

target.path = $$[QT_INSTALL_EXAMPLES]/qthtbris
sources.path = $$[QT_INSTALL_EXAMPLES]/qthybris
sources.files = $$OTHER_FILES $$HEADERS $$SOURCES examples.pro
INSTALLS += target
