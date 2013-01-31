// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#include <qpa/qplatformintegrationplugin.h>
#include "integration.h"

QT_BEGIN_NAMESPACE

class QUbuntuLegacyIntegrationPlugin : public QPlatformIntegrationPlugin {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QPA.QPlatformIntegrationFactoryInterface.5.1"
                    FILE "ubuntulegacy.json")

 public:
  QStringList keys() const;
  QPlatformIntegration* create(const QString&, const QStringList&);
};

QStringList QUbuntuLegacyIntegrationPlugin::keys() const {
  QStringList list;
  list << "ubuntulegacy";
  return list;
}

QPlatformIntegration* QUbuntuLegacyIntegrationPlugin::create(
    const QString& system, const QStringList& paramList) {
  Q_UNUSED(paramList);
  if (system.toLower() == "ubuntulegacy")
    return new QUbuntuLegacyIntegration();
  return 0;
}

QT_END_NAMESPACE

#include "main.moc"
