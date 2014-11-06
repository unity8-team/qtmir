/*
 * Copyright (C) 2013-2014 Canonical, Ltd.
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

#include "mirserver.h"

// local
#include "focussetter.h"
#include "mirglconfig.h"
#include "mirplacementstrategy.h"
#include "mirserverstatuslistener.h"
#include "promptsessionlistener.h"
#include "sessionlistener.h"
#include "surfaceconfigurator.h"
#include "sessionauthorizer.h"
#include "qtcompositor.h"
#include "qteventfeeder.h"
#include "logging.h"

// egl
#include <EGL/egl.h>

namespace msh = mir::shell;
namespace ms = mir::scene;

namespace
{
void ignore_unparsed_arguments(int /*argc*/, char const* const/*argv*/[])
{
}
}

Q_LOGGING_CATEGORY(QTMIR_MIR_MESSAGES, "qtmir.mir")

MirServer::MirServer(int argc, char const* argv[], QObject* parent)
    : QObject(parent)
{
    override_the_compositor([]() -> std::shared_ptr<mir::compositor::Compositor>
        {
            return std::make_shared<QtCompositor>();
        });

    override_the_placement_strategy([this] () -> std::shared_ptr<ms::PlacementStrategy>
        {
            return std::make_shared<MirPlacementStrategy>(the_shell_display_layout());
        });

    override_the_input_dispatcher([]() -> std::shared_ptr<mir::input::InputDispatcher>
        {
            return std::make_shared<QtEventFeeder>();
        });

    override_the_gl_config([]() -> std::shared_ptr<mir::graphics::GLConfig>
        {
#ifdef QTMIR_USE_OPENGL
            // Should desktop-GL be desired, need to bind that API before a context is created
            eglBindAPI(EGL_OPENGL_API);
#endif
            return std::make_shared<MirGLConfig>();
        });

    override_the_server_status_listener([]() -> std::shared_ptr<mir::ServerStatusListener>
        {
            return std::make_shared<MirServerStatusListener>();
        });

    override_the_shell_focus_setter([]() -> std::shared_ptr<mir::shell::FocusSetter>
        {
            return std::make_shared<FocusSetter>();
        });

    override_the_session_listener([]() -> std::shared_ptr<ms::SessionListener>
        {
            return std::make_shared<SessionListener>();
        });

    override_the_prompt_session_listener([]() -> std::shared_ptr<ms::PromptSessionListener>
        {
            return std::make_shared<PromptSessionListener>();
        });

    override_the_surface_configurator([]() -> std::shared_ptr<ms::SurfaceConfigurator>
        {
            return std::make_shared<SurfaceConfigurator>();
        });

    override_the_session_authorizer([]() -> std::shared_ptr<mir::frontend::SessionAuthorizer>
        {
            return std::make_shared<SessionAuthorizer>();
        });

    set_command_line_handler(&ignore_unparsed_arguments);

    set_command_line(argc, argv);

    apply_settings();

    qCDebug(QTMIR_MIR_MESSAGES) << "MirServer created";
}

/************************************ Shell side ************************************/

//
// Note about the
//     if (sharedPtr.unique()) return 0;
// constructs used in the functions below.
// The rationale is that if when you do
//     the_session_authorizer()
// get a pointer that is unique means that Mir is not
// holding the pointer and thus when we return from the
//     sessionAuthorizer()
// scope the unique pointer will be destroyed so we return 0
//

SessionAuthorizer *MirServer::sessionAuthorizer()
{
    auto sharedPtr = the_session_authorizer();
    if (sharedPtr.unique()) return 0;

    return static_cast<SessionAuthorizer*>(sharedPtr.get());
}

SessionListener *MirServer::sessionListener()
{
    auto sharedPtr = the_session_listener();
    if (sharedPtr.unique()) return 0;

    return static_cast<SessionListener*>(sharedPtr.get());
}

PromptSessionListener *MirServer::promptSessionListener()
{
    auto sharedPtr = the_prompt_session_listener();
    if (sharedPtr.unique()) return 0;

    return static_cast<PromptSessionListener*>(sharedPtr.get());
}

SurfaceConfigurator *MirServer::surfaceConfigurator()
{
    auto sharedPtr = the_surface_configurator();
    if (sharedPtr.unique()) return 0;

    return static_cast<SurfaceConfigurator*>(sharedPtr.get());
}