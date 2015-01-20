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

#include "offscreensurface.h"

#include "mirserver.h"

// Mir
#include <mir/graphics/display.h>
#include <mir/graphics/gl_context.h>

//Qt
#include <QOffscreenSurface>
#include <QSurfaceFormat>
#include <QtPlatformSupport/private/qeglconvenience_p.h>

#define MESA_EGL_NO_X11_HEADERS
#include <EGL/egl.h>
#include <QDebug>

namespace mg = mir::graphics;

OffscreenSurface::OffscreenSurface(const QSharedPointer<MirServer> &mirServer,
                                   QOffscreenSurface *offscreenSurface)
    : QPlatformOffscreenSurface(offscreenSurface)
    , m_format(offscreenSurface->requestedFormat())
{
    std::shared_ptr<mg::Display> display = mirServer->the_display();

    // create a temporary GL context to fetch the EGL display and config, so Qt can determine the surface format
    std::unique_ptr<mg::GLContext> mirContext = display->create_gl_context();
    mirContext->make_current();

    m_eglDisplay = eglGetCurrentDisplay();
    if (m_eglDisplay == EGL_NO_DISPLAY) {
        qFatal("[qtmir] OffscreenSurface: Unable to determine current EGL Display");
    }

    EGLContext eglContext = eglGetCurrentContext();
    if (eglContext == EGL_NO_CONTEXT) {
        qFatal("Unable to determine current EGL Context");
    }
    EGLint eglConfigId = -1;
    EGLBoolean result;
    result = eglQueryContext(m_eglDisplay, eglContext, EGL_CONFIG_ID, &eglConfigId);
    if (result != EGL_TRUE || eglConfigId < 0) {
        qFatal("Unable to determine current EGL Config ID");
    }

//    EGLint matchingEglConfigCount;
//    EGLint const attribList1[] = {
//        EGL_CONFIG_ID, eglConfigId,
//        EGL_NONE
//    };
//    result = eglChooseConfig(m_eglDisplay, attribList1, &eglConfig, 1, &matchingEglConfigCount);
//    if (result != EGL_TRUE || eglConfig == nullptr || matchingEglConfigCount < 1) {
//        qFatal("Unable to select EGL Config with the supposed current config ID");
//    }
qDebug() << eglConfigId;
    EGLint matching = 0;
        EGLint const attribList[] = {
            EGL_SURFACE_TYPE,  EGL_PBUFFER_BIT,
            EGL_NONE
        };
    eglChooseConfig(m_eglDisplay ,attribList, 0, 0, &matching);
    QVector<EGLConfig> configs(matching);
    eglChooseConfig(m_eglDisplay, attribList, configs.data(), configs.size(), &matching);
    for (int i = 0; i < configs.size(); ++i) {
        qDebug() << "Config" << i;
        q_printEglConfig(m_eglDisplay, configs[i]);
    }


    EGLConfig eglConfig = q_configFromGLFormat(m_eglDisplay, m_format, false, EGL_PBUFFER_BIT);
    if (!eglConfig) {
        qFatal("[qtmir] OffscreenSurface: Unable to determine current EGL config");
    }

    const EGLint attributes[] = {
        EGL_WIDTH, offscreenSurface->size().width(),
        EGL_HEIGHT, offscreenSurface->size().height(),
        EGL_LARGEST_PBUFFER, EGL_FALSE,
        EGL_NONE
    };

    m_eglSurface = eglCreatePbufferSurface(m_eglDisplay, eglConfig, attributes);

    if (m_eglSurface == EGL_NO_SURFACE) {
        qFatal("[qtmir] OffscreenSurface: Unable to create EGL surface for offscreen rendering");
    }

    m_format = q_glFormatFromConfig(m_eglDisplay, eglConfig);
}

OffscreenSurface::~OffscreenSurface()
{
    eglDestroySurface(m_eglDisplay, m_eglSurface);
}

QSurfaceFormat OffscreenSurface::format() const
{
    return m_format;
}

bool OffscreenSurface::isValid() const
{
    return m_eglSurface != EGL_NO_SURFACE;
}

EGLSurface OffscreenSurface::eglSurface() const
{
    return m_eglSurface;
}
