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

#ifndef DISPLAYCONFIGURATIONLISTENER_H
#define DISPLAYCONFIGURATIONLISTENER_H

#include <QObject>
#include <qpa/qplatformscreen.h>

class MirServer;
class MirServerIntegration;

class DisplayConfigurationListener : public QObject
{
    Q_OBJECT
public:
    DisplayConfigurationListener(const QSharedPointer<MirServer> &server,
                                 MirServerIntegration *platformIntegration);
    ~DisplayConfigurationListener();

private Q_SLOTS:
    void updateScreens();

private:
    QHash<int, QPlatformScreen*> m_screens;
    const QSharedPointer<MirServer> m_mirServer;
    MirServerIntegration *m_platformIntegration;
};

#endif // DISPLAYCONFIGURATIONLISTENER_H
