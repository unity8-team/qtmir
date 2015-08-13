/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
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

#ifndef MIR_SURFACE_MANAGER_H
#define MIR_SURFACE_MANAGER_H

// std
#include <memory>

// Qt
#include <QObject>
#include <QHash>
#include <QMutex>
#include <QSharedPointer>

// Mir
#include <mir_toolkit/common.h>

// local
#include <mirserver.h>

namespace mir {
    namespace scene {
        class Surface;
        class Session;
        class PromptSession;
    }
}

class MirServer;
class SurfaceObserver;

namespace qtmir {

class Application;
class ApplicationManager;
class MirSurfaceInterface;
class SessionManager;

class MirSurfaceManager : public QObject
{
    Q_OBJECT

public:
    explicit MirSurfaceManager(
        const QSharedPointer<MirServer>& mirServer,
        MirShell *shell,
        SessionManager* sessionManager,
        QObject *parent = 0
    );
    ~MirSurfaceManager();

    static MirSurfaceManager* singleton();

Q_SIGNALS:
    void surfaceCreated(MirSurfaceInterface* surface);
    void surfaceDestroyed(MirSurfaceInterface* surface);

public Q_SLOTS:
    void onSessionCreatedSurface(const mir::scene::Session *,
                                 const std::shared_ptr<mir::scene::Surface> &,
                                 std::shared_ptr<SurfaceObserver> const&);
    void onSessionDestroyingSurface(const mir::scene::Session *, const std::shared_ptr<mir::scene::Surface> &);

protected:
    QHash<const mir::scene::Surface *, MirSurfaceInterface *> m_mirSurfaceToQmlSurfaceHash;
    QMutex m_mutex;

private:
    QSharedPointer<MirServer> m_mirServer;
    MirShell *const m_shell;
    SessionManager* m_sessionManager;
    static MirSurfaceManager *instance;
};

} // namespace qtmir

#endif // MIR_SURFACE_MANAGER_H
