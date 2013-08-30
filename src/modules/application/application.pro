TARGET = unityapplicationplugin
TEMPLATE = lib

QT += quick-private qml-private
CONFIG += link_pkgconfig

QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_LFLAGS += -Wl,-no-undefined

PKGCONFIG += unity-shell-application

CONFIG(debug) {
  QMAKE_CXXFLAGS_DEBUG += -Werror
}

SOURCES = plugin.cc \
          application_manager.cc \
          application_list_model.cc \
          application_image.cc \
          application_window.cc \
          application.cc \
          input_filter_area.cc

HEADERS = application_manager.h \
          application_list_model.h \
          application_image.h \
          application_window.h \
          application.h \
          input_filter_area.h \
          logging.h

CONFIG += plugin

LIBS += -lubuntu_application_api

target.files += libunityapplicationplugin.so qmldir
target.path += $$[QT_INSTALL_IMPORTS]/Unity/Application
INSTALLS += target
