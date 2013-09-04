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

#include "desktopdata.h"

#include <QFile>

#include "logging.h"

// Retrieves the size of an array at compile time.
#define ARRAY_SIZE(a) \
    ((sizeof(a) / sizeof(*(a))) / static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))

const QString desktopFilePath = "/usr/share/applications/";

DesktopData::DesktopData(QString appId)
    : appId_(appId)
    , entries_(DesktopData::kNumberOfEntries, "") {
    DLOG("DesktopData::DesktopData (this=%p, appId='%s')", this, appId.toLatin1().data());
    DASSERT(appId != NULL);
    loaded_ = load();
}

DesktopData::~DesktopData() {
    DLOG("DesktopData::~DesktopData");
    entries_.clear();
}

QString DesktopData::file() const {
    return desktopFilePath + appId_ + ".desktop";
}

bool DesktopData::load() {
    DLOG("DesktopData::load (this=%p, appId='%s')", this, qPrintable(appId_));

    const struct { const char* const name; int size; unsigned int flag; } kEntryNames[] = {
        { "Name=", sizeof("Name=") - 1, 1 << DesktopData::kNameIndex },
        { "Comment=", sizeof("Comment=") - 1, 1 << DesktopData::kCommentIndex },
        { "Icon=", sizeof("Icon=") - 1, 1 << DesktopData::kIconIndex },
        { "Exec=", sizeof("Exec=") - 1, 1 << DesktopData::kExecIndex },
        { "Path=", sizeof("Path=") - 1, 1 << DesktopData::kPathIndex },
        { "X-Ubuntu-StageHint=", sizeof("X-Ubuntu-StageHint=") - 1, 1 << DesktopData::kStageHintIndex }
    };
    const unsigned int kAllEntriesMask =
            (1 << DesktopData::kNameIndex) | (1 << DesktopData::kCommentIndex)
            | (1 << DesktopData::kIconIndex) | (1 << DesktopData::kExecIndex)
            | (1 << DesktopData::kPathIndex) | (1 << DesktopData::kStageHintIndex);
    const unsigned int kMandatoryEntriesMask =
            (1 << DesktopData::kNameIndex) | (1 << DesktopData::kIconIndex)
            | (1 << DesktopData::kExecIndex);
    const int kEntriesCount = ARRAY_SIZE(kEntryNames);
    const int kBufferSize = 256;
    static char buffer[kBufferSize];

    QFile file(this->file());

    // Open file.
    if (!file.open(QFile::ReadOnly | QIODevice::Text)) {
        DLOG("can't open file: %s", file.errorString().toLatin1().data());
        return false;
    }

    // Validate "magic key" (standard group header).
    if (file.readLine(buffer, kBufferSize) != -1) {
        if (strncmp(buffer, "[Desktop Entry]", sizeof("[Desktop Entry]" - 1))) {
            DLOG("not a desktop file");
            return false;
        }
    }

    int length;
    unsigned int entryFlags = 0;
    while ((length = file.readLine(buffer, kBufferSize)) != -1) {
        // Skip empty lines.
        if (length > 1) {
            // Stop when reaching unsupported next group header.
            if (buffer[0] == '[') {
                DLOG("reached next group header, leaving loop");
                break;
            }
            // Lookup entries ignoring duplicates if any.
            for (int i = 0; i < kEntriesCount; i++) {
                if (!strncmp(buffer, kEntryNames[i].name, kEntryNames[i].size)) {
                    if (~entryFlags & kEntryNames[i].flag) {
                        buffer[length-1] = '\0';
                        entries_[i] = QString::fromLatin1(&buffer[kEntryNames[i].size]);
                        entryFlags |= kEntryNames[i].flag;
                        break;
                    }
                }
            }
            // Stop when matching the right number of entries.
            if (entryFlags == kAllEntriesMask) {
                break;
            }
        }
    }

    // Check that the mandatory entries are set.
    if ((entryFlags & kMandatoryEntriesMask) == kMandatoryEntriesMask) {
        DLOG("loaded desktop file with name='%s', comment='%s', icon='%s', exec='%s', path='%s', stagehint='%s'",
             entries_[DesktopData::kNameIndex].toLatin1().data(),
                entries_[DesktopData::kCommentIndex].toLatin1().data(),
                entries_[DesktopData::kIconIndex].toLatin1().data(),
                entries_[DesktopData::kExecIndex].toLatin1().data(),
                entries_[DesktopData::kPathIndex].toLatin1().data(),
                entries_[DesktopData::kStageHintIndex].toLatin1().data());
        return true;
    } else {
        DLOG("not a valid desktop file, missing mandatory entries in the standard group header");
        return false;
    }
}
