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
 *
 */

#include "applicationinfo.h"
#include "taskcontroller.h"

// qtmir
#include <logging.h>

// Qt
#include <QStandardPaths>

// UAL
#include <ubuntu-app-launch/registry.h>

namespace ual = ubuntu::app_launch;

namespace qtmir
{
namespace upstart
{

namespace {
/**
 * @brief toShortAppIdIfPossible
 * @param appId - any string that you think is an appId
 * @return if a valid appId was input, a shortened appId is returned, else returns the input string unaltered
 */
QString toShortAppIdIfPossible(const std::shared_ptr<ual::Application> &app) {
    const ual::AppID &appId = app->appId();
    if (ual::AppID::valid(std::string(appId))) {
        return QString::fromStdString(appId.package) + "_" + QString::fromStdString(appId.appname);
    } else {
        return QString::fromStdString(std::string(appId));
    }
}

std::shared_ptr<ual::Application> createApp(const QString &inputAppId, std::shared_ptr<ual::Registry> registry)
{
    auto appId = ual::AppID::find(inputAppId.toStdString());
    if (appId.empty()) {
        qCDebug(QTMIR_APPLICATIONS) << "ApplicationController::createApp could not find appId" << inputAppId;
        return {};
    }
    return ual::Application::create(appId, registry);
}

} // namespace

class TaskController::Private : public ual::Registry::Manager
{
public:
    Private() : ual::Registry::Manager() {};

    TaskController *parent = nullptr;
    std::shared_ptr<ual::Registry> registry;
    std::shared_ptr<core::ScopedConnection> startedCallback;
    std::shared_ptr<core::ScopedConnection> stopCallback;
    std::shared_ptr<core::ScopedConnection> resumeCallback;
    std::shared_ptr<core::ScopedConnection> pausedCallback;
    std::shared_ptr<core::ScopedConnection> failureCallback;

    bool focusRequest(std::shared_ptr<ual::Application> app,
                      std::shared_ptr<ual::Application::Instance>) override
    {
        Q_EMIT parent->focusRequested(toShortAppIdIfPossible(app));
        return true;
    }

    bool startingRequest(std::shared_ptr<ual::Application> app,
                         std::shared_ptr<ual::Application::Instance>) override
    {
        Q_EMIT parent->processStarting(toShortAppIdIfPossible(app));
        return true;
    }
};

TaskController::TaskController()
    : qtmir::TaskController(),
      impl(new Private())
{
    impl->parent = this;
    impl->registry = std::make_shared<ual::Registry>();

    impl->registry->setManager(impl.data());

    impl->startedCallback = std::make_shared<core::ScopedConnection>(
        impl->registry->appStarted().connect(
            [this](std::shared_ptr<ual::Application> app,
                   std::shared_ptr<ual::Application::Instance>) {
        Q_EMIT applicationStarted(toShortAppIdIfPossible(app));
    }));

    impl->stopCallback = std::make_shared<core::ScopedConnection>(
        impl->registry->appStopped().connect(
            [this](std::shared_ptr<ual::Application> app,
                   std::shared_ptr<ual::Application::Instance>) {
        Q_EMIT processStopped(toShortAppIdIfPossible(app));
    }));

    impl->resumeCallback = std::make_shared<core::ScopedConnection>(
        impl->registry->appResumed().connect(
            [this](std::shared_ptr<ual::Application> app,
                   std::shared_ptr<ual::Application::Instance>) {
        Q_EMIT resumeRequested(toShortAppIdIfPossible(app));
    }));

    impl->pausedCallback = std::make_shared<core::ScopedConnection>(
        impl->registry->appPaused().connect(
            [this](std::shared_ptr<ual::Application> app,
                   std::shared_ptr<ual::Application::Instance>) {
        Q_EMIT processSuspended(toShortAppIdIfPossible(app));
    }));

    impl->failureCallback = std::make_shared<core::ScopedConnection>(
        impl->registry->appFailed().connect(
            [this](std::shared_ptr<ual::Application> app,
                   std::shared_ptr<ual::Application::Instance>,
                   ual::Registry::FailureType failureType) {
        TaskController::Error error;
        switch(failureType)
        {
        case ual::Registry::FailureType::CRASH: error = TaskController::Error::APPLICATION_CRASHED;
        case ual::Registry::FailureType::START_FAILURE: error = TaskController::Error::APPLICATION_FAILED_TO_START;
        }

        Q_EMIT processFailed(toShortAppIdIfPossible(app), error);
    }));
}

TaskController::~TaskController()
{
}

bool TaskController::appIdHasProcessId(const QString& appId, pid_t pid)
{
    auto app = createApp(appId, impl->registry);
    if (!app) {
        return false;
    }

    for (auto &instance: app->instances()) {
        if (instance->hasPid(pid)) {
            return true;
        }
    }

    return false;
}

bool TaskController::stop(const QString& appId)
{
    auto app = createApp(appId, impl->registry);
    if (!app) {
        return false;
    }

    for (auto &instance: app->instances()) {
        instance->stop();
    }

    return true;
}

bool TaskController::start(const QString& appId, const QStringList& arguments)
{
    auto app = createApp(appId, impl->registry);
    if (!app) {
        return false;
    }

    // Convert arguments QStringList into format suitable for ubuntu-app-launch
    std::vector<ual::Application::URL> urls;
    for (auto &arg: arguments) {
        urls.emplace_back(ual::Application::URL::from_raw(arg.toStdString()));
    }

    app->launch(urls);

    return true;
}

bool TaskController::approveStart(const QString& appId, bool approved)
{
    auto app = createApp(appId, impl->registry);
    if (!app) {
        return false;
    }

    Q_UNUSED(approved); // TODO
    return true;
}

bool TaskController::suspend(const QString& appId)
{
    auto app = createApp(appId, impl->registry);
    if (!app) {
        return false;
    }

    for (auto &instance: app->instances()) {
        instance->pause();
    }

    return true;
}

bool TaskController::resume(const QString& appId)
{
    auto app = createApp(appId, impl->registry);
    if (!app) {
        return false;
    }

    for (auto &instance: app->instances()) {
        instance->resume();
    }

    return true;
}

QSharedPointer<qtmir::ApplicationInfo> TaskController::getInfoForApp(const QString &appId) const
{
    auto app = createApp(appId, impl->registry);
    if (!app || !app->info()) {
        return QSharedPointer<qtmir::ApplicationInfo>();
    }

    QString shortAppId = toShortAppIdIfPossible(app);
    auto appInfo = new qtmir::upstart::ApplicationInfo(shortAppId, app->info());
    return QSharedPointer<qtmir::ApplicationInfo>(appInfo);
}

} // namespace upstart
} // namespace qtmir
