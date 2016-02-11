/*
 * Copyright (C) 2014-2015 Canonical, Ltd.
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

// Local
#include "integration.h"
#include "backingstore.h"
#include "clipboard.h"
#include "glcontext.h"
#include "input.h"
#include "logging.h"
#include "nativeinterface.h"
#include "offscreensurface.h"
#include "screen.h"
#include "theme.h"
#include "window.h"

// Qt
#include <QGuiApplication>
#include <private/qguiapplication_p.h>
#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatforminputcontextfactory_p.h>
#include <qpa/qplatforminputcontext.h>
#include <QtPlatformSupport/private/qgenericunixfontdatabase_p.h>
#include <QtPlatformSupport/private/qgenericunixeventdispatcher_p.h>
#include <QOpenGLContext>

// platform-api
#include <ubuntu/application/lifecycle_delegate.h>
#include <ubuntu/application/id.h>
#include <ubuntu/application/options.h>

static void resumedCallback(const UApplicationOptions *options, void* context)
{
    Q_UNUSED(options)
    Q_UNUSED(context)
    Q_ASSERT(context != NULL);
    if (qGuiApp->focusWindow()) {
        QWindowSystemInterface::handleApplicationStateChanged(Qt::ApplicationActive);
    } else {
        QWindowSystemInterface::handleApplicationStateChanged(Qt::ApplicationInactive);
    }
}

static void aboutToStopCallback(UApplicationArchive *archive, void* context)
{
    Q_UNUSED(archive)
    Q_ASSERT(context != NULL);
    UbuntuClientIntegration* integration = static_cast<UbuntuClientIntegration*>(context);
    QPlatformInputContext *inputContext = integration->inputContext();
    if (inputContext) {
        inputContext->hideInputPanel();
    } else {
        qCWarning(ubuntumirclient) << "aboutToStopCallback(): no input context";
    }
    QWindowSystemInterface::handleApplicationStateChanged(Qt::ApplicationSuspended);
}

UbuntuClientIntegration::UbuntuClientIntegration()
    : QPlatformIntegration()
    , mNativeInterface(new UbuntuNativeInterface)
    , mFontDb(new QGenericUnixFontDatabase)
    , mServices(new UbuntuPlatformServices)
    , mClipboard(new UbuntuClipboard)
    , mScaleFactor(1.0)
{
    setupOptions();
    setupDescription();

    // Create new application instance
    mInstance = u_application_instance_new_from_description_with_options(mDesc, mOptions);

    if (mInstance == nullptr)
        qFatal("UbuntuClientIntegration: connection to Mir server failed. Check that a Mir server is\n"
               "running, and the correct socket is being used and is accessible. The shell may have\n"
               "rejected the incoming connection, so check its log file");

    mNativeInterface->setMirConnection(u_application_instance_get_mir_connection(mInstance));
}

void UbuntuClientIntegration::initialize()
{
    MirConnection *mirConnection = u_application_instance_get_mir_connection(mInstance);

    // Init the ScreenObserver
    mScreenObserver.reset(new UbuntuScreenObserver(mirConnection));
    connect(mScreenObserver.data(), &UbuntuScreenObserver::screenAdded,
            [this](UbuntuScreen *screen) { this->screenAdded(screen); });
    connect(mScreenObserver.data(), &UbuntuScreenObserver::screenRemoved,
                     this, &UbuntuClientIntegration::destroyScreen);

    Q_FOREACH(auto screen, mScreenObserver->screens()) {
        screenAdded(screen);
    }

    // Initialize input.
    mInput = new UbuntuInput(this);
    mInputContext = QPlatformInputContextFactory::create();

    // compute the scale factor
    const int defaultGridUnit = 8;
    int gridUnit = defaultGridUnit;
    QByteArray gridUnitString = qgetenv("GRID_UNIT_PX");
    if (!gridUnitString.isEmpty()) {
        bool ok;
        gridUnit = gridUnitString.toInt(&ok);
        if (!ok) {
            gridUnit = defaultGridUnit;
        }
    }
    mScaleFactor = static_cast<qreal>(gridUnit) / defaultGridUnit;
}

UbuntuClientIntegration::~UbuntuClientIntegration()
{
    delete mInput;
    delete mInputContext;
    delete mServices;
}

QPlatformServices *UbuntuClientIntegration::services() const
{
    return mServices;
}

void UbuntuClientIntegration::setupOptions()
{
    QStringList args = QCoreApplication::arguments();
    int argc = args.size() + 1;
    char **argv = new char*[argc];
    for (int i = 0; i < argc - 1; i++)
        argv[i] = qstrdup(args.at(i).toLocal8Bit());
    argv[argc - 1] = nullptr;

    mOptions = u_application_options_new_from_cmd_line(argc - 1, argv);

    for (int i = 0; i < argc; i++)
        delete [] argv[i];
    delete [] argv;
}

void UbuntuClientIntegration::setupDescription()
{
    mDesc = u_application_description_new();
    UApplicationId* id = u_application_id_new_from_stringn("QtUbuntu", 8);
    u_application_description_set_application_id(mDesc, id);

    UApplicationLifecycleDelegate* delegate = u_application_lifecycle_delegate_new();
    u_application_lifecycle_delegate_set_application_resumed_cb(delegate, &resumedCallback);
    u_application_lifecycle_delegate_set_application_about_to_stop_cb(delegate, &aboutToStopCallback);
    u_application_lifecycle_delegate_set_context(delegate, this);
    u_application_description_set_application_lifecycle_delegate(mDesc, delegate);
}

QPlatformWindow* UbuntuClientIntegration::createPlatformWindow(QWindow* window) const
{
    return const_cast<UbuntuClientIntegration*>(this)->createPlatformWindow(window);
}

QPlatformWindow* UbuntuClientIntegration::createPlatformWindow(QWindow* window)
{
    return new UbuntuWindow(window, mClipboard, mInput, mNativeInterface,
                            u_application_instance_get_mir_connection(mInstance));
}

bool UbuntuClientIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    switch (cap) {
    case ThreadedPixmaps:
        return true;

    case OpenGL:
        return true;

    case ApplicationState:
        return true;

    case ThreadedOpenGL:
        if (qEnvironmentVariableIsEmpty("QTUBUNTU_NO_THREADED_OPENGL")) {
            return true;
        } else {
            qCDebug(ubuntumirclient, "disabled threaded OpenGL");
            return false;
        }
    case MultipleWindows:
    case NonFullScreenWindows:
        return true;
    default:
        return QPlatformIntegration::hasCapability(cap);
    }
}

QAbstractEventDispatcher *UbuntuClientIntegration::createEventDispatcher() const
{
    return createUnixEventDispatcher();
}

QPlatformBackingStore* UbuntuClientIntegration::createPlatformBackingStore(QWindow* window) const
{
    return new UbuntuBackingStore(window);
}

QPlatformOpenGLContext* UbuntuClientIntegration::createPlatformOpenGLContext(
        QOpenGLContext* context) const
{
    return const_cast<UbuntuClientIntegration*>(this)->createPlatformOpenGLContext(context);
}

QPlatformOpenGLContext* UbuntuClientIntegration::createPlatformOpenGLContext(
        QOpenGLContext* context)
{
    return new UbuntuOpenGLContext(static_cast<UbuntuScreen*>(context->screen()->handle()),
                                   static_cast<UbuntuOpenGLContext*>(context->shareHandle()));
}

QStringList UbuntuClientIntegration::themeNames() const
{
    return QStringList(UbuntuTheme::name);
}

QPlatformTheme* UbuntuClientIntegration::createPlatformTheme(const QString& name) const
{
    Q_UNUSED(name);
    return new UbuntuTheme;
}

QVariant UbuntuClientIntegration::styleHint(StyleHint hint) const
{
    switch (hint) {
        case QPlatformIntegration::StartDragDistance: {
            // default is 10 pixels (see QPlatformTheme::defaultThemeHint)
            return 10.0 * mScaleFactor;
        }
        case QPlatformIntegration::PasswordMaskDelay: {
            // return time in milliseconds - 1 second
            return QVariant(1000);
        }
        default:
            break;
    }
    return QPlatformIntegration::styleHint(hint);
}

QPlatformClipboard* UbuntuClientIntegration::clipboard() const
{
    return mClipboard.data();
}

QPlatformNativeInterface* UbuntuClientIntegration::nativeInterface() const
{
    return mNativeInterface;
}

QPlatformOffscreenSurface *UbuntuClientIntegration::createPlatformOffscreenSurface(
        QOffscreenSurface *surface) const
{
    return new UbuntuOffscreenSurface(surface);
}

void UbuntuClientIntegration::destroyScreen(UbuntuScreen *screen)
{
    // FIXME: on deleting a screen while a Window is on it, Qt will automatically
    // move the window to the primaryScreen(). This will trigger a screenChanged
    // signal, causing things like QQuickScreenAttached to re-fetch screen properties
    // like DPI and physical size. However this is crashing, as Qt is calling virtual
    // functions on QPlatformScreen, for reasons unclear. As workaround, move window
    // to primaryScreen() before deleting the screen. Might be QTBUG-38650

    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    if (screen != primaryScreen->handle()) {
        uint32_t movedWindowCount = 0;
        Q_FOREACH (QWindow *w, QGuiApplication::topLevelWindows()) {
            if (w->screen()->handle() == screen) {
                QWindowSystemInterface::handleWindowScreenChanged(w, primaryScreen);
                ++movedWindowCount;
            }
        }
        if (movedWindowCount > 0) {
            QWindowSystemInterface::flushWindowSystemEvents();
        }
    }

    qDebug() << "Removing Screen with id" << screen->outputId() << "and geometry" << screen->geometry();
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
    delete screen;
#else
    this->destroyScreen(screen);
#endif
}
