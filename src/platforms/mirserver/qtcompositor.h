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
 *
 * Author: Gerry Boland <gerry.boland@canonical.com>
 */

#ifndef QTCOMPOSITOR_H
#define QTCOMPOSITOR_H

#include <mir/compositor/compositor.h>

// Qt
#include <QObject>
#include <QMutex>

#include <mir/graphics/display_configuration.h>

// std
#include <memory>

class QScreen;
class MirServerIntegration;
class Screen;
namespace mir {
    namespace graphics { class Display; }
}

class QtCompositor : public QObject, public mir::compositor::Compositor
{
    Q_OBJECT
public:
    QtCompositor(const std::shared_ptr<mir::graphics::Display> &display,
                 MirServerIntegration *platformIntegration);
    ~QtCompositor();

    void start();
    void stop();

Q_SIGNALS:
    void starting();
    void stopping();

private Q_SLOTS:
    void onStarting();
    void onStopping();

private:
    void updateScreens();
    Screen* findScreen(const QList<QScreen*> &list, const mir::graphics::DisplayConfigurationOutputId id);

    bool m_running;
    QMutex m_runningMutex;
    const std::shared_ptr<mir::graphics::Display> &m_display;
    MirServerIntegration *m_platformIntegration;
};

#endif // QTCOMPOSITOR_H
