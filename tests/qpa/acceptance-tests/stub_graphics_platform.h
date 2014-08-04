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

#ifndef STUB_GRAPHICS_PLATFORM_H_
#define STUB_GRAPHICS_PLATFORM_H_

#include <mir/graphics/platform.h>

struct StubPlatform : public mir::graphics::Platform
{
    StubPlatform() = default;
    virtual ~StubPlatform() = default;
    
    std::shared_ptr<mir::graphics::GraphicBufferAllocator> create_buffer_allocator(
        std::shared_ptr<mir::graphics::BufferInitializer> const& buffer_initializer) override;

    std::shared_ptr<mir::graphics::Display> create_display(
        std::shared_ptr<mir::graphics::DisplayConfigurationPolicy> const& initial_conf_policy,
        std::shared_ptr<mir::graphics::GLProgramFactory> const& gl_program_factory,
        std::shared_ptr<mir::graphics::GLConfig> const& gl_config) override;

    std::shared_ptr<mir::graphics::PlatformIPCPackage> get_ipc_package() override;
    
    void fill_buffer_package(mir::graphics::BufferIPCPacker* packer, mir::graphics::Buffer const* buffer,
        mir::graphics::BufferIpcMsgType msg_type) const override;
    
    std::shared_ptr<mir::graphics::InternalClient> create_internal_client() override;
    
    EGLNativeDisplayType egl_native_display() const override;
};

#endif // STUB_GRAPHICS_PLATFORM_H_

