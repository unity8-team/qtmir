/*
 * Copyright (C) 2013-2014 Canonical, Ltd.
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
#include "application.h"
#include "application_manager.h"
#include "desktopfilereader.h"
#include "taskcontroller.h"

// QPA mirserver
#include "logging.h"

// mir
#include <mir/scene/session.h>
#include <mir/scene/snapshot.h>

namespace qtmir
{

Application::Application(const QSharedPointer<TaskController>& taskController,
                         DesktopFileReader *desktopFileReader,
                         State state,
                         const QStringList &arguments,
                         QObject *parent)
    : ApplicationInfoInterface(desktopFileReader->appId(), parent)
    , m_taskController(taskController)
    , m_desktopData(desktopFileReader)
    , m_pid(0)
    , m_stage((m_desktopData->stageHint() == "SideStage") ? Application::SideStage : Application::MainStage)
    , m_state(state)
    , m_focused(false)
    , m_canBeResumed(true)
    , m_fullscreen(false)
    , m_arguments(arguments)
    , m_suspendTimer(new QTimer(this))
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::Application - appId=" << desktopFileReader->appId() << "state=" << state;

    m_suspendTimer->setSingleShot(true);
    connect(m_suspendTimer, SIGNAL(timeout()), this, SLOT(suspend()));

    // FIXME(greyback) need to save long appId internally until upstart-app-launch can hide it from us
    m_longAppId = desktopFileReader->file().remove(QRegExp(".desktop$")).split('/').last();
}

Application::~Application()
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::~Application";
    delete m_desktopData;
}

bool Application::isValid() const
{
    return m_desktopData->loaded();
}

QString Application::desktopFile() const
{
    return m_desktopData->file();
}

QString Application::appId() const
{
    return m_desktopData->appId();
}

QString Application::name() const
{
    return m_desktopData->name();
}

QString Application::comment() const
{
    return m_desktopData->comment();
}

QUrl Application::icon() const
{
    QString iconString = m_desktopData->icon();
    QString pathString = m_desktopData->path();

    if (QFileInfo(iconString).exists()) {
        return QUrl(iconString);
    } else if (QFileInfo(pathString + '/' + iconString).exists()) {
        return QUrl(pathString + '/' + iconString);
    } else {
        return QUrl("image://theme/" + iconString);
    }
}

QString Application::exec() const
{
    return m_desktopData->exec();
}

Application::Stage Application::stage() const
{
    return m_stage;
}

Application::Stages Application::supportedStages() const
{
    return m_supportedStages;
}

Application::State Application::state() const
{
    return m_state;
}

bool Application::focused() const
{
    return m_focused;
}

QUrl Application::screenshot() const
{
    return m_screenshot;
}

bool Application::fullscreen() const
{
    return m_fullscreen;
}

std::shared_ptr<mir::scene::Session> Application::session() const
{
    return m_session;
}

bool Application::canBeResumed() const
{
    return m_canBeResumed;
}

void Application::setCanBeResumed(const bool resume)
{
    m_canBeResumed = resume;
}

pid_t Application::pid() const
{
    return m_pid;
}

void Application::setPid(pid_t pid)
{
    m_pid = pid;
}

void Application::setSession(const std::shared_ptr<mir::scene::Session>& session)
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::setSession - appId=" << appId() << "session=" << session.get();

    // TODO(greyback) what if called with new surface?
    m_session = session;
}

void Application::setSessionName(const QString& name)
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::setSessionName - appId=" << appId() << "name=" << name;
    if (m_session) {
        qCritical() << "Application::setSessionName should not be called once session exists";
        return;
    }
    m_sessionName = name;
}

bool Application::setStage(const Application::Stage stage)
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::setStage - appId=" << appId() << "stage=" << stage;

    if (m_stage != stage) {
        if (stage | m_supportedStages) {
            return false;
        }

        m_stage = stage;
        Q_EMIT stageChanged(stage);
        return true;
    }
    return true;
}

QImage Application::screenshotImage() const
{
    return m_screenshotImage;
}

void Application::updateScreenshot()
{
    session()->take_snapshot(
        [&](mir::scene::Snapshot const& snapshot)
        {
            qCDebug(QTMIR_APPLICATIONS) << "ApplicationScreenshotProvider - Mir snapshot ready with size"
                                        << snapshot.size.height.as_int() << "x" << snapshot.size.width.as_int();

            m_screenshotImage = QImage( (const uchar*)snapshot.pixels, // since we mirror, no need to offset starting position
                            snapshot.size.width.as_int(),
                            snapshot.size.height.as_int(),
                            QImage::Format_ARGB32_Premultiplied).mirrored();

            m_screenshot = QString("image://application/%1/%2").arg(m_desktopData->appId()).arg(QDateTime::currentMSecsSinceEpoch());
            Q_EMIT screenshotChanged(m_screenshot);
        });
}

void Application::setState(Application::State state)
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::setState - appId=" << appId() << "state=" << state;
    if (m_state != state) {
        switch (state)
        {
        case Application::Suspended:
            if (m_state == Application::Running) {
                session()->set_lifecycle_state(mir_lifecycle_state_will_suspend);
                m_suspendTimer->start(3000);
            }
            break;
        case Application::Running:
            if (m_suspendTimer->isActive())
                m_suspendTimer->stop();

            if (m_state == Application::Suspended) {
                resume();
                session()->set_lifecycle_state(mir_lifecycle_state_resumed);
            } else if (m_state == Application::Stopped) {
                respawn();
                state = Application::Starting;
            }
            break;
        case Application::Stopped:
            if (m_suspendTimer->isActive())
                m_suspendTimer->stop();
            break;
        default:
            break;
        }
        m_state = state;
        Q_EMIT stateChanged(state);
    }
}

void Application::setFocused(bool focused)
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::setFocused - appId=" << appId() << "focused=" << focused;
    if (m_focused != focused) {
        m_focused = focused;
        Q_EMIT focusedChanged(focused);
    }
}

void Application::setFullscreen(bool fullscreen)
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::setFullscreen - appId=" << appId() << "fullscreen=" << fullscreen;
    if (m_fullscreen != fullscreen) {
        m_fullscreen = fullscreen;
        Q_EMIT fullscreenChanged();
    }
}

void Application::suspend()
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::suspend - appId=" << appId();
    m_taskController->suspend(longAppId());
}

void Application::resume()
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::resume - appId=" << appId();
    m_taskController->resume(longAppId());
}

void Application::respawn()
{
    qCDebug(QTMIR_APPLICATIONS) << "Application::respawn - appId=" << appId();
    m_taskController->start(appId(), m_arguments);
}

QString Application::longAppId() const
{
    return m_longAppId;
}

} // namespace qtmir
