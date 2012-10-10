QT += core quick gui qml

TARGET = hybris-qmlscene
TEMPLATE = app

HEADERS += hybrisqmlscene.h
SOURCES += hybrisqmlscene.cpp

LIBS += -lhybris_ics -lrt

OTHER_FILES += *.qml logo.png noise.png

target.path = $$[QT_INSTALL_EXAMPLES]/qthtbris
sources.path = $$[QT_INSTALL_EXAMPLES]/qthybris
sources.files = $$OTHER_FILES $$HEADERS $$SOURCES examples.pro
INSTALLS += target
