/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Qt
#include <QQmlExtensionPlugin>

// local
#include "application.h"
#include "application_manager.h"
#include "applicationscreenshotprovider.h"
#include "mirsurfacemanager.h"
#include "mirsurfaceitem.h"
#include "sessionmanager.h"
#include "ubuntukeyboardinfo.h"

// qtmir
#include "logging.h"

namespace {

QObject* applicationManagerSingleton(QQmlEngine* /*engine*/, QJSEngine* jsEngine)
{
    return qtmir::ApplicationManager::singleton(jsEngine);
}

QObject* surfaceManagerSingleton(QQmlEngine* /*engine*/, QJSEngine* jsEngine)
{
    return qtmir::MirSurfaceManager::singleton(jsEngine);
}

QObject* sessionManagerSingleton(QQmlEngine* /*engine*/, QJSEngine* jsEngine)
{
    return qtmir::SessionManager::singleton(jsEngine);
}

QObject* ubuntuKeyboardInfoSingleton(QQmlEngine* /*engine*/, QJSEngine* /*scriptEngine*/)
{
    if (!qtmir::UbuntuKeyboardInfo::instance()) {
        new qtmir::UbuntuKeyboardInfo;
    }
    return qtmir::UbuntuKeyboardInfo::instance();
}
} // anonymous namespace

class UnityApplicationPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface/1.0")

    virtual void registerTypes(const char* uri)
    {
        qCDebug(QTMIR_APPLICATIONS) << "UnityApplicationPlugin::registerTypes - this=" << this << "uri=" << uri;
        Q_ASSERT(QLatin1String(uri) == QLatin1String("Unity.Application"));

        qRegisterMetaType<qtmir::ApplicationManager*>("ApplicationManager*"); //need for queueing signals
        qRegisterMetaType<qtmir::Application*>("Application*");
        qRegisterMetaType<qtmir::MirSurfaceItem*>("MirSurfaceItem*");
        qRegisterMetaType<qtmir::MirSurfaceItemModel*>("MirSurfaceItemModel*");
        qRegisterMetaType<qtmir::Session*>("Session*");
        qRegisterMetaType<qtmir::SessionInterface*>("SessionInterface*");
        qRegisterMetaType<qtmir::SessionModel*>("SessionModel*");

        qmlRegisterUncreatableType<unity::shell::application::ApplicationManagerInterface>(
                    uri, 0, 1, "ApplicationManagerInterface", "Abstract interface. Cannot be created in QML");
        qmlRegisterSingletonType<qtmir::ApplicationManager>(
                    uri, 0, 1, "ApplicationManager", applicationManagerSingleton);
        qmlRegisterUncreatableType<unity::shell::application::ApplicationInfoInterface>(
                    uri, 0, 1, "ApplicationInfoInterface", "Abstract interface. Cannot be created in QML");
        qmlRegisterUncreatableType<qtmir::Application>(
                    uri, 0, 1, "ApplicationInfo", "Application can't be instantiated");
        qmlRegisterSingletonType<qtmir::MirSurfaceManager>(
                    uri, 0, 1, "SurfaceManager", surfaceManagerSingleton);
        qmlRegisterSingletonType<qtmir::SessionManager>(
                    uri, 0, 1, "SessionManager", sessionManagerSingleton);
        qmlRegisterUncreatableType<qtmir::MirSurfaceItem>(
                    uri, 0, 1, "MirSurfaceItem", "MirSurfaceItem can't be instantiated from QML");
        qmlRegisterUncreatableType<qtmir::Session>(
                    uri, 0, 1, "Session", "Session can't be instantiated from QML");
        qmlRegisterSingletonType<qtmir::UbuntuKeyboardInfo>(
                uri, 0, 1, "UbuntuKeyboardInfo", ubuntuKeyboardInfoSingleton);
    }

    virtual void initializeEngine(QQmlEngine *engine, const char *uri)
    {
        QQmlExtensionPlugin::initializeEngine(engine, uri);

        qtmir::ApplicationManager* appManager
                = static_cast<qtmir::ApplicationManager*>(applicationManagerSingleton(engine, engine));
        engine->addImageProvider(QLatin1String("application"), new qtmir::ApplicationScreenshotProvider(appManager));
    }
};

#include "plugin.moc"
