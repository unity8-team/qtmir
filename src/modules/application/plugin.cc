// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include <QtQml/QtQml>
#include <QtQuick/QQuickWindow>
#include "application.h"
#include "application_manager.h"
#include "application_list_model.h"
#include "application_image.h"
#include "application_window.h"
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
        uri, 0, 1, "Application", "Application can't be instantiated");
    qmlRegisterUncreatableType<ApplicationListModel>(
        uri, 0, 1, "ApplicationListModel", "ApplicationListModel can't be instantiated");
    qmlRegisterExtendedType<QQuickWindow, ApplicationWindow>(uri, 0, 1, "Window");
    qmlRegisterType<ApplicationImage>(uri, 0, 1, "ApplicationImage");
  }
};

#include "plugin.moc"
