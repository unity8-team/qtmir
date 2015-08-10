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

#include "cursorimageprovider.h"

#include <QFile>

using namespace qtmir;

CursorImageProvider *CursorImageProvider::m_instance = nullptr;

CursorImage::CursorImage(const QString &theme, const QString &file)
    : xcursorImages(nullptr)
{

    xcursorImages = XcursorLibraryLoadImages(QFile::encodeName(file), QFile::encodeName(theme), 32);
    if (!xcursorImages) {
        return;
    }

    bool loaded = false;
    for (int i = 0; i < xcursorImages->nimage && !loaded; ++i) {
        XcursorImage *xcursorImage = xcursorImages->images[i];
        if (xcursorImage->size == 32) {

            qimage = QImage((uchar*)xcursorImage->pixels,
                    xcursorImage->width, xcursorImage->height, QImage::Format_ARGB32);

            hotspot.setX(xcursorImage->xhot);
            hotspot.setY(xcursorImage->yhot);

            loaded = true;
        }
    }
}

CursorImage::~CursorImage()
{
    XcursorImagesDestroy(xcursorImages);
}

CursorImageProvider::CursorImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
    if (m_instance) {
        qFatal("QPA mirsever: cannot have multiple CursorImageProvider instances");
    }
    m_instance = this;
}

CursorImageProvider::~CursorImageProvider()
{
    {
        QList<CursorImage*> cursorImageList = m_cursors.values();
        for (int i = 0; i < cursorImageList.count(); ++i) {
            delete cursorImageList[i];
        }
    }
    m_cursors.clear();
    m_instance = nullptr;
}

QImage CursorImageProvider::requestImage(const QString &cursorThemeAndName, QSize *size, const QSize & /*requestedSize*/)
{
    CursorImage *cursorImage = fetchCursor(cursorThemeAndName);
    size->setWidth(cursorImage->qimage.width());
    size->setHeight(cursorImage->qimage.height());

    return cursorImage->qimage;
}

QPoint CursorImageProvider::hotspot(const QString &themeName, const QString &cursorName)
{
    QString themeAndCursor = themeName + "/" + cursorName;
    CursorImage *cursorImage = fetchCursor(themeAndCursor);
    if (cursorImage) {
        return cursorImage->hotspot;
    } else {
        return QPoint(0,0);
    }
}

CursorImage *CursorImageProvider::fetchCursor(const QString &cursorThemeAndName)
{
    QString themeName;
    QString cursorName;
    {
        QStringList themeAndNameList = cursorThemeAndName.split("/");
        if (themeAndNameList.size() != 2) {
            return nullptr;
        }
        themeName = themeAndNameList[0];
        cursorName = themeAndNameList[1];
    }

    CursorImage *cursorImage = nullptr;

    if (!m_cursors.contains(cursorThemeAndName)) {
        m_cursors[cursorThemeAndName] = new CursorImage(themeName, cursorName);
    }
    cursorImage = m_cursors[cursorThemeAndName];

    if (cursorImage->qimage.isNull()) {
        if (!m_cursors.contains("default/left_ptr")) {
            m_cursors["default/left_ptr"] = new CursorImage("default", "left_ptr");
        }
        cursorImage = m_cursors["default/left_ptr"];
    }

    return cursorImage;
}
