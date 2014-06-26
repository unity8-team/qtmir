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

#ifndef APPLICATIONMANAGER_H
#define APPLICATIONMANAGER_H

// std
#include <memory>

// Qt
#include <QObject>
#include <QStringList>

// Unity API
#include <unity/shell/application/ApplicationManagerInterface.h>

// local
#include "application.h"
#include "desktopfilereader.h"

namespace mir {
    namespace scene {
        class Session;
        class Surface;
    }
}

class MirServerConfiguration;
class QJSEngine;

namespace qtmir
{

class DBusWindowStack;
class MirSurfaceManager;
class ProcInfo;
class TaskController;

class ApplicationManager : public unity::shell::application::ApplicationManagerInterface
{
    Q_OBJECT
    Q_ENUMS(MoreRoles)

public:
    class Factory
    {
    public:
        ApplicationManager* create(QJSEngine *jsEngine);
    };

    enum MoreRoles {
        RoleSurface = RoleScreenshot+1,
        RoleFullscreen,
    };

    static ApplicationManager* singleton(QJSEngine *jsEngine);

    explicit ApplicationManager(
            const QSharedPointer<MirServerConfiguration> &mirConfig,
            const QSharedPointer<TaskController> &taskController,
            const QSharedPointer<DesktopFileReader::Factory> &desktopFileReaderFactory,
            const QSharedPointer<ProcInfo> &processInfo,
            QJSEngine *jsEngine,
            QObject *parent = 0);
    virtual ~ApplicationManager();

    // ApplicationManagerInterface
    Q_INVOKABLE Application* startApplication(const QString &appId, const QStringList &arguments) override;
    Q_INVOKABLE bool stopApplication(const QString &appId) override;
    Q_INVOKABLE bool suspendApplication(const QString &appId) override;
    Q_INVOKABLE bool resumeApplication(const QString &appId) override;

    Q_INVOKABLE bool updateScreenshot(const QString &appId) override;

    Q_INVOKABLE Application* get(int index) const override;
    Q_INVOKABLE Application* findApplication(const QString &appId) const override;

    Q_INVOKABLE void registerSurfaceSizer(const QJSValue slot);
    Q_INVOKABLE void deregisterSurfaceSizer();

    Q_INVOKABLE bool moveToFront(const QString &appId);

    QString focusedApplicationId() const override;
    bool suspended() const override;
    void setSuspended(bool suspended) override;

    // QAbstractListModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public Q_SLOTS:
    void authorizeSession(const quint64 pid, bool &authorized);
    void determineSizeForNewSurface(mir::scene::Session const *session, QSize &size);

    void onSessionStarting(const std::shared_ptr<mir::scene::Session> &session);
    void onSessionStopping(const std::shared_ptr<mir::scene::Session> &session);

    void onSessionCreatedSurface(const mir::scene::Session *session,
                                 const std::shared_ptr<mir::scene::Surface> &surface);

    void onProcessFailed(const QString &appId, const bool duringStartup);
    void onProcessStarting(const QString &appId);
    void onProcessStopped(const QString &appId);
    void onFocusRequested(const QString &appId);
    void onResumeRequested(const QString &appId);

Q_SIGNALS:
    void focusRequested(const QString &appId);
    void topmostApplicationChanged(Application *application);
    void emptyChanged();

private Q_SLOTS:
    void screenshotUpdated();

private:
    void setFocused(Application *application);
    void add(Application *application);
    void remove(Application *application);
    void move(const int from, const int to);
    QList<Application*> list() const;
    Application* findApplicationWithSession(const std::shared_ptr<mir::scene::Session> &session) const;
    Application* findApplicationWithSession(const mir::scene::Session *session) const;
    Application* findApplicationWithPid(const qint64 pid) const;
    QModelIndex findIndex(const Application *application) const;

    void suspendApplication(Application *application);
    void resumeApplication(Application *application);

    QSharedPointer<MirServerConfiguration> m_mirConfig;

    QList<Application*> m_applications;
    QString m_focusedApplicationId;
    QStringList m_lifecycleExceptions;
    DBusWindowStack *m_dbusWindowStack;
    QSharedPointer<TaskController> m_taskController;
    QSharedPointer<DesktopFileReader::Factory> m_desktopFileReaderFactory;
    QSharedPointer<ProcInfo> m_procInfo;
    QJSValue m_surfaceSizer;
    QJSEngine *m_jsEngine;
    static ApplicationManager *the_application_manager;
    bool m_suspended;

    friend class Application;
    friend class DBusWindowStack;
    friend class MirSurfaceManager;
};

} // namespace qtmir

Q_DECLARE_METATYPE(qtmir::ApplicationManager*)

#endif // APPLICATIONMANAGER_H
