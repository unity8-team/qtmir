/*
 * Copyright (C) 2013,2014 Canonical, Ltd.
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
 * Authors: Gerry Boland <gerry.boland@canonical.com>
 *          Daniel d'Andrada <daniel.dandrada@canonical.com>
 */

#ifndef SCREEN_H
#define SCREEN_H

// Qt
#include <QObject>
#include <QTimer>
#include <QPointer>
#include <QtDBus/QDBusInterface>
#include <qpa/qplatformscreen.h>

// Mir
#include "mir/graphics/display_configuration.h"

// local
#include "displaywindow.h"

class QOrientationSensor;

class Screen : public QObject, public QPlatformScreen
{
    Q_OBJECT
public:
    Screen(const mir::graphics::DisplayConfigurationOutput &);
    ~Screen();

    // QPlatformScreen methods.
    QRect geometry() const override { return m_geometry; }
    int depth() const override { return m_depth; }
    QImage::Format format() const override { return m_format; }
    QSizeF physicalSize() const override { return m_physicalSize; }
    qreal refreshRate() const override { return m_refreshRate; }
    Qt::ScreenOrientation nativeOrientation() const override { return m_nativeOrientation; }
    Qt::ScreenOrientation orientation() const override { return m_currentOrientation; }

    void toggleSensors(const bool enable) const;

    // QObject methods.
    void customEvent(QEvent* event) override;

    DisplayWindow* window() const;
    void setWindow(DisplayWindow *window);

    // To make it testable
    static bool skipDBusRegistration;
    bool orientationSensorEnabled();

public Q_SLOTS:
   void onDisplayPowerStateChanged(int, int);
   void onOrientationReadingChanged();

private:
    void setMirDisplayConfiguration(const mir::graphics::DisplayConfigurationOutput &);

    QRect m_geometry;
    int m_depth;
    QImage::Format m_format;
    QSizeF m_physicalSize;
    qreal m_refreshRate;

    mir::graphics::DisplayConfigurationOutputId m_outputId;
    mir::graphics::DisplayConfigurationCardId m_cardId;
    mir::graphics::DisplayConfigurationOutputType m_type;
    MirPowerMode m_powerMode;

    Qt::ScreenOrientation m_nativeOrientation;
    Qt::ScreenOrientation m_currentOrientation;
    QOrientationSensor *m_orientationSensor;

    QPointer<DisplayWindow> m_displayWindow;
    QDBusInterface *m_unityScreen;

    friend class QtCompositor;
};

#endif // SCREEN_H
