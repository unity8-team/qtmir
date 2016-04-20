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

#ifndef UBUNTU_SCREEN_OBSERVER_H
#define UBUNTU_SCREEN_OBSERVER_H

#include <QObject>

#include <mir_toolkit/mir_connection.h>

class UbuntuScreen;

class UbuntuScreenObserver : public QObject
{
    Q_OBJECT

public:
    UbuntuScreenObserver(MirConnection *connection);

    QList<UbuntuScreen*> screens() const { return mScreenList; }
    UbuntuScreen *findScreenWithId(int id);

    void handleScreenPropertiesChange(UbuntuScreen *screen, int dpi,
                                      MirFormFactor formFactor, float scale);

Q_SIGNALS:
    void screenAdded(UbuntuScreen *screen);
    void screenRemoved(UbuntuScreen *screen);

private Q_SLOTS:
    void update();

private:
    UbuntuScreen *findScreenWithId(const QList<UbuntuScreen *> &list, int id);
    void removeScreen(UbuntuScreen *screen);

    MirConnection *mMirConnection;
    QList<UbuntuScreen*> mScreenList;
};

#endif // UBUNTU_SCREEN_OBSERVER_H
