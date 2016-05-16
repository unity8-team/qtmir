/*
 * Copyright (C) 2015 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License, as
 * published by the  Free Software Foundation; either version 2.1 or 3.0
 * of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the applicable version of the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of both the GNU Lesser General Public
 * License along with this program. If not, see <http://www.gnu.org/licenses/>
 */

#ifndef QTMIR_TIMESOURCE_H
#define QTMIR_TIMESOURCE_H

#include <QSharedPointer>

namespace qtmir {
/*
    Interface for a time source.
 */
class TimeSource {
public:
    virtual ~TimeSource() {}
    /* Returns the current time in milliseconds since some reference time in the past. */
    virtual qint64 msecsSinceReference() = 0;
};
typedef QSharedPointer<TimeSource> SharedTimeSource;

/*
    Implementation of a time source
 */
class RealTimeSourcePrivate;
class RealTimeSource : public TimeSource {
public:
    RealTimeSource();
    virtual ~RealTimeSource();
    qint64 msecsSinceReference() override;
private:
    RealTimeSourcePrivate *d;
};

/*
    A fake time source, useful for tests
 */
class FakeTimeSource : public TimeSource {
public:
    FakeTimeSource() { m_msecsSinceReference = 0; }
    qint64 msecsSinceReference() override { return m_msecsSinceReference; }
    qint64 m_msecsSinceReference;
};

} // namespace qtmir

#endif // QTMIR_TIMESOURCE_H
