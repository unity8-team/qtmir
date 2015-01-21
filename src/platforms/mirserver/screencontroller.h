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
 *
 * Author: Gerry Boland <gerry.boland@canonical.com>
 */

#ifndef SCREENCONTROLLER_H
#define SCREENCONTROLLER_H

#include <QObject>
#include <QMutex>

// Mir
#include <mir/graphics/display_configuration.h>

// std
#include <memory>

class MirServer;
class Screen;
class QWindow;


class ScreenController : public QObject
{
    Q_OBJECT
public:
    explicit ScreenController(QObject *parent = 0);

    Screen* getUnusedScreen();

    QWindow* getWindowForPoint(const QPoint &point);

Q_SIGNALS:
    void screenAdded(Screen *screen);

private Q_SLOTS:
    void onCompositorStarting();
    void onCompositorStopping();
    void update();

private:
    void setMirServer(MirServer *ms) { m_server = ms; } // call before MirServer is running - dangerous
    void init(); // call after the MirServer is running
    Screen* findScreenWithId(const QList<Screen*> &list, const mir::graphics::DisplayConfigurationOutputId id);


    MirServer *m_server;
    QList<Screen*> m_screenList;
    bool m_watchForUpdates;
    QMutex m_mutex;

    friend class QMirServer;
    friend class MirServer;
};

#endif // SCREENCONTROLLER_H
