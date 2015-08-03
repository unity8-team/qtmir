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

namespace {
QMap<QString,int> xcursorNameToQtShape;

void loadXCursorImages(XcursorImages *images, qtmir::CursorImageProvider *cursorImageProvider)
{
    cursorImageProvider->loadXCursor(images);
}
} // anonymous namespace

using namespace qtmir;

CursorImageProvider *CursorImageProvider::m_instance = nullptr;

CursorImage::CursorImage(XcursorImages *xcursorImgs)
    : xcursorImages(xcursorImgs)
{
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

    // TODO: do it in a worker thread and/or load it only when first needed
    loadCursors();
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

QImage CursorImageProvider::requestImage(const QString &cursorName, QSize *size, const QSize & /*requestedSize*/)
{
    QString actualCursorName;

    if (m_cursors.contains(cursorName)) {
        actualCursorName = cursorName;
    } else if (m_cursors.contains("left_ptr")) {
        actualCursorName = "left_ptr";
    } else {
        return QImage();
    }

    QImage image = m_cursors[actualCursorName]->qimage;
    size->setWidth(image.width());
    size->setWidth(image.height());

    return image;
}

void CursorImageProvider::loadCursors()
{
    xcursor_load_theme("DMZ-White", 32,
        [](XcursorImages* images, void *this_ptr) -> void
        {
            // Can't use lambda capture as this lambda is thunked to a C function ptr
            auto cursor = static_cast<qtmir::CursorImageProvider*>(this_ptr);
            loadXCursorImages(images, cursor);
        }, this);
}

void CursorImageProvider::loadXCursor(XcursorImages *xcursorImages)
{
    m_cursors[xcursorImages->name] = new CursorImage(xcursorImages);
}

QPoint CursorImageProvider::hotspot(const QString &cursorName)
{
    if (m_cursors.contains(cursorName)) {
        return m_cursors[cursorName]->hotspot;
    } else if (m_cursors.contains("left_ptr")) {
        return m_cursors["left_ptr"]->hotspot;
    } else {
        return QPoint(0,0);
    }
}
