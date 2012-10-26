QT += core quick gui qml

TARGET = qmlscene-hybris
TEMPLATE = app

HEADERS += hybrisqmlscene.h
SOURCES += hybrisqmlscene.cc

LIBS += -lrt

OTHER_FILES += *.qml logo.png noise.png

target.path = $$[QT_INSTALL_EXAMPLES]/qthtbris
sources.path = $$[QT_INSTALL_EXAMPLES]/qthybris
sources.files = $$OTHER_FILES $$HEADERS $$SOURCES examples.pro
INSTALLS += target
