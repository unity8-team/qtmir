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

// local
#include "application_manager.h"
#include "application.h"
#include "desktopfilereader.h"
#include "dbuswindowstack.h"

// QPA mirserver
#include "mirserverconfiguration.h"
#include "nativeinterface.h"
#include "sessionlistener.h"
#include "sessionauthorizer.h"
#include "taskcontroller.h"
#include "logging.h"

// mir
#include <mir/scene/surface.h>
#include <mir/scene/session.h>
#include <mir/shell/focus_controller.h>

// Qt
#include <QGuiApplication>

Q_LOGGING_CATEGORY(QTMIR_APPLICATIONS, "qtmir.applications")

namespace ms = mir::scene;

using namespace unity::shell::application;

ApplicationManager *ApplicationManager::the_application_manager = nullptr;

ApplicationManager* ApplicationManager::singleton()
{
    if (!the_application_manager) {
        the_application_manager = new ApplicationManager();
    }
    return the_application_manager;
}

ApplicationManager::ApplicationManager(QObject *parent)
:   ApplicationManagerInterface(parent)
,   m_lifecycleExceptions(QStringList() << "com.ubuntu.music")
,   m_taskController(TaskController::singleton())
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::ApplicationManager (this=%p)" << this;

    NativeInterface *nativeInterface = dynamic_cast<NativeInterface*>(QGuiApplication::platformNativeInterface());

    m_mirConfig = nativeInterface->m_mirConfig;

    if (!nativeInterface) {
        qCritical() << "ERROR: Unity.Application QML plugin requires use of the 'mirserver' QPA plugin";
        QGuiApplication::quit();
        return;
    }

    SessionListener *sessionListener = static_cast<SessionListener*>(nativeInterface->nativeResourceForIntegration("SessionListener"));
    SessionAuthorizer *sessionAuthorizer = static_cast<SessionAuthorizer*>(nativeInterface->nativeResourceForIntegration("SessionAuthorizer"));

    QObject::connect(sessionListener, &SessionListener::sessionStarting,
                     this, &ApplicationManager::onSessionStarting);
    QObject::connect(sessionListener, &SessionListener::sessionStopping,
                     this, &ApplicationManager::onSessionStopping);
    QObject::connect(sessionListener, &SessionListener::sessionCreatedSurface,
                     this, &ApplicationManager::onSessionCreatedSurface);
    QObject::connect(sessionAuthorizer, &SessionAuthorizer::requestAuthorizationForSession,
                     this, &ApplicationManager::authorizeSession, Qt::BlockingQueuedConnection);

    QObject::connect(m_taskController.data(), &TaskController::processStartReport,
                     this, &ApplicationManager::onProcessStartReportReceived);
    QObject::connect(m_taskController.data(), &TaskController::processStopped,
                     this, &ApplicationManager::onProcessStopped);
    QObject::connect(m_taskController.data(), &TaskController::requestFocus,
                     this, &ApplicationManager::onFocusRequested);
    QObject::connect(m_taskController.data(), &TaskController::requestResume,
                     this, &ApplicationManager::onResumeRequested);

    m_dbusWindowStack = new DBusWindowStack(this);
}

ApplicationManager::~ApplicationManager()
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::~ApplicationManager";
}

int ApplicationManager::rowCount(const QModelIndex &parent) const
{
    return !parent.isValid() ? m_applications.size() : 0;
}

QVariant ApplicationManager::data(const QModelIndex &index, int role) const
{
    if (index.row() >= 0 && index.row() < m_applications.size()) {
        Application *application = m_applications.at(index.row());
        switch (role) {
            case RoleAppId:
                return QVariant::fromValue(application->appId());
            case RoleName:
                return QVariant::fromValue(application->name());
            case RoleComment:
                return QVariant::fromValue(application->comment());
            case RoleIcon:
                return QVariant::fromValue(application->icon());
            case RoleStage:
                return QVariant::fromValue((int)application->stage());
            case RoleState:
                return QVariant::fromValue((int)application->state());
            case RoleFocused:
                return QVariant::fromValue(application->focused());
            default:
                return QVariant();
        }
    } else {
        return QVariant();
    }
}

Application* ApplicationManager::get(int index) const
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::get - index=" << index  << "count=" << m_applications.count();
    if (index < 0 || index >= m_applications.count()) {
        return nullptr;
    }
    return m_applications.at(index);
}

Application* ApplicationManager::findApplication(const QString &appId) const
{
    for (Application *app : m_applications) {
        if (app->appId() == appId) {
            return app;
        }
    }
    return nullptr;
}

QString ApplicationManager::focusedApplicationId() const
{
    return m_focusedApplicationId;
}

bool ApplicationManager::suspended() const
{
    return m_suspended;
}

void ApplicationManager::setSuspended(bool suspended)
{
    if (suspended == m_suspended) {
        return;
    }
    m_suspended = suspended;
    Q_EMIT suspendedChanged();

    if (m_suspended) {
        // TODO - save state of all apps, then suspend all
    } else {
        // TODO - restore state
    }
}

bool ApplicationManager::suspendApplication(const QString &appId)
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::suspendApplication - appId=" << appId;
    Application *application = findApplication(appId);
    if (application == nullptr)
        return false;

    // If present in exceptions list, do nothing and just return true.
    if (!m_lifecycleExceptions.filter(application->appId().section('_',0,0)).empty())
        return true;

    if (application->state() == Application::Running)
        application->setState(Application::Suspended);

    return true;
}

bool ApplicationManager::resumeApplication(const QString &appId)
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::resumeApplication - appId=" << appId;
    Application *application = findApplication(appId);

    if (application == nullptr)
        return false;

    if (application->state() != Application::Running)
        application->setState(Application::Running);

    return true;
}


Application* ApplicationManager::startApplication(const QString &appId,
                                                  const QStringList &arguments)
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::startApplication - this=" << this << "appId" << qPrintable(appId);

    if (!m_taskController->start(appId, arguments)) {
        qCritical() << "Upstart failed to start application with appId" << appId;
        return nullptr;
    }

    Application* application = new Application(appId, Application::Starting, arguments, this);
    if (!application->isValid()) {
        qCritical() << "Unable to instantiate application with appId" << appId;
        return nullptr;
    }

    add(application);
    return application;
}

void ApplicationManager::onProcessStartReportReceived(const QString &appId, const bool failure)
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::onProcessStartReportReceived - appId=" << appId << "failure=" << failure;

    if (failure) {
        onProcessStopped(appId, true);
    }

    Application *application = findApplication(appId);

    if (!application) { // if shell did not start this application, but upstart did
        application = new Application(appId, Application::Starting, QStringList(), this);
        if (!application->isValid()) {
            qCritical() << "Unable to instantiate application with appId" << appId;
            return;
        }
        add(application);
    }
}

bool ApplicationManager::stopApplication(const QString &appId)
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::stopApplication - appId=" << appId;

    Application *application = findApplication(appId);

    if (!application) {
        qCritical() << "No such running application with appId" << appId;
        return false;
    }

    remove(application);
    m_dbusWindowStack->WindowDestroyed(0, appId);

    bool result = m_taskController->stop(appId);

    if (!result) {
        qCritical() << "FAILED to ask Upstart to stop application with appId" << appId;
    }
    delete application;

    // FIXME(dandrader): lying here. The operation is async. So we will only know whether
    // the focusing was successful once the server replies. Maybe the API in unity-api should
    // reflect that?
    return result;
}

void ApplicationManager::onProcessStopped(const QString &appId, const bool unexpected)
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::onProcessStopped - appId=" << appId;
    Application *application = findApplication(appId);

    // if shell did not stop the application, but upstart says it died, we assume the process has been
    // killed, so it can be respawned later. Only exception is if that application is focused or running
    // as then it most likely crashed. Update this logic when upstart gives some failure info.
    if (application) {
        bool removeApplication = false;

        if (application->state() == Application::Running || application->state() == Application::Starting) {
            // Application probably crashed, else OOM killer struck. Either way state wasn't saved
            // so just remove application
            removeApplication = true;
        } else if (application->state() == Application::Suspended) {
            application->setState(Application::Stopped);
            application->setSession(nullptr);
        }

        if (removeApplication) {
            remove(application);
            m_dbusWindowStack->WindowDestroyed(0, application->appId());
            delete application;
        }
    }

    if (unexpected) {
        // TODO: pop up a message box/notification?
        qCritical() << "ApplicationManager::onProcessStopped - app with appId" << appId << "died unexpectedly";
    }
}

void ApplicationManager::onFocusRequested(const QString& appId)
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::onFocusRequested - appId=" << appId;

    Q_EMIT focusRequested(appId);
}

void ApplicationManager::onResumeRequested(const QString& appId)
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::onResumeRequested - appId=" << appId;

    Application *application = findApplication(appId);

    if (!application) {
        qCritical() << "ApplicationManager::onResumeRequested: No such running application" << appId;
        return;
    }

    // If app Stopped, trust that upstart-app-launch respawns it itself, and AppManager will
    // be notified of that through the onProcessStartReportReceived slot. Else resume.
    if (application->state() == Application::Suspended) {
        application->setState(Application::Running);
    }
}


/************************************* Mir-side methods *************************************/

void ApplicationManager::authorizeSession(const quint64 pid, bool &authorized)
{
    authorized = false; //to be proven wrong

    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::authorizeSession - pid=" << pid;

    for (Application *app : m_applications) {
        if (app->state() == Application::Starting
                && m_taskController->appIdHasProcessId(app->appId(), pid)) {
            app->setPid(pid);
            authorized = true;
            return;
        }
    }

    /*
     * Hack: Allow applications to be launched externally, but must be executed with the
     * "desktop_file_hint" parameter attached. This exists until upstart-app-launch can
     * notify shell it is starting an application and so shell should allow it. Also reads
     * the --stage parameter to determine the desired stage
     */
    QFile cmdline(QString("/proc/%1/cmdline").arg(pid));
    if (!cmdline.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "ApplicationManager REJECTED connection from app with pid" << pid
                    << "as unable to read the process command line";
        return;
    }

    QByteArray command = cmdline.readLine().replace('\0', ' ');

    // FIXME: special exception for the OSK - maliit-server - not very secure
    if (command.startsWith("maliit-server") || command.startsWith("/usr/lib/arm-linux-gnueabihf/qt5/libexec/QtWebProcess")
        || command.startsWith("/usr/bin/signon-ui")) {
        authorized = true;
        return;
    }

    QString pattern = QRegularExpression::escape("--desktop_file_hint=") + "(\\S+)";
    QRegularExpression regExp(pattern);
    QRegularExpressionMatch regExpMatch = regExp.match(command);

    if (!regExpMatch.hasMatch()) {
        qCritical() << "ApplicationManager REJECTED connection from app with pid" << pid
                    << "as no desktop_file_hint specified";
        return;
    }

    QString desktopFileName = regExpMatch.captured(1);
    qCDebug(QTMIR_APPLICATIONS) << "Process supplied desktop_file_hint, loading" << desktopFileName;

    // FIXME: right now we support --desktop_file_hint=appId for historical reasons. So let's try that in
    // case we didn't get an existing .desktop file path
    DesktopFileReader* desktopData;
    if (QFileInfo(desktopFileName).exists()) {
        desktopData = new DesktopFileReader(QFileInfo(desktopFileName));
    } else {
        desktopData = new DesktopFileReader(desktopFileName);
    }

    if (!desktopData->loaded()) {
        delete desktopData;
        qCritical() << "ApplicationManager REJECTED connection from app with pid" << pid
                    << "as the file specified by the desktop_file_hint argument could not be opened";
        return;
    }

    // some naughty applications use a script to launch the actual application. Check for the
    // case where shell actually launched the script.
    Application *application = findApplication(desktopData->appId());
    if (application && application->state() == Application::Starting) {
        qCDebug(QTMIR_APPLICATIONS) << "Process with pid" << pid << "appeared, attaching to existing entry"
                                    << "in application list with appId:" << application->appId();
        delete desktopData;
        application->setSessionName(application->appId());
        application->setPid(pid);
        authorized = true;
        return;
    }

    // if stage supplied in CLI, fetch that
    Application::Stage stage = Application::MainStage;
    pattern = QRegularExpression::escape("--stage=") + "(\\S+)";
    regExp.setPattern(pattern);
    regExpMatch = regExp.match(command);

    if (regExpMatch.hasMatch() && regExpMatch.captured(1) == "side_stage") {
        stage = Application::SideStage;
    }

    qCDebug(QTMIR_APPLICATIONS) << "New process with pid" << pid << "appeared, adding new application to the"
                                << "application list with appId:" << desktopData->appId();

    QString argStr(command.data());
    QStringList arguments(argStr.split(' '));
    application = new Application(desktopData, Application::Starting, arguments, this);
    application->setPid(pid);
    application->setStage(stage);
    add(application);
    authorized = true;
}

void ApplicationManager::onSessionStarting(std::shared_ptr<ms::Session> const& session)
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::onSessionStarting - sessionName=" <<  session->name().c_str();

    Application* application = findApplicationWithPid(session->process_id());
    if (application && application->state() != Application::Running) {
        application->setSession(session);
    } else {
        qCritical() << "ApplicationManager::onSessionStarting - unauthorized application!!";
    }
}

void ApplicationManager::onSessionStopping(std::shared_ptr<ms::Session> const& session)
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::onSessionStopping - sessionName=" << session->name().c_str();

    // in case application closed not by hand of shell, check again here:
    Application* application = findApplicationWithSession(session);
    if (application) {
        bool removeApplication = true;

        if (application->state() != Application::Starting) {
            application->setState(Application::Stopped);
            application->setSession(nullptr);
            m_dbusWindowStack->WindowDestroyed(0, application->appId());
            // TODO - decide if app dying can be masked from user or not
        }

        if (removeApplication) {
            remove(application);
            delete application;
            Q_EMIT focusedApplicationIdChanged();
        }
    }
}

void ApplicationManager::onSessionCreatedSurface(ms::Session const* session,
                                               std::shared_ptr<ms::Surface> const& surface)
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::onSessionCreatedSurface - sessionName=" << session->name().c_str();
    Q_UNUSED(surface);

    Application* application = findApplicationWithSession(session);
    if (application && application->state() == Application::Starting) {
        m_dbusWindowStack->WindowCreated(0, application->appId()); //FIXME(greyback) - SurfaceManager should do this
    }
}

void ApplicationManager::setFocused(Application *application)
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::setFocused - appId=" << application->appId();

    m_focusedApplicationId = application->appId();
    move(m_applications.indexOf(application), 0);
    Q_EMIT focusedApplicationIdChanged();
    m_dbusWindowStack->FocusedWindowChanged(0, application->appId(), application->stage()); //FIXME(greyback) - SurfaceManager should do this
}

Application* ApplicationManager::findApplicationWithSession(const std::shared_ptr<ms::Session> &session)
{
    return findApplicationWithSession(session.get());
}

Application* ApplicationManager::findApplicationWithSession(const ms::Session *session)
{
    for (Application *app : m_applications) {
        if (app->session().get() == session) {
            return app;
        }
    }
    return nullptr;
}

Application* ApplicationManager::findApplicationWithPid(const qint64 pid)
{
    if (pid <= 0)
        return nullptr;

    for (Application *app : m_applications) {
        if (app->m_pid == pid) {
            return app;
        }
    }
    return nullptr;
}

void ApplicationManager::add(Application* application)
{
    Q_ASSERT(application != NULL);
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::add - appId=" << application->appId();

    beginInsertRows(QModelIndex(), m_applications.size(), m_applications.size());
    m_applications.append(application);
    endInsertRows();
    Q_EMIT countChanged();
    Q_EMIT applicationAdded(application->appId());
}

void ApplicationManager::remove(Application *application)
{
    Q_ASSERT(application != NULL);
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::remove - appId=" << application->appId();

    int i = m_applications.indexOf(application);
    if (i != -1) {
        beginRemoveRows(QModelIndex(), i, i);
        m_applications.removeAt(i);
        endRemoveRows();
        Q_EMIT applicationRemoved(application->appId());
        Q_EMIT countChanged();
    }
}

void ApplicationManager::move(const int from, const int to)
{
    qCDebug(QTMIR_APPLICATIONS) << "ApplicationManager::move - from=" << from << "to=" << to;
    if (from == to) return;

    if (from >= 0 && from < m_applications.size() && to >= 0 && to < m_applications.size()) {
        QModelIndex parent;
        /* When moving an item down, the destination index needs to be incremented
           by one, as explained in the documentation:
           http://qt-project.org/doc/qt-5.0/qtcore/qabstractitemmodel.html#beginMoveRows */
        beginMoveRows(parent, from, from, parent, to + (to > from ? 1 : 0));
        m_applications.move(from, to);
        endMoveRows();
    }
}

QList<Application*> ApplicationManager::list() const
{
    return m_applications;
}
