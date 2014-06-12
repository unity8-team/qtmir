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
#include "ubuntumircommon/integration.h"

QT_BEGIN_NAMESPACE

class QUbuntuMirClientIntegrationPlugin : public QPlatformIntegrationPlugin {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID QPlatformIntegrationFactoryInterface_iid
                    FILE "ubuntumirclient.json")

 public:
  QStringList keys() const;
  QPlatformIntegration* create(const QString&, const QStringList&);
};

QStringList QUbuntuMirClientIntegrationPlugin::keys() const {
  QStringList list;
  list << "ubuntumirclient";
  return list;
}

QPlatformIntegration* QUbuntuMirClientIntegrationPlugin::create(
    const QString& system, const QStringList& paramList) {
  Q_UNUSED(paramList);
  if (system.toLower() == "ubuntumirclient") {
#ifdef PLATFORM_API_TOUCH
        setenv("UBUNTU_PLATFORM_API_BACKEND", "touch_mirclient", 1);
#else
        setenv("UBUNTU_PLATFORM_API_BACKEND", "desktop_mirclient", 1);
#endif
    return new QUbuntuMirIntegration();
  }
  return 0;
}

QT_END_NAMESPACE

#include "main.moc"
