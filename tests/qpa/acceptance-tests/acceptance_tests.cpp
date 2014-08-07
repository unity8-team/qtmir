/*
 * Copyright (C) 2014 Canonical, Ltd.
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

#include <Unity/Application/application_manager.h>

#include <Unity/Application/applicationcontroller.h>
#include <Unity/Application/taskcontroller.h>
#include <Unity/Application/proc_info.h>
#include <mirserverconfiguration.h>
#include <qmirserver.h>

#include <mir_toolkit/mir_client_library.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "stub_graphics_platform.h"
#include "mir_test_framework/deferred_in_process_server.h"

namespace mg = mir::graphics;
namespace mtf = mir_test_framework;

namespace
{

static char const* argv[] = {
    "acceptance-tests",
};

struct TestingServerConfiguration : public MirServerConfiguration
{
    TestingServerConfiguration()
        : MirServerConfiguration(1, argv)
    {
    }
    std::shared_ptr<mg::Platform> the_graphics_platform() override
    {
        return std::make_shared<StubPlatform>();
    }
};

struct TestQPAServer : public mtf::DeferredInProcessServer
{
    TestingServerConfiguration conf;
    mir::DefaultServerConfiguration& server_config() override
    {
        return conf;
    }
    
    void launch_client(std::function<void(std::string)> const& client_exec)
    {
        auto thread = std::thread(client_exec, new_connection());
        client_threads.push_back(std::move(thread));
    }

    void TearDown() override
    {
        for (auto &thread : client_threads)
        {
            if (thread.joinable())
                thread.join();
        }
        DeferredInProcessServer::TearDown();
    }

    std::vector<std::thread> client_threads;
};

}

TEST_F(TestQPAServer, client_may_connect_and_exit)
{
    using namespace testing;

    start_server();
    
    launch_client([&](std::string const& connect_string) -> void
    {
        MirConnection *connection = mir_connect_sync(connect_string.c_str(),
            __PRETTY_FUNCTION__);
        ASSERT_TRUE(mir_connection_is_valid(connection));
        mir_connection_release(connection);
    });
}
