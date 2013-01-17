QT += core quick qml gui-private

TARGET = qmlscene-ubuntu
TEMPLATE = app

HEADERS += qmlscene_ubuntu.h
SOURCES += qmlscene_ubuntu.cc

LIBS += -lrt

OTHER_FILES += *.qml logo.png noise.png

target.path = $$[QT_INSTALL_EXAMPLES]/qtubuntu
sources.path = $$[QT_INSTALL_EXAMPLES]/qtubuntu
sources.files = $$OTHER_FILES $$HEADERS $$SOURCES examples.pro
INSTALLS += target
