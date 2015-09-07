/*
 * Copyright (C) 2014-2015 Canonical, Ltd.
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

#ifndef QT_MIR_TEST_FRAMEWORK_H
#define QT_MIR_TEST_FRAMEWORK_H

#include <memory>

#include <gtest/gtest.h>

#include <Unity/Application/application.h>
#include <Unity/Application/application_manager.h>
#include <Unity/Application/applicationcontroller.h>
#include <Unity/Application/mirsurfacemanager.h>
#include <Unity/Application/sessionmanager.h>
#include <Unity/Application/session_interface.h>
#include <Unity/Application/sharedwakelock.h>
#include <Unity/Application/taskcontroller.h>
#include <Unity/Application/proc_info.h>
#include <mirserver.h>

#include "mock_application_controller.h"
#include "mock_desktop_file_reader.h"
#include "mock_proc_info.h"
#include "mock_mir_session.h"
#include "mock_prompt_session_manager.h"
#include "mock_prompt_session.h"
#include "mock_shared_wakelock.h"
#include "mock_settings.h"
#include "mock_task_controller.h"

namespace ms = mir::scene;
using namespace qtmir;

namespace qtmir {

// For better output in ASSERT_* and EXPECT_* error messages
void PrintTo(const Application::InternalState& state, ::std::ostream* os);
void PrintTo(const SessionInterface::State& state, ::std::ostream* os);

// Initialization of mir::Server needed for by tests
class TestMirServerInit : virtual mir::Server
{
public:
    TestMirServerInit()
    {
        override_the_prompt_session_manager(
            [this]{ return the_mock_prompt_session_manager(); });
    }

    std::shared_ptr<mir::scene::MockPromptSessionManager> the_mock_prompt_session_manager()
    {
        return mock_prompt_session_manager;
    }

private:
    typedef testing::NiceMock<mir::scene::MockPromptSessionManager> StubPromptSessionManager;
    std::shared_ptr<StubPromptSessionManager> const mock_prompt_session_manager
        {std::make_shared<StubPromptSessionManager>()};
};


namespace {  char const* argv[] = { nullptr }; }

class FakeMirServer: private TestMirServerInit, public MirServer
{
public:
    FakeMirServer()
    : MirServer(0, argv)
    {
    }

    using TestMirServerInit::the_mock_prompt_session_manager;
};

} // namespace qtmir

namespace testing {

class QtMirTest : public ::testing::Test
{
public:
    QtMirTest()
        : mirServer{
            QSharedPointer<FakeMirServer> (new FakeMirServer)
        }
        , applicationManager{
            mirServer,
            QSharedPointer<MockTaskController>(&taskController, [](MockTaskController *){}),
            QSharedPointer<MockSharedWakelock>(&sharedWakelock, [](MockSharedWakelock *){}),
            QSharedPointer<DesktopFileReader::Factory>(
                &desktopFileReaderFactory,
                [](DesktopFileReader::Factory*){}),
            QSharedPointer<ProcInfo>(&procInfo,[](ProcInfo *){}),
            QSharedPointer<MockSettings>(&settings,[](MockSettings *){})
        }
        , sessionManager{
            mirServer,
            &applicationManager,
        }
        , surfaceManager{
            mirServer,
            mirShell,
            &sessionManager
        }
    {
    }

    Application* startApplication(quint64 procId, QString const& appId)
    {
        using namespace testing;

        ON_CALL(taskController,appIdHasProcessId(appId, procId)).WillByDefault(Return(true));

        // Set up Mocks & signal watcher
        EXPECT_CALL(taskController, start(appId, _));

        auto application = applicationManager.startApplication(appId, ApplicationManager::NoFlag);
        applicationManager.onProcessStarting(appId);

        bool authed = false;
        applicationManager.authorizeSession(procId, authed);
        EXPECT_EQ(authed, true);

        auto appSession = std::make_shared<mir::scene::MockSession>(appId.toStdString(), procId);
        applicationManager.onSessionStarting(appSession);
        sessionManager.onSessionStarting(appSession);
        
        Mock::VerifyAndClearExpectations(&taskController);
        return application;
    }

    void connectToTaskController(ApplicationManager *manager, TaskControllerInterface *controller)
    {
        QObject::connect(controller, &TaskControllerInterface::processStarting,
                         manager, &ApplicationManager::onProcessStarting);
        QObject::connect(controller, &TaskControllerInterface::processStopped,
                         manager, &ApplicationManager::onProcessStopped);
        QObject::connect(controller, &TaskControllerInterface::processSuspended,
                         manager, &ApplicationManager::onProcessSuspended);
        QObject::connect(controller, &TaskControllerInterface::processFailed,
                         manager, &ApplicationManager::onProcessFailed);
        QObject::connect(controller, &TaskControllerInterface::focusRequested,
                         manager, &ApplicationManager::onFocusRequested);
        QObject::connect(controller, &TaskControllerInterface::resumeRequested,
                         manager, &ApplicationManager::onResumeRequested);
    }

    testing::NiceMock<testing::MockApplicationController> appController;
    testing::NiceMock<testing::MockProcInfo> procInfo;
    testing::NiceMock<testing::MockDesktopFileReaderFactory> desktopFileReaderFactory;
    testing::NiceMock<testing::MockSharedWakelock> sharedWakelock;
    testing::NiceMock<testing::MockSettings> settings;
    QSharedPointer<FakeMirServer> mirServer;
    MirShell *mirShell{nullptr};
    testing::NiceMock<testing::MockTaskController> taskController;
    ApplicationManager applicationManager;
    SessionManager sessionManager;
    MirSurfaceManager surfaceManager;
};
} // namespace testing

#endif // QT_MIR_TEST_FRAMEWORK_H
