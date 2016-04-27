/*
 * Copyright (C) 2016 Canonical, Ltd.
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

    const char *mirFormFactorToStr(MirFormFactor formFactor)
    {
        switch (formFactor) {
        case mir_form_factor_unknown: return "unknown";
        case mir_form_factor_phone: return "phone";
        case mir_form_factor_tablet: return "tablet";
        case mir_form_factor_monitor: return "monitor";
        case mir_form_factor_tv: return "tv";
        case mir_form_factor_projector: return "projector";
        }
        return "";
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
    mScreenList.clear();

    for (int i = 0; i < mir_display_config_get_num_outputs(displayConfig.get()); i++) {
        const MirOutput *output = mir_display_config_get_output(displayConfig.get(), i);
        if (mir_output_is_enabled(output)) {
            UbuntuScreen *screen = findScreenWithId(oldScreenList, mir_output_get_id(output));
            if (screen) { // we've already set up this display before
                screen->updateMirOutput(output);
                oldScreenList.removeAll(screen);
            } else {
                // new display, so create UbuntuScreen for it
                screen = new UbuntuScreen(output, mMirConnection);
                newScreenList.append(screen);
                qCDebug(ubuntumirclient) << "Added Screen with id" << mir_output_get_id(output)
                                         << "and geometry" << screen->geometry();
            }
            mScreenList.append(screen);
        }
    }

    // Announce old & unused Screens, should be deleted by the slot
    Q_FOREACH (const auto screen, oldScreenList) {
        Q_EMIT screenRemoved(screen);
    }

    /*
     * Mir's MirDisplayOutput does not include formFactor or scale for some reason, but Qt
     * will want that information on creating the QScreen. Only way we get that info is when
     * Mir positions a Window on that Screen. See "handleScreenPropertiesChange" method
     */

    // Announce new Screens
    Q_FOREACH (const auto screen, newScreenList) {
        Q_EMIT screenAdded(screen);
    }

    qCDebug(ubuntumirclient) << "=======================================";
    for (auto screen: mScreenList) {
        qCDebug(ubuntumirclient) << screen << "- id:" << screen->outputId()
                                 << "geometry:" << screen->geometry()
                                 << "form factor:" << mirFormFactorToStr(screen->formFactor())
                                 << "scale:" << screen->scale();
    }
    qCDebug(ubuntumirclient) << "=======================================";
}

UbuntuScreen *UbuntuScreenObserver::findScreenWithId(int id)
{
    return findScreenWithId(mScreenList, id);
}

UbuntuScreen *UbuntuScreenObserver::findScreenWithId(const QList<UbuntuScreen *> &list, int id)
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

