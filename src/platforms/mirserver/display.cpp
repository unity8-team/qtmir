/*
 * Copyright (C) 2013 Canonical, Ltd.
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
 * Author: Gerry Boland <gerry.boland@canonical.com>
 */

#include "display.h"

#include "screen.h"
#include "mirserver.h"
#include "mirserverintegration.h"

#include <mir/graphics/display.h>
#include <mir/graphics/display_configuration.h>
#include <mir/main_loop.h>
#include <QDebug>

namespace mg = mir::graphics;

Display::Display(const QSharedPointer<MirServer> &server, MirServerIntegration *platformIntegration)
    : m_mirServer(server)
    , m_platformIntegration(platformIntegration)
{
    std::shared_ptr<mg::Display> display = m_mirServer->the_display();

    display->register_configuration_change_handler(*m_mirServer->the_main_loop(), [this]() {
        // display configuration changed, update!
        //this->updateScreens(); // need to learn what thread this is called in
        QMetaObject::invokeMethod(this, "updateScreens", Qt::QueuedConnection);
    });

    display->register_pause_resume_handlers(*m_mirServer->the_main_loop(), []() -> bool {
        // pause handler - set the Window associated with the Screen as hidden
                                                // TODO
        return true;
    }, []() -> bool {
        // resume handler - set the Window associated with the Screen as visible
                                                // TODO
        return true;
    });

    updateScreens();
}

Display::~Display()
{
    for (auto screen : m_screens) {
        delete screen;
    }
}

void Display::updateScreens()
{
    qDebug() << "Display::updateScreens";
    std::shared_ptr<mg::DisplayConfiguration> displayConfig = m_mirServer->the_display()->configuration();

    QList<int> oldOutputIds = m_screens.keys();

    displayConfig->for_each_output([this, &oldOutputIds](mg::DisplayConfigurationOutput const& output) {
        if (output.used && output.connected) {
            int outputId = output.id.as_value();
            if (oldOutputIds.contains(outputId)) { // we've already set up this display before
                // TODO: check if the geometry matches the old Screen geometry

            } else {
                // new display, so create Screen for it
                auto screen = new Screen(output);
                m_screens.insert(outputId, screen);
                m_platformIntegration->screenAdded(screen); // notify Qt
            }

            oldOutputIds.removeAll(outputId);
        }
    });

    // Delete any old & unused Screens
    for (auto id: oldOutputIds) {
        delete m_screens.value(id);
        m_screens.remove(id);
    }
}
