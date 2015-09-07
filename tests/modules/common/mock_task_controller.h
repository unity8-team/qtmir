/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#ifndef MOCK_TASKCONTROLLER_H
#define MOCK_TASKCONTROLLER_H

#include <Unity/Application/task_controller_interface.h>

#include <core/posix/fork.h>
#include <gmock/gmock.h>

namespace testing
{
struct MockTaskController : public qtmir::TaskControllerInterface
{
    MOCK_CONST_METHOD2(appIdHasProcessId, bool(const QString&, const quint64));
    MOCK_CONST_METHOD1(findDesktopFileForAppId, QFileInfo(const QString&));

    MOCK_METHOD2(start, bool(const QString&, const QStringList&));
    MOCK_METHOD1(stop, bool(const QString&));
    MOCK_METHOD1(suspend, bool(const QString&));
    MOCK_METHOD1(resume, bool(const QString&));

    MockTaskController()
    : TaskControllerInterface(nullptr)
    {
        using namespace ::testing;

        ON_CALL(*this, appIdHasProcessId(_, _))
                .WillByDefault(
                    Invoke(this, &MockTaskController::doAppIdHasProcessId));

        ON_CALL(*this, findDesktopFileForAppId(_))
                .WillByDefault(
                    Invoke(this, &MockTaskController::doFindDesktopFileForAppId));

        ON_CALL(*this, start(_, _))
                .WillByDefault(
                    Invoke(this, &MockTaskController::doStart));

        ON_CALL(*this, stop(_))
                .WillByDefault(
                    Invoke(this, &MockTaskController::doStop));

        ON_CALL(*this, suspend(_))
                .WillByDefault(
                    Invoke(this, &MockTaskController::doSuspend));

        ON_CALL(*this, resume(_))
                .WillByDefault(
                    Invoke(this, &MockTaskController::doResume));
    }

    bool doAppIdHasProcessId(const QString& appId, const pid_t pid)
    {
        auto it = children.find(appId);
        if (it == children.end())
            return false;

        return it->pid() == pid;
    }

    QFileInfo doFindDesktopFileForAppId(const QString& appId) const
    {
        QString path = QString("/usr/share/applications/%1.desktop").arg(appId);
        return QFileInfo(path);
    }

    bool doStart(const QString& appId, const QStringList& args)
    {
        Q_UNUSED(args);

        auto child = core::posix::fork([]()
        {
            while (true);

            return core::posix::exit::Status::success;
        }, core::posix::StandardStream::empty);

        if (child.pid() > 0)
        {
            children.insert(appId, child);
            Q_EMIT processStarting(appId);
            return true;
        }
        return false;
    }

    bool doStop(const QString& appId)
    {
        auto it = children.find(appId);
        if (it == children.end())
            return false;

        children.remove(appId);

        Q_EMIT processStopped(appId);
        return true;
    }

    bool doSuspend(const QString& appId)
    {            
        auto it = children.find(appId);
        if (it == children.end()) {
            return false;
        }

        Q_EMIT processSuspended(appId);
        return true;
    }

    bool doResume(const QString& appId)
    {
        auto it = children.find(appId);
        if (it == children.end())
            return false;

        Q_EMIT resumeRequested(appId);
        return true;
    }

    QMap<QString, core::posix::ChildProcess> children;
};

} // namespace testing

#endif // MOCK_TASKCONTROLLER_H
