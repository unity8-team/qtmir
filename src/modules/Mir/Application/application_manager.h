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

class MirServerConfiguration;
class DBusWindowStack;
class MirSurfaceManager;
class TaskController;

namespace mir {
    namespace scene {
        class Session;
        class Surface;
    }
}

class ApplicationManager : public unity::shell::application::ApplicationManagerInterface
{
    Q_OBJECT

public:
    static ApplicationManager* singleton();

    explicit ApplicationManager(QObject *parent = 0);
    virtual ~ApplicationManager();

    // ApplicationManagerInterface
    Q_INVOKABLE Application* startApplication(const QString &appId, const QStringList &arguments) override;
    Q_INVOKABLE bool stopApplication(const QString &appId) override;
    Q_INVOKABLE bool suspendApplication(const QString &appId) override;
    Q_INVOKABLE bool resumeApplication(const QString &appId) override;

    Q_INVOKABLE Application* get(int index) const override;
    Q_INVOKABLE Application* findApplication(const QString &appId) const override;

    QString focusedApplicationId() const override;
    bool suspended() const override;
    void setSuspended(bool suspended) override;

    // QAbstractListModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public Q_SLOTS:
    void authorizeSession(const quint64 pid, bool &authorized);

    void onSessionStarting(const std::shared_ptr<mir::scene::Session> &session);
    void onSessionStopping(const std::shared_ptr<mir::scene::Session> &session);

    void onSessionCreatedSurface(const mir::scene::Session *session,
                                 const std::shared_ptr<mir::scene::Surface> &surface);

    void onProcessStartReportReceived(const QString &appId, const bool failure);
    void onProcessStopped(const QString& appId, const bool unexpected);
    void onFocusRequested(const QString& appId);
    void onResumeRequested(const QString& appId);

Q_SIGNALS:
    void focusRequested(const QString &appId);

private:
    void setFocused(Application *application);
    void add(Application *application);
    void remove(Application *application);
    void move(const int from, const int to);
    QList<Application*> list() const;
    Application* findApplicationWithSession(const std::shared_ptr<mir::scene::Session> &session);
    Application* findApplicationWithSession(const mir::scene::Session *session);
    Application* findApplicationWithPid(const qint64 pid);

    QList<Application*> m_applications;
    QString m_focusedApplicationId;
    QStringList m_lifecycleExceptions;
    QSharedPointer<MirServerConfiguration> m_mirConfig;
    DBusWindowStack* m_dbusWindowStack;
    QScopedPointer<TaskController> m_taskController;
    static ApplicationManager* the_application_manager;
    bool m_suspended;

    friend class DBusWindowStack;
    friend class MirSurfaceManager;
};

Q_DECLARE_METATYPE(ApplicationManager*)

#endif // APPLICATIONMANAGER_H
