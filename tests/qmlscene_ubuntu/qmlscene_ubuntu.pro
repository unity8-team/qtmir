QT += core quick qml gui-private

TARGET = qmlscene-ubuntu
TEMPLATE = app

HEADERS += qmlscene_ubuntu.h
SOURCES += qmlscene_ubuntu.cc

LIBS += -lrt

target.path = $$[QT_INSTALL_EXAMPLES]/qtubuntu
sources.path = $$[QT_INSTALL_EXAMPLES]/qtubuntu
sources.files = $$HEADERS $$SOURCES qmlscene_ubuntu.pro
INSTALLS += target
