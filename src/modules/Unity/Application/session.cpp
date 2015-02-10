/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// local
#include "application.h"
#include "debughelpers.h"
#include "session.h"
#include "mirsurfacemanager.h"
#include "mirsurfaceitem.h"

// mirserver
#include "logging.h"

// mir
#include <mir/scene/session.h>
#include <mir/scene/prompt_session.h>
#include <mir/scene/prompt_session_manager.h>

// Qt
#include <QPainter>
#include <QQmlEngine>

namespace ms = mir::scene;

using unity::shell::application::ApplicationInfoInterface;

namespace qtmir
{

Session::Session(const std::shared_ptr<ms::Session>& session,
                 const std::shared_ptr<ms::PromptSessionManager>& promptSessionManager,
                 QObject *parent)
    : SessionInterface(parent)
    , m_session(session)
    , m_application(nullptr)
    , m_parentSession(nullptr)
    , m_children(new SessionModel(this))
    , m_fullscreen(false)
    , m_state(State::Starting)
    , m_live(true)
    , m_suspendTimer(new QTimer(this))
    , m_promptSessionManager(promptSessionManager)
{
    qCDebug(QTMIR_SESSIONS) << "Session::Session() " << this->name();

    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    m_suspendTimer->setSingleShot(true);
    connect(m_suspendTimer, &QTimer::timeout, this, [this]() {
//        if (m_surface) {
//            m_surface->stopFrameDropper();
//        } else {
//            qDebug() << "Application::suspend - no surface to call stopFrameDropper() on!";
//        }
        Q_EMIT suspended();
    });
}

Session::~Session()
{
    qCDebug(QTMIR_SESSIONS) << "Session::~Session() " << name();
    stopPromptSessions();

    QList<SessionInterface*> children(m_children->list());
    for (SessionInterface* child : children) {
        delete child;
    }
    if (m_parentSession) {
        m_parentSession->removeChildSession(this);
    }
    if (m_application) {
        m_application->setSession(nullptr);
    }
    for (auto surface : m_surfaces) { // GERRY - no animation??
        delete surface;
    }
    delete m_children; m_children = nullptr;
}

void Session::release()
{
    qCDebug(QTMIR_SESSIONS) << "Session::release " << name();
    Q_EMIT aboutToBeDestroyed();

    if (m_parentSession) {
        m_parentSession->removeChildSession(this);
    }
    if (m_application) {
        m_application->setSession(nullptr);
    }
    if (!parent()) {
        deleteLater();
    }
}

QString Session::name() const
{
    return QString::fromStdString(m_session->name());
}

std::shared_ptr<ms::Session> Session::session() const
{
    return m_session;
}

ApplicationInfoInterface* Session::application() const
{
    return m_application;
}

QQmlListProperty<qtmir::MirSurfaceItem> Session::surfaces()
{
    return QQmlListProperty<MirSurfaceItem>(this, 0,
            [] (QQmlListProperty<MirSurfaceItem> *list) -> int // count function
            {
                auto _this = qobject_cast<Session *>(list->object);
                return _this->m_surfaces.count();
            },
            [] (QQmlListProperty<MirSurfaceItem> *list, int at) -> MirSurfaceItem* // at function
            {
                auto _this = qobject_cast<Session *>(list->object);
                return _this->m_surfaces.at(at);
            }
    );

    // Only notify QML of surface creation once it has drawn its first frame.
}

SessionInterface* Session::parentSession() const
{
    return m_parentSession;
}

Session::State Session::state() const
{
    return m_state;
}

bool Session::fullscreen() const
{
    return m_fullscreen;
}

bool Session::live() const
{
    return m_live;
}

void Session::setApplication(ApplicationInfoInterface* application)
{
    if (m_application == application)
        return;

    m_application = static_cast<Application*>(application);
    Q_EMIT applicationChanged(application);
}

void Session::addSurface(MirSurfaceItem *newSurface)
{
    if (!newSurface || m_surfaces.contains(newSurface)) {
        return;
    }
    qCDebug(QTMIR_SESSIONS) << "Session::addSurface - session=" << newSurface->name() << "surface=" << newSurface;

    newSurface->setParent(this);
    newSurface->setSession(this);

    if (newSurface->isFirstFrameDrawn()) {
        addSurfaceToModel(newSurface);
        qCDebug(QTMIR_SESSIONS) << "Session::addSurface - added to model";
    } else {
        // Only notify QML of surface creation once it has drawn its first frame.
        connect(newSurface, &MirSurfaceItem::firstFrameDrawn, this,
                [this](MirSurfaceItem *item) {
                    addSurfaceToModel(item);
                    m_notDrawnToSurfaces.removeOne(item);
                    qCDebug(QTMIR_SESSIONS) << "Session::addSurface - added to model after first frame drew";
        });
        m_notDrawnToSurfaces.append(newSurface);
    }

    connect(newSurface, &MirSurfaceItem::stateChanged,
        this, &Session::updateFullscreenProperty);
}

void Session::addSurfaceToModel(MirSurfaceItem *newSurface)
{
    m_surfaces.append(newSurface);

    Q_EMIT surfacesChanged();
    updateFullscreenProperty();
}

void Session::removeSurface(MirSurfaceItem *surface)
{
    qCDebug(QTMIR_SESSIONS) << "Session::removeSurface - session=" << surface->name() << "surface=" << surface;
    if (m_notDrawnToSurfaces.contains(surface)) {
        m_notDrawnToSurfaces.removeOne(surface);
    }

    // when QML calls release() on this MirSurfaceItem, then it will be removed from the list
    connect(surface, &MirSurfaceItem::destroyed,
            [&](QObject *item){
                auto surface = static_cast<MirSurfaceItem *>(item);
                m_surfaces.removeOne(surface);
                Q_EMIT surfacesChanged();
    });
}

void Session::updateFullscreenProperty()
{
    if (!m_surfaces.isEmpty()) {
        setFullscreen(m_surfaces.first()->state() == MirSurfaceItem::Fullscreen);
    } else {
        // Keep the current value of the fullscreen property until we get a new
        // surface
    }
}

void Session::setFullscreen(bool fullscreen)
{
    qCDebug(QTMIR_SESSIONS) << "Session::setFullscreen - session=" << this << "fullscreen=" << fullscreen;
    if (m_fullscreen != fullscreen) {
        m_fullscreen = fullscreen;
        Q_EMIT fullscreenChanged(m_fullscreen);
    }
}

void Session::setState(State state)
{
    qCDebug(QTMIR_SESSIONS) << "Session::setState - session=" << this << "state=" << applicationStateToStr(state);
    if (m_state != state) {
        switch (state)
        {
        case Session::State::Suspended:
            if (m_state == Session::State::Running) {
                session()->set_lifecycle_state(mir_lifecycle_state_will_suspend);
                m_suspendTimer->start(1500);
            }
            break;
        case Session::State::Running:
            if (m_suspendTimer->isActive())
                m_suspendTimer->stop();

            if (m_state == Session::State::Suspended) {
//                if (m_surface)
//                    m_surface->startFrameDropper();
                Q_EMIT resumed();
                session()->set_lifecycle_state(mir_lifecycle_state_resumed);
            }
            break;
        case Session::State::Stopped:
            stopPromptSessions();
            if (m_suspendTimer->isActive())
                m_suspendTimer->stop();
//            if (m_surface)
//                m_surface->stopFrameDropper();
            break;
        default:
            break;
        }

        m_state = state;
        Q_EMIT stateChanged(state);

        foreachPromptSession([this, state](const std::shared_ptr<ms::PromptSession>& promptSession) {
            switch (state) {
                case Session::State::Suspended:
                    m_promptSessionManager->suspend_prompt_session(promptSession);
                    break;
                case Session::State::Running:
                    m_promptSessionManager->resume_prompt_session(promptSession);
                    break;
                default:
                    break;
            }
        });

        foreachChildSession([state](SessionInterface* session) {
            session->setState(state);
        });
    }
}

void Session::setLive(const bool live)
{
    if (m_live != live) {
        m_live = live;
        Q_EMIT liveChanged(m_live);
    }
}

void Session::setParentSession(Session* session)
{
    if (m_parentSession == session || session == this)
        return;

    m_parentSession = session;

    Q_EMIT parentSessionChanged(session);
}

void Session::addChildSession(SessionInterface* session)
{
    insertChildSession(m_children->rowCount(), session);
}

void Session::insertChildSession(uint index, SessionInterface* session)
{
    qCDebug(QTMIR_SESSIONS) << "Session::insertChildSession - " << session->name() << " to " << name() << " @  " << index;

    static_cast<Session*>(session)->setParentSession(this);
    m_children->insert(index, session);

    session->setState(state());
}

void Session::removeChildSession(SessionInterface* session)
{
    qCDebug(QTMIR_SESSIONS) << "Session::removeChildSession - " << session->name() << " from " << name();

    if (m_children->contains(session)) {
        m_children->remove(session);
        static_cast<Session*>(session)->setParentSession(nullptr);
    }
}

void Session::foreachChildSession(std::function<void(SessionInterface* session)> f) const
{
    QList<SessionInterface*> children(m_children->list());
    for (SessionInterface* child : children) {
        f(child);
    }
}

SessionModel* Session::childSessions() const
{
    return m_children;
}

void Session::appendPromptSession(const std::shared_ptr<ms::PromptSession>& promptSession)
{
    qCDebug(QTMIR_SESSIONS) << "Session::appendPromptSession session=" << name()
            << "promptSession=" << (promptSession ? promptSession.get() : nullptr);

    m_promptSessions.append(promptSession);
}

void Session::removePromptSession(const std::shared_ptr<ms::PromptSession>& promptSession)
{
    qCDebug(QTMIR_SESSIONS) << "Session::removePromptSession session=" << name()
            << "promptSession=" << (promptSession ? promptSession.get() : nullptr);

    m_promptSessions.removeAll(promptSession);
}

void Session::stopPromptSessions()
{
    QList<SessionInterface*> children(m_children->list());
    for (SessionInterface* child : children) {
        static_cast<Session*>(child)->stopPromptSessions();
    }

    QList<std::shared_ptr<ms::PromptSession>> copy(m_promptSessions);
    QListIterator<std::shared_ptr<ms::PromptSession>> it(copy);
    for ( it.toBack(); it.hasPrevious(); ) {
        std::shared_ptr<ms::PromptSession> promptSession = it.previous();
        qCDebug(QTMIR_SESSIONS) << "Session::stopPromptSessions - promptSession=" << promptSession.get();

        m_promptSessionManager->stop_prompt_session(promptSession);
    }
}

std::shared_ptr<ms::PromptSession> Session::activePromptSession() const
{
    if (m_promptSessions.count() > 0)
        return m_promptSessions.back();
    return nullptr;
}

void Session::foreachPromptSession(std::function<void(const std::shared_ptr<ms::PromptSession>&)> f) const
{
    for (std::shared_ptr<ms::PromptSession> promptSession : m_promptSessions) {
        f(promptSession);
    }
}

} // namespace qtmir
