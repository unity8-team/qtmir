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

#include <qpa/qplatformintegrationplugin.h>
#include "integration.h"

QT_BEGIN_NAMESPACE

class QUbuntuLegacyIntegrationPlugin : public QPlatformIntegrationPlugin {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID QPlatformIntegrationFactoryInterface_iid
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
