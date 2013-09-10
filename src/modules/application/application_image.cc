// This file is part of QtUbuntu, a set of Qt components for Ubuntu.
// Copyright © 2013 Canonical Ltd.
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 3, as published by
// the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
// SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "application_image.h"
#include "application.h"
#include "logging.h"
#include <QtGui/QPainter>
#include <QtCore/QCoreApplication>
#include <QtCore/QThread>
#include <ubuntu/ui/ubuntu_ui_session_service.h>

class ApplicationImageEvent : public QEvent {
 public:
  ApplicationImageEvent(QEvent::Type type, QImage image, const QRect& sourceRect)
      : QEvent(type)
      , image_(image)
      , sourceRect_(sourceRect) {
    DLOG("ApplicationImageEvent::ApplicationImageEvent (this=%p, type=%d)", this, type);
  }
  ~ApplicationImageEvent() {
    DLOG("ApplicationImageEvent::~ApplicationImageEvent");
  }
  static const QEvent::Type type_;
  QImage image_;
  QRect sourceRect_;
};

const QEvent::Type ApplicationImageEvent::type_ =
    static_cast<QEvent::Type>(QEvent::registerEventType());

static void snapshotCallback(const void* pixels, unsigned int bufferWidth, unsigned int bufferHeight,
                             unsigned int sourceX, unsigned int sourceY,
                             unsigned int sourceWidth, unsigned int sourceHeight,
                             unsigned int stride, void* context) {
  // FIXME(loicm) stride from Ubuntu Platform API is wrong.
  Q_UNUSED(stride);
  DLOG("snapshotCallback (pixels=%p, bufferWidth=%u, bufferHeight=%u, sourceX=%u, sourceY=%u, sourceWidth=%u, sourceHeight=%u, stride=%u, context=%p)",
       pixels, bufferWidth, bufferHeight, sourceX, sourceY, sourceHeight, sourceHeight, stride, context);
  DASSERT(context != NULL);
  // Copy the pixels and post an event to the GUI thread so that we can safely schedule an update.
  ApplicationImage* applicationImage = static_cast<ApplicationImage*>(context);

  if (pixels == NULL || bufferWidth == 0 || bufferHeight == 0) {
      QCoreApplication::postEvent(applicationImage, new ApplicationImageEvent(
                                  ApplicationImageEvent::type_, QImage(), QRect()));
  } else {
    QRect sourceRect(sourceX, sourceY, sourceWidth, sourceHeight);
    QImage image(static_cast<const uchar*>(pixels), bufferWidth, bufferHeight, bufferWidth * 4,
                 QImage::Format_ARGB32_Premultiplied);
    QCoreApplication::postEvent(applicationImage, new ApplicationImageEvent(
          ApplicationImageEvent::type_, image.rgbSwapped(), sourceRect));
  }
}

QHash<Application*, QPair<QImage, QRect> > ApplicationImage::imageCache_;

ApplicationImage::ApplicationImage(QQuickPaintedItem* parent)
    : QQuickPaintedItem(parent)
    , source_(NULL)
    , fillMode_(Stretch)
    , ready_(false) {
  DLOG("ApplicationImage::ApplicationImage (this=%p, parent=%p)", this, parent);
  setRenderTarget(QQuickPaintedItem::FramebufferObject);
  setFillColor(QColor(0, 0, 0, 255));
  setOpaquePainting(true);
}

ApplicationImage::~ApplicationImage() {
  DLOG("ApplicationImage::~ApplicationImage");
}

void ApplicationImage::customEvent(QEvent* event) {
  DLOG("ApplicationImage::customEvent (this=%p, event=%p)", this, event);
  DASSERT(QThread::currentThread() == thread());
  ApplicationImageEvent* imageEvent = static_cast<ApplicationImageEvent*>(event);
  // Store the new image and schedule an update.
  image_ = imageEvent->image_;
  sourceRect_ = imageEvent->sourceRect_;
  DLOG("ApplicationImage: inserted image in cache (this=%p)", this);
  imageCache_.insert(source_, QPair<QImage, QRect>(image_, sourceRect_));
  connect(source_, &Application::destroyed,
          this, &ApplicationImage::onSourceDestroyed, Qt::UniqueConnection);
  update();
  if (!ready_) {
    ready_ = true;
    emit readyChanged();
  }
}

void ApplicationImage::setSource(Application* source) {
  DLOG("ApplicationImage::setApplication (this=%p, source=%p)", this, source);
  if (source_ != source) {
    source_ = source;
    image_ = QImage();
    sourceRect_ = QRect();
    if (ready_) {
      ready_ = false;
      emit readyChanged();
    }
    emit sourceChanged();
  }
}

void ApplicationImage::setFillMode(FillMode fillMode) {
  DLOG("ApplicationImage::setApplication (this=%p, fillMode=%d)", this, fillMode);
  if (fillMode_ != fillMode) {
    fillMode_ = fillMode;
    update();
    emit fillModeChanged();
  }
}

void ApplicationImage::scheduleUpdate() {
  DLOG("ApplicationImage::scheduleUpdate (this=%p)", this);
  if (ready_) {
    ready_ = false;
    emit readyChanged();
  }
  if (source_ != NULL && source_->state() == Application::Running)
    ubuntu_ui_session_snapshot_running_session_with_id(source_->pid(), snapshotCallback, this);
  else {
    update();
  }
}

void ApplicationImage::updateFromCache() {
  DLOG("ApplicationImage::updateFromCache (this=%p)", this);
  if (imageCache_.contains(source_)) {
    DLOG("ApplicationImage: using image from cache (this=%p)", this);
    QPair<QImage, QRect> value = imageCache_.value(source_);
    if (image_ != value.first || sourceRect_ != value.second) {
      image_ = value.first;
      sourceRect_ = value.second;
      update();
    }
    if (!ready_) {
      ready_ = true;
      emit readyChanged();
    }
  }
}


void ApplicationImage::paint(QPainter* painter) {
  DLOG("ApplicationImage::paint (this=%p, painter=%p)", this, painter);
  if (source_ != NULL && !image_.isNull() && sourceRect_.isValid()
      && source_->state() == Application::Running) {
    painter->setCompositionMode(QPainter::CompositionMode_Source);

    QRect targetRect;
    QRect sourceRect;

    switch(fillMode_) {
      case Stretch:
        targetRect = QRect(0, 0, width(), height());
        sourceRect = sourceRect_;
        break;
      case PreserveAspectCrop:
        // assume AlignTop and AlignLeft alignment
        targetRect = QRect(0, 0, width(), height());
        float widthScale = width() / float(sourceRect_.width());
        float heightScale = height() / float(sourceRect_.height());

        if (widthScale > heightScale) {
          int croppedHeight = height() / widthScale;
          sourceRect = QRect(sourceRect_.x(), sourceRect_.y(), sourceRect_.width(), croppedHeight);
        } else {
          int croppedWidth = width() / heightScale;
          sourceRect = QRect(sourceRect_.x(), sourceRect_.y(), croppedWidth, sourceRect_.height());
        }
        break;
    }

    painter->drawImage(targetRect, image_, sourceRect);
  }
}

void ApplicationImage::onSourceDestroyed() {
  DLOG("ApplicationImage::onSourceDestroyed (this=%p)", this);
  if (imageCache_.remove(source_) != 0) {
    DLOG("ApplicationImage: removed image from cache (this=%p)", this);
  }
  image_ = QImage();
  sourceRect_ = QRect();
  if (ready_) {
    ready_ = false;
    emit readyChanged();
  }
  if (source_ != NULL) {
    source_ = NULL;
    emit sourceChanged();
  }
}
