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

#include "screenobserver.h"
#include "screen.h"
#include "window.h"
#include "logging.h"

// Qt
#include <QMetaObject>
#include <QPointer>

// Mir
#include <mirclient/mir_toolkit/mir_connection.h>
#include <mirclient/mir_toolkit/mir_display_configuration.h>

#include <memory>

namespace {
    static void displayConfigurationChangedCallback(MirConnection */*connection*/, void* context)
    {
        ASSERT(context != NULL);
        UbuntuScreenObserver *observer = static_cast<UbuntuScreenObserver *>(context);
        QMetaObject::invokeMethod(observer, "update");
    }
} // anonymous namespace

UbuntuScreenObserver::UbuntuScreenObserver(MirConnection *mirConnection)
    : mMirConnection(mirConnection)
{
    mir_connection_set_display_config_change_callback(mirConnection, ::displayConfigurationChangedCallback, this);
    update();
}

void UbuntuScreenObserver::update()
{
    // Wrap MirDisplayConfiguration to always delete when out of scope
    auto configDeleter = [](MirDisplayConfig *config) { mir_display_config_release(config); };
    using configUp = std::unique_ptr<MirDisplayConfig, decltype(configDeleter)>;
    configUp displayConfig(mir_connection_create_display_configuration(mMirConnection), configDeleter);

    // Mir only tells us something changed, it is up to us to figure out what.
    QList<UbuntuScreen*> newScreenList;
    QList<UbuntuScreen*> oldScreenList = mScreenList;
    typedef QPair<UbuntuScreen*,UbuntuScreen*> ScreenPair;
    QVector<ScreenPair> replaceScreenList; //for Screens we need to destroy & recreate
    mScreenList.clear();

    for (int i=0; i<mir_display_config_get_num_outputs(displayConfig.get()); i++) {
        const MirOutput *output = mir_display_config_get_output(displayConfig.get(), i);
        if (mir_output_is_enabled(output)) {
            UbuntuScreen *screen = findScreenWithId(oldScreenList, mir_output_get_id(output));
            if (screen) { // we've already set up this display before
                if (screen->canUpdateMirOutput(output)) { // can we re-use it?
                    screen->updateMirOutput(output);
                    oldScreenList.removeAll(screen);
                } else { // need to delete & recreate
                    auto oldScreen = screen;
                    screen = new UbuntuScreen(output, mMirConnection);
                    newScreenList.append(screen);
                    replaceScreenList.append({oldScreen, screen});
                }
            } else {
                // new display, so create UbuntuScreen for it
                screen = new UbuntuScreen(output, mMirConnection);
                newScreenList.append(screen);
                qDebug() << "Added Screen with id" << mir_output_get_id(output)
                         << "and geometry" << screen->geometry();
            }
            mScreenList.append(screen);
        }
    }

    // Announce new Screens first, as some Windows may need to be "moved" to their new Screen before
    // the old Screen is deleted.
    Q_FOREACH (const auto screen, newScreenList) {
        Q_EMIT screenAdded(screen);
    }

    // Announce that one Screen will be replacing another, so that Windows are "moved" from the old
    // screen to the new one, before the old one is deleted
    Q_FOREACH (const auto screens, replaceScreenList) {
        Q_EMIT screenReplaced(screens.first, screens.second);
    }

    // Announce old & unused Screens, should be deleted by the slot
    Q_FOREACH (const auto screen, oldScreenList) {
        Q_EMIT screenRemoved(screen);
    }

    qDebug() << "=======================================";
    for (auto screen: mScreenList) {
        qDebug() << screen << "- id:" << screen->outputId()
                           << "geometry:" << screen->geometry()
                           << "form factor:" << screen->formFactor()
                           << "scale:" << screen->scale();
    }
    qDebug() << "=======================================";
}

UbuntuScreen *UbuntuScreenObserver::findScreenWithId(uint32_t id)
{
    return findScreenWithId(mScreenList, id);
}

UbuntuScreen *UbuntuScreenObserver::findScreenWithId(const QList<UbuntuScreen *> &list, uint32_t id)
{
    Q_FOREACH (const auto screen, list) {
        if (screen->outputId() == id) {
            return screen;
        }
    }
    return nullptr;
}

void UbuntuScreenObserver::handleScreenPropertiesChange(UbuntuScreen *screen, int dpi,
                                                        MirFormFactor formFactor, float scale)
{
    screen->setAdditionalMirDisplayProperties(scale, formFactor, dpi);
}

