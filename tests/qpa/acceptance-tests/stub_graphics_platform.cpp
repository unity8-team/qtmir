/*
 * Copyright © 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <mir/graphics/graphic_buffer_allocator.h>
#include <mir/graphics/display.h>
#include <mir/graphics/gl_context.h>
#include <mir/graphics/display_configuration.h>
#include <mir/graphics/platform_ipc_package.h>

#include "stub_graphics_platform.h"

#include <thread>

namespace mg = mir::graphics;

namespace
{

struct StubBufferAllocator : public mg::GraphicBufferAllocator
{
    std::shared_ptr<mg::Buffer> alloc_buffer(
        mg::BufferProperties const& properties)
    {
        (void) properties;
        return nullptr;
    }

    std::vector<MirPixelFormat> supported_pixel_formats()
    {
        return {};
    }
};

struct StubGLContext : public mg::GLContext
{
    void make_current() const
    {
    }

    void release_current() const
    {
    }
};

class StubDisplayConfiguration : public mg::DisplayConfiguration
{
    void for_each_card(std::function<void(mg::DisplayConfigurationCard const&)>) const override
    {
    }
    void for_each_output(std::function<void(mg::DisplayConfigurationOutput const&)>) const override
    {
    }
    void for_each_output(std::function<void(mg::UserDisplayConfigurationOutput&)>) override
    {
    }
};

class StubDisplay : public mg::Display
{
 public:
    void for_each_display_buffer(std::function<void(mg::DisplayBuffer&)> const&)
    {
        /* yield() is needed to ensure reasonable runtime under valgrind for some tests */
        std::this_thread::yield();
    }
    std::unique_ptr<mg::DisplayConfiguration> configuration() const override
    {
        return std::unique_ptr<mg::DisplayConfiguration>(
            new StubDisplayConfiguration
        );
    }
    void configure(mg::DisplayConfiguration const&) {}
    void register_configuration_change_handler(
        mg::EventHandlerRegister&,
        mg::DisplayConfigurationChangeHandler const&) override
    {
    }
    void register_pause_resume_handlers(mg::EventHandlerRegister&,
                                        mg::DisplayPauseHandler const&,
                                        mg::DisplayResumeHandler const&) override
    {
    }
    void pause() {}
    void resume() {}
    std::shared_ptr<mg::Cursor> create_hardware_cursor(std::shared_ptr<mg::CursorImage> const& /* initial_image */)
    {
         return {}; 
    }
    std::unique_ptr<mg::GLContext> create_gl_context()
    {
        return std::unique_ptr<StubGLContext>{new StubGLContext()};
    }
};

}

std::shared_ptr<mg::GraphicBufferAllocator> StubPlatform::create_buffer_allocator(
    std::shared_ptr<mg::BufferInitializer> const& buffer_initializer)
{
    (void) buffer_initializer;
    return std::make_shared<StubBufferAllocator>();
}

std::shared_ptr<mg::Display> StubPlatform::create_display(
    std::shared_ptr<mg::DisplayConfigurationPolicy> const& initial_conf_policy,
    std::shared_ptr<mg::GLProgramFactory> const& gl_program_factory,
    std::shared_ptr<mg::GLConfig> const& gl_config)
{
    (void) initial_conf_policy;
    (void) gl_program_factory;
    (void) gl_config;
    // TODO: Probably need some sizing...
    return std::make_shared<StubDisplay>();
}

std::shared_ptr<mg::PlatformIPCPackage> StubPlatform::get_ipc_package()
{
    return std::make_shared<mg::PlatformIPCPackage>();
}
    
void StubPlatform::fill_buffer_package(mg::BufferIPCPacker* packer, mg::Buffer const* buffer,
    mg::BufferIpcMsgType msg_type) const
{
    (void) packer;
    (void) buffer;
    (void) msg_type;
}
    
std::shared_ptr<mg::InternalClient> StubPlatform::create_internal_client()
{
    return nullptr;
}

EGLNativeDisplayType StubPlatform::egl_native_display() const
{
    return EGLNativeDisplayType();
}
