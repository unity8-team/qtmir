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

#include <gtest/gtest.h>
#include "gmock_fixes.h"

#include "stub_display.h"
#include "mock_main_loop.h"
#include "qtcompositor.h"
#include "fake_displayconfigurationoutput.h"

#include "testable_screencontroller.h"
#include "screen.h"

using namespace ::testing;

namespace mg = mir::graphics;
namespace geom = mir::geometry;

class ScreenControllerTest : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;

    ScreenController *sc;
    std::shared_ptr<StubDisplay> display;
    std::shared_ptr<QtCompositor> compositor;
};

void ScreenControllerTest::SetUp()
{
    setenv("QT_QPA_PLATFORM", "minimal", 1);
    Screen::skipDBusRegistration = true;

    sc = new TestableScreenController;
    display = std::make_shared<StubDisplay>();
    compositor = std::make_shared<QtCompositor>();
    auto mainLoop = std::make_shared<MockMainLoop>();

    EXPECT_CALL(*display, register_configuration_change_handler(_,_))
        .Times(1);

    static_cast<TestableScreenController*>(sc)->do_init(display, compositor, mainLoop);
}

void ScreenControllerTest::TearDown()
{
    delete sc;
}

TEST_F(ScreenControllerTest, SingleScreenFound)
{
    std::vector<mg::DisplayConfigurationOutput> config{fake_output1};
    std::vector<geom::Rectangle> bufferConfig{{ {0, 0}, fake_output1.modes[0].size} };

    display->setFakeConfiguration(config, bufferConfig);
    sc->update();

    ASSERT_EQ(sc->screens().count(), 1);
    Screen* screen = sc->screens().first();
    EXPECT_EQ(screen->geometry(), QRect(0, 0, 150, 200));
}

TEST_F(ScreenControllerTest, MultipleScreenFound)
{
    std::vector<mg::DisplayConfigurationOutput> config{fake_output1, fake_output2};
    std::vector<geom::Rectangle> bufferConfig{
        { {0, 0}, fake_output1.modes[0].size},
        { {0, 0}, fake_output2.modes[1].size}
    };

    display->setFakeConfiguration(config, bufferConfig);
    sc->update();

    ASSERT_EQ(sc->screens().count(), 2);
    EXPECT_EQ(sc->screens().at(0)->geometry(), QRect(0, 0, 150, 200));
    EXPECT_EQ(sc->screens().at(1)->geometry(), QRect(500, 600, 1500, 2000));
}

TEST_F(ScreenControllerTest, ScreenAdded)
{
    std::vector<mg::DisplayConfigurationOutput> config{fake_output1};
    std::vector<geom::Rectangle> bufferConfig{{ {0, 0}, fake_output1.modes[0].size} };

    display->setFakeConfiguration(config, bufferConfig);
    sc->update();

    config.push_back(fake_output2);
    bufferConfig.push_back({ {0, 0}, fake_output2.modes[1].size});

    ASSERT_EQ(sc->screens().count(), 1);
    EXPECT_EQ(sc->screens().at(0)->geometry(), QRect(0, 0, 150, 200));

    display->setFakeConfiguration(config, bufferConfig);
    sc->update();

    ASSERT_EQ(sc->screens().count(), 2);
    EXPECT_EQ(sc->screens().at(0)->geometry(), QRect(0, 0, 150, 200));
    EXPECT_EQ(sc->screens().at(1)->geometry(), QRect(500, 600, 1500, 2000));
}

TEST_F(ScreenControllerTest, ScreenRemoved)
{
    std::vector<mg::DisplayConfigurationOutput> config{fake_output2, fake_output1};
    std::vector<geom::Rectangle> bufferConfig{
        { {0, 0}, fake_output2.modes[1].size},
        { {0, 0}, fake_output1.modes[0].size}
    };

    display->setFakeConfiguration(config, bufferConfig);
    sc->update();

    config.pop_back();
    bufferConfig.pop_back();

    ASSERT_EQ(sc->screens().count(), 2);
    EXPECT_EQ(sc->screens().at(0)->geometry(), QRect(500, 600, 1500, 2000));
    EXPECT_EQ(sc->screens().at(1)->geometry(), QRect(0, 0, 150, 200));

    display->setFakeConfiguration(config, bufferConfig);
    sc->update();

    ASSERT_EQ(sc->screens().count(), 1);
    EXPECT_EQ(sc->screens().at(0)->geometry(), QRect(500, 600, 1500, 2000));
}

TEST_F(ScreenControllerTest, CompositorStopHidesAllWindows)
{

}

TEST_F(ScreenControllerTest, CompositorStartShowsAllWindows)
{
}
