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

#ifndef QUBUNTULOGGING_H
#define QUBUNTULOGGING_H

#include <QLoggingCategory>

#define ASSERT(cond) ((!(cond)) ? qt_assert(#cond,__FILE__,__LINE__) : qt_noop())

Q_DECLARE_LOGGING_CATEGORY(ubuntumirclient)
Q_DECLARE_LOGGING_CATEGORY(ubuntumirclientBufferSwap)
Q_DECLARE_LOGGING_CATEGORY(ubuntumirclientInput)

#endif  // QUBUNTULOGGING_H
