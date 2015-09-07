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

#ifndef TASKCONTROLLER_H
#define TASKCONTROLLER_H

#include "task_controller_interface.h"

#include "application.h"
#include "applicationcontroller.h"

namespace qtmir
{

class TaskController : public TaskControllerInterface
{
    Q_OBJECT
public:
    TaskController(
            QObject *parent,
            const QSharedPointer<ApplicationController> &appController);
    ~TaskController();

    bool start(const QString &appId, const QStringList &args) override;
    bool stop(const QString &appId) override;

    bool suspend(const QString &appId) override;
    bool resume(const QString &appId) override;

    bool appIdHasProcessId(const QString &appId, const quint64 pid) const override;
    QFileInfo findDesktopFileForAppId(const QString &appId) const override;

private Q_SLOTS:
    void onApplicationError(const QString &id, ApplicationController::Error error);

private:
    const QSharedPointer<ApplicationController> m_appController;
};

} // namespace qtmir

#endif // TASKCONTROLLER_H
