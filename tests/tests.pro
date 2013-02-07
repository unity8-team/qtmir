TEMPLATE = subdirs

SUBDIRS += qmlscene_ubuntu clipboard

OTHER_FILES += *.qml logo.png noise.png

target.path = $$[QT_INSTALL_EXAMPLES]/qtubuntu
sources.path = $$[QT_INSTALL_EXAMPLES]/qtubuntu
sources.files = $$OTHER_FILES tests.pro
INSTALLS += target
