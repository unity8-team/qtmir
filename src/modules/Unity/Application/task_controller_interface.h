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

#ifndef TASKCONTROLLER_INTERFACE_H
#define TASKCONTROLLER_INTERFACE_H

#include <QObject>
#include <QFileInfo>

namespace qtmir
{

class TaskControllerInterface : public QObject
{
    Q_OBJECT
public:
    TaskControllerInterface(QObject *parent = 0) : QObject(parent) {}
    virtual ~TaskControllerInterface() {}
    
    virtual bool start(const QString &appId, const QStringList &args) = 0;
    virtual bool stop(const QString &appId) = 0;

    virtual bool suspend(const QString &appId) = 0;
    virtual bool resume(const QString &appId) = 0;

    virtual bool appIdHasProcessId(const QString &appId, const quint64 pid) const = 0;
    virtual QFileInfo findDesktopFileForAppId(const QString &appId) const = 0;

Q_SIGNALS:
    void processStarting(const QString &appId);
    void processStopped(const QString &appId);
    void processSuspended(const QString &appId);
    void processFailed(const QString &appId, const bool duringStartup);
    void focusRequested(const QString &appId);
    void resumeRequested(const QString &appId);
};

} // namespace qtmir

#endif // TASKCONTROLLER_INTERFACE_H
