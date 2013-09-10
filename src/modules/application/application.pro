TARGET = unityapplicationplugin
TEMPLATE = lib

QT += quick-private qml-private
CONFIG += link_pkgconfig

# CONFIG += c++11 # only enables C++0x
QMAKE_CXXFLAGS += -std=c++11 -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_CXXFLAGS_RELEASE += -Werror     # so no stop on warning in debug builds
QMAKE_LFLAGS = -std=c++11 -Wl,-no-undefined

PKGCONFIG += unity-shell-application

CONFIG(debug) {
  QMAKE_CXXFLAGS_DEBUG += -Werror
}

SOURCES = plugin.cc \
          application_manager.cc \
          application_image.cc \
          application_window.cc \
          application.cc \
          input_filter_area.cc \
          desktopdata.cpp

HEADERS = application_manager.h \
          application_image.h \
          application_window.h \
          application.h \
          input_filter_area.h \
          desktopdata.h \
          logging.h \
          /usr/include/unity/shell/application/ApplicationInfoInterface.h \
          /usr/include/unity/shell/application/ApplicationManagerInterface.h

CONFIG += plugin

LIBS += -lubuntu_application_api

target.files += libunityapplicationplugin.so qmldir OSKController.qml
target.path += $$[QT_INSTALL_IMPORTS]/Unity/Application
INSTALLS += target
