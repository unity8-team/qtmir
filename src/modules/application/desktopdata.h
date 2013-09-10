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

#ifndef DESKTOPDATA_H
#define DESKTOPDATA_H

#include <QString>
#include <QVector>
#include <QUrl>

class DesktopData {
public:
    DesktopData(QString appId);
    ~DesktopData();

    QString appId() const { return appId_; }
    QString file() const;
    QString name() const { return entries_[kNameIndex]; }
    QString comment() const { return entries_[kCommentIndex]; }
    QUrl icon() const { return QUrl(entries_[kIconIndex]); }
    QString exec() const { return entries_[kExecIndex]; }
    QString path() const { return entries_[kPathIndex]; }
    QString stageHint() const { return entries_[kStageHintIndex]; }
    bool loaded() const { return loaded_; }

private:
    static const int kNameIndex = 0,
    kCommentIndex = 1,
    kIconIndex = 2,
    kExecIndex = 3,
    kPathIndex = 4,
    kStageHintIndex = 5,
    kNumberOfEntries = 6;

    bool load();

    QString appId_;
    QVector<QString> entries_;
    bool loaded_;
};

#endif // DESKTOPDATA_H
