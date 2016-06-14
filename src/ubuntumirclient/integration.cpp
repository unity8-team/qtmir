/*
 * Copyright (C) 2014-2016 Canonical, Ltd.
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
#include <QFileInfo>
#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatforminputcontextfactory_p.h>
#include <qpa/qplatforminputcontext.h>
#include <QtPlatformSupport/private/qeglconvenience_p.h>
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
    , mNativeInterface(new UbuntuNativeInterface(this))
    , mFontDb(new QGenericUnixFontDatabase)
    , mServices(new UbuntuPlatformServices)
    , mClipboard(new UbuntuClipboard)
    , mScaleFactor(1.0)
{
    {
        QStringList args = QCoreApplication::arguments();
        setupOptions(args);
        QByteArray sessionName = generateSessionName(args);
        setupDescription(sessionName);
    }

    // Create new application instance
    mInstance = u_application_instance_new_from_description_with_options(mDesc, mOptions);

    if (mInstance == nullptr)
        qFatal("UbuntuClientIntegration: connection to Mir server failed. Check that a Mir server is\n"
               "running, and the correct socket is being used and is accessible. The shell may have\n"
               "rejected the incoming connection, so check its log file");

    mMirConnection = u_application_instance_get_mir_connection(mInstance);

    // Choose the default surface format suited to the Mir platform
    QSurfaceFormat defaultFormat;
    defaultFormat.setRedBufferSize(8);
    defaultFormat.setGreenBufferSize(8);
    defaultFormat.setBlueBufferSize(8);
    QSurfaceFormat::setDefaultFormat(defaultFormat);

    // Initialize EGL.
    mEglNativeDisplay = mir_connection_get_egl_native_display(mMirConnection);
    ASSERT((mEglDisplay = eglGetDisplay(mEglNativeDisplay)) != EGL_NO_DISPLAY);
    ASSERT(eglInitialize(mEglDisplay, nullptr, nullptr) == EGL_TRUE);
}

void UbuntuClientIntegration::initialize()
{
    // Init the ScreenObserver
    mScreenObserver.reset(new UbuntuScreenObserver(mMirConnection));
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
    eglTerminate(mEglDisplay);
    delete mInput;
    delete mInputContext;
    delete mServices;
}

QPlatformServices *UbuntuClientIntegration::services() const
{
    return mServices;
}

void UbuntuClientIntegration::setupOptions(QStringList &args)
{
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

void UbuntuClientIntegration::setupDescription(QByteArray &sessionName)
{
    mDesc = u_application_description_new();

    UApplicationId* id = u_application_id_new_from_stringn(sessionName.data(), sessionName.count());
    u_application_description_set_application_id(mDesc, id);

    UApplicationLifecycleDelegate* delegate = u_application_lifecycle_delegate_new();
    u_application_lifecycle_delegate_set_application_resumed_cb(delegate, &resumedCallback);
    u_application_lifecycle_delegate_set_application_about_to_stop_cb(delegate, &aboutToStopCallback);
    u_application_lifecycle_delegate_set_context(delegate, this);
    u_application_description_set_application_lifecycle_delegate(mDesc, delegate);
}

QByteArray UbuntuClientIntegration::generateSessionName(QStringList &args)
{
    // Try to come up with some meaningful session name to uniquely identify this session,
    // helping with shell debugging

    if (args.count() == 0) {
        return QByteArray("QtUbuntu");
    } if (args[0].contains("qmlscene")) {
        return generateSessionNameFromQmlFile(args);
    } else {
        // use the executable name
        QFileInfo fileInfo(args[0]);
        return fileInfo.fileName().toLocal8Bit();
    }
}

QByteArray UbuntuClientIntegration::generateSessionNameFromQmlFile(QStringList &args)
{
    Q_FOREACH (QString arg, args) {
        if (arg.endsWith(".qml")) {
            QFileInfo fileInfo(arg);
            return fileInfo.fileName().toLocal8Bit();
        }
    }

    // give up
    return "qmlscene";
}

QPlatformWindow* UbuntuClientIntegration::createPlatformWindow(QWindow* window) const
{
    return new UbuntuWindow(window, mClipboard, mInput, mNativeInterface, mEglDisplay, mMirConnection);
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
    QSurfaceFormat format(context->format());

    auto platformContext = new UbuntuOpenGLContext(format, context->shareHandle(), mEglDisplay);
    if (!platformContext->isValid()) {
        // Older Intel Atom-based devices only support OpenGL 1.4 compatibility profile but by default
        // QML asks for at least OpenGL 2.0. The XCB GLX backend ignores this request and returns a
        // 1.4 context, but the XCB EGL backend tries to honour it, and fails. The 1.4 context appears to
        // have sufficient capabilities on MESA (i915) to render correctly however. So reduce the default
        // requested OpenGL version to 1.0 to ensure EGL will give us a working context (lp:1549455).
        static const bool isMesa = QString(eglQueryString(mEglDisplay, EGL_VENDOR)).contains(QStringLiteral("Mesa"));
        if (isMesa) {
            qCDebug(ubuntumirclient, "Attempting to choose OpenGL 1.4 context which may suit Mesa");
            format.setMajorVersion(1);
            format.setMinorVersion(4);
            delete platformContext;
            platformContext = new UbuntuOpenGLContext(format, context->shareHandle(), mEglDisplay);
        }
    }
    return platformContext;
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

    qCDebug(ubuntumirclient) << "Removing Screen with id" << screen->mirOutputId() << "and geometry" << screen->geometry();
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
    delete screen;
#else
    QPlatformIntegration::destroyScreen(screen);
#endif
}
