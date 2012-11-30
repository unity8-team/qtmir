// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include <Qt/QtQml>
#include "application_manager.h"
#include "logging.h"

class UbuntuApplicationPlugin : public QQmlExtensionPlugin {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface/1.0")

 public:
  ~UbuntuApplicationPlugin() {
    delete applicationManager_;
  }

  virtual void registerTypes(const char* uri) {
    DLOG("UbuntuApplicationPlugin::registerTypes (this=%p, uri='%s')", this, uri);
    ASSERT(QLatin1String(uri) == QLatin1String("Ubuntu.Application"));
    qmlRegisterUncreatableType<ApplicationManager>(
        uri, 0, 1, "ApplicationManager", "ApplicationManager can't be created as it's only meant "
        "to expose enum values, use the applicationManager context property instead.");
  }

  virtual void initializeEngine(QQmlEngine* engine, const char* uri) {
    DLOG("UbuntuApplicationPlugin::initializeEngine (this=%p, engine=%p, uri='%s')",
         this, engine, uri);
    ASSERT(QLatin1String(uri) == QLatin1String("Ubuntu.Application"));
    applicationManager_ = new ApplicationManager();
    engine->rootContext()->setContextProperty("applicationManager", applicationManager_);
  }

 private:
  ApplicationManager* applicationManager_;
};

#include "plugin.moc"
