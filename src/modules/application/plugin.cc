// This file is part of QtUbuntu, a set of Qt components for Ubuntu.
// Copyright © 2013 Canonical Ltd.
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 3, as published by
// the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
// SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <QtQml/QtQml>
#include <QtQuick/QQuickWindow>
#include "application.h"
#include "application_manager.h"
#include "application_list_model.h"
#include "application_image.h"
#include "application_window.h"
#include "input_filter_area.h"
#include "logging.h"

static QObject* applicationManagerSingleton(QQmlEngine* engine, QJSEngine* scriptEngine) {
  Q_UNUSED(engine);
  Q_UNUSED(scriptEngine);
  DLOG("applicationManagerSingleton (engine=%p, scriptEngine=%p)", engine, scriptEngine);
  return new ApplicationManager();
}

class UbuntuApplicationPlugin : public QQmlExtensionPlugin {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface/1.0")

  virtual void registerTypes(const char* uri) {
    DLOG("UbuntuApplicationPlugin::registerTypes (this=%p, uri='%s')", this, uri);
    ASSERT(QLatin1String(uri) == QLatin1String("Ubuntu.Application"));
    qmlRegisterSingletonType<ApplicationManager>(
        uri, 0, 1, "ApplicationManager", applicationManagerSingleton);
    qmlRegisterUncreatableType<Application>(
        uri, 0, 1, "ApplicationInfo", "ApplicationInfo can't be instantiated");
    qmlRegisterUncreatableType<ApplicationListModel>(
        uri, 0, 1, "ApplicationListModel", "ApplicationListModel can't be instantiated");
    qmlRegisterExtendedType<QQuickWindow, ApplicationWindow>(uri, 0, 1, "Window");
    qmlRegisterType<ApplicationImage>(uri, 0, 1, "ApplicationImage");
    qmlRegisterType<InputFilterArea>(uri, 0, 1, "InputFilterArea");
  }
};

#include "plugin.moc"
