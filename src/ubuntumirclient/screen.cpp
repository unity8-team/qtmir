/*
 * Copyright (C) 2014-2015 Canonical, Ltd.
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

// local
#include "screen.h"
#include "logging.h"
#include "orientationchangeevent_p.h"
#include "nativeinterface.h"
#include "utils.h"

#include <mir_toolkit/mir_client_library.h>

// Qt
#include <QGuiApplication>
#include <QtCore/qmath.h>
#include <QScreen>
#include <QThread>
#include <qpa/qwindowsysteminterface.h>
#include <QtPlatformSupport/private/qeglconvenience_p.h>

#include <memory>

static const int overrideDevicePixelRatio = qgetenv("QT_DEVICE_PIXEL_RATIO").toInt();

static const char *orientationToStr(Qt::ScreenOrientation orientation) {
    switch (orientation) {
        case Qt::PrimaryOrientation:
            return "primary";
        case Qt::PortraitOrientation:
            return "portrait";
        case Qt::LandscapeOrientation:
            return "landscape";
        case Qt::InvertedPortraitOrientation:
            return "inverted portrait";
        case Qt::InvertedLandscapeOrientation:
            return "inverted landscape";
        default:
            return "INVALID!";
    }
}


const QEvent::Type OrientationChangeEvent::mType =
        static_cast<QEvent::Type>(QEvent::registerEventType());


UbuntuScreen::UbuntuScreen(const MirOutput *output, MirConnection *connection)
    : mDevicePixelRatio(1.0)
    , mFormat(QImage::Format_RGB32)
    , mDepth(32)
    , mDpi{0}
    , mFormFactor{mir_form_factor_unknown}
    , mScale{1.0}
    , mOutputId(0)
    , mCursor(connection)
{
    setMirOutput(output);
}

UbuntuScreen::~UbuntuScreen()
{
}

void UbuntuScreen::customEvent(QEvent* event)
{
    Q_ASSERT(QThread::currentThread() == thread());

    OrientationChangeEvent* oReadingEvent = static_cast<OrientationChangeEvent*>(event);
    switch (oReadingEvent->mOrientation) {
        case OrientationChangeEvent::LeftUp: {
            mCurrentOrientation = (screen()->primaryOrientation() == Qt::LandscapeOrientation) ?
                        Qt::InvertedPortraitOrientation : Qt::LandscapeOrientation;
            break;
        }
        case OrientationChangeEvent::TopUp: {
            mCurrentOrientation = (screen()->primaryOrientation() == Qt::LandscapeOrientation) ?
                        Qt::LandscapeOrientation : Qt::PortraitOrientation;
            break;
        }
        case OrientationChangeEvent::RightUp: {
            mCurrentOrientation = (screen()->primaryOrientation() == Qt::LandscapeOrientation) ?
                        Qt::PortraitOrientation : Qt::InvertedLandscapeOrientation;
            break;
        }
        case OrientationChangeEvent::TopDown: {
            mCurrentOrientation = (screen()->primaryOrientation() == Qt::LandscapeOrientation) ?
                        Qt::InvertedLandscapeOrientation : Qt::InvertedPortraitOrientation;
            break;
        }
        default: {
            qCDebug(ubuntumirclient, "UbuntuScreen::customEvent - Unknown orientation.");
            return;
        }
    }

    // Raise the event signal so that client apps know the orientation changed
    qCDebug(ubuntumirclient, "UbuntuScreen::customEvent - handling orientation change to %s", orientationToStr(mCurrentOrientation));
    QWindowSystemInterface::handleScreenOrientationChange(screen(), mCurrentOrientation);
}

void UbuntuScreen::handleWindowSurfaceResize(int windowWidth, int windowHeight)
{
    if ((windowWidth > windowHeight && mGeometry.width() < mGeometry.height())
     || (windowWidth < windowHeight && mGeometry.width() > mGeometry.height())) {

        // The window aspect ratio differ's from the screen one. This means that
        // unity8 has rotated the window in its scene.
        // As there's no way to express window rotation in Qt's API, we have
        // Flip QScreen's dimensions so that orientation properties match
        // (primaryOrientation particularly).
        // FIXME: This assumes a phone scenario. Won't work, or make sense,
        //        on the desktop

        QRect currGeometry = mGeometry;
        mGeometry.setWidth(currGeometry.height());
        mGeometry.setHeight(currGeometry.width());

        qCDebug(ubuntumirclient, "UbuntuScreen::handleWindowSurfaceResize - new screen geometry (w=%d, h=%d)",
            mGeometry.width(), mGeometry.height());
        QWindowSystemInterface::handleScreenGeometryChange(screen(),
                                                           mGeometry /* newGeometry */,
                                                           mGeometry /* newAvailableGeometry */);

        if (mGeometry.width() < mGeometry.height()) {
            mCurrentOrientation = Qt::PortraitOrientation;
        } else {
            mCurrentOrientation = Qt::LandscapeOrientation;
        }
        qCDebug(ubuntumirclient, "UbuntuScreen::handleWindowSurfaceResize - new orientation %s",orientationToStr(mCurrentOrientation));
        QWindowSystemInterface::handleScreenOrientationChange(screen(), mCurrentOrientation);
    }
}

void UbuntuScreen::setMirOutput(const MirOutput *output)
{
    // Physical screen size
    mPhysicalSize.setWidth(mir_output_get_physical_width_mm(output));
    mPhysicalSize.setHeight(mir_output_get_physical_height_mm(output));

    // Pixel Format
//    mFormat = qImageFormatFromMirPixelFormat(mir_output_get_current_pixel_format(output)); // GERRY: TODO

    // Pixel depth
    mDepth = 8 * MIR_BYTES_PER_PIXEL(mir_output_get_current_pixel_format(output));

    // Mode = Resolution & refresh rate
    const MirOutputMode *mode = mir_output_get_current_mode(output);
    mNativeGeometry.setX(mir_output_get_position_x(output));
    mNativeGeometry.setY(mir_output_get_position_y(output));
    mNativeGeometry.setWidth(mir_output_mode_get_width(mode));
    mNativeGeometry.setHeight(mir_output_mode_get_height(mode));

    mRefreshRate = mir_output_mode_get_refresh_rate(mode);

    // UI scale & DPR - do not emit change signals on construction
    mScale = mir_output_get_scale_factor(output);
    if (overrideDevicePixelRatio > 0) {
        mDevicePixelRatio = overrideDevicePixelRatio;
    } else {
        mDevicePixelRatio = devicePixelRatioFromScale(mScale);
    }

    // Form factor - do not emit change signals on construction
    mFormFactor = mir_output_get_form_factor(output);

    mOutputId = mir_output_get_id(output);

    // geometry in device pixels
    mGeometry.setX(mNativeGeometry.x() / mDevicePixelRatio);
    mGeometry.setY(mNativeGeometry.y() / mDevicePixelRatio);
    mGeometry.setWidth(mNativeGeometry.width() / mDevicePixelRatio);
    mGeometry.setHeight(mNativeGeometry.height() / mDevicePixelRatio);

    // Set the default orientation based on the initial screen dimmensions.
    mNativeOrientation = (mGeometry.width() >= mGeometry.height()) ? Qt::LandscapeOrientation : Qt::PortraitOrientation;

    // If it's a landscape device (i.e. some tablets), start in landscape, otherwise portrait
    mCurrentOrientation = (mNativeOrientation == Qt::LandscapeOrientation) ? Qt::LandscapeOrientation : Qt::PortraitOrientation;
}

void UbuntuScreen::updateMirOutput(const MirOutput *output)
{
    auto oldRefreshRate = mRefreshRate;
    auto oldScale = mScale;
    auto oldFormFactor = mFormFactor;
    auto oldGeometry = mGeometry;

    setMirOutput(output);

    // Emit change signals in particular order
    if (oldGeometry != mGeometry) {
        QWindowSystemInterface::handleScreenGeometryChange(screen(),
                                                           mGeometry /* newGeometry */,
                                                           mGeometry /* newAvailableGeometry */);
    }

    if (!qFuzzyCompare(mRefreshRate, oldRefreshRate)) {
        QWindowSystemInterface::handleScreenRefreshRateChange(screen(), mRefreshRate);
    }

    auto nativeInterface = static_cast<UbuntuNativeInterface *>(qGuiApp->platformNativeInterface());
    if (!qFuzzyCompare(mScale, oldScale)) {
        nativeInterface->screenPropertyChanged(this, QStringLiteral("scale"));
    }
    if (mFormFactor != oldFormFactor) {
        nativeInterface->screenPropertyChanged(this, QStringLiteral("formFactor"));
    }
}

/*
 * Qt does not have handleScreen*Change functions for all properties of the QPlatformScreen. Therefore it is not
 * always possible to update an existing UbuntuScreen to reflect a new MirOutput. This checks if the current
 * UbuntuScreen can be updated to reflect the new MirOutput or not. If not, this instance needs to be destroyed
 * and a new instance created.
 */
bool UbuntuScreen::canUpdateMirOutput(const MirOutput *output) const
{
    // DevicePixelRatio is not a notifyable property, so requires destroy/create
    if (overrideDevicePixelRatio) {
        return false;
    }

    const auto scale = mir_output_get_scale_factor(output);
    return qFuzzyCompare(mDevicePixelRatio, devicePixelRatioFromScale(scale));
}

void UbuntuScreen::setAdditionalMirDisplayProperties(float /*scale*/, MirFormFactor /*formFactor*/, float dpi)
{
    if (mDpi != dpi) {
        mDpi = dpi;
        QWindowSystemInterface::handleScreenLogicalDotsPerInchChange(screen(), dpi, dpi);
    }
}

QDpi UbuntuScreen::logicalDpi() const
{
    if (mDpi > 0) {
        return QDpi(mDpi, mDpi);
    } else {
        return QPlatformScreen::logicalDpi();
    }
}

qreal UbuntuScreen::devicePixelRatioFromScale(const float scale) const
{
    // This is a rough heuristic to choose the best devicePixelRatio value based on the requested
    // grid unit value - with the ambition to have DPR as close to the scale as possible.
    const int gridUnit = static_cast<int>(scale * 8);
    if (gridUnit < 12) {
        return 1.0;
    } else if (gridUnit < 21) {
        return 2.0;
    } else if (gridUnit < 28) {
        return 3.0;
    } else {
        return 4.0; // not expecting anything bigger being needed just now
    }
}
