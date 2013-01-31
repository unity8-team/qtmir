// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include <qpa/qplatformintegrationplugin.h>
#include "integration.h"

QT_BEGIN_NAMESPACE

class QUbuntuIntegrationPlugin : public QPlatformIntegrationPlugin {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QPA.QPlatformIntegrationFactoryInterface.5.1"
                    FILE "ubuntu.json")

 public:
  QStringList keys() const;
  QPlatformIntegration* create(const QString&, const QStringList&);
};

QStringList QUbuntuIntegrationPlugin::keys() const {
  QStringList list;
  list << "ubuntu";
  return list;
}

QPlatformIntegration* QUbuntuIntegrationPlugin::create(
    const QString& system, const QStringList& paramList) {
  Q_UNUSED(paramList);
  if (system.toLower() == "ubuntu")
    return new QUbuntuIntegration();
  return 0;
}

QT_END_NAMESPACE

#include "main.moc"
