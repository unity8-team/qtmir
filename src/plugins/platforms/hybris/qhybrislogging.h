// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef QHYBRISLOGGING_H
#define QHYBRISLOGGING_H

// Logging macros to compile out debugging for release builds.
#define LOG(...) qDebug(__VA_ARGS__)
#define LOG_IF(cond,...) do { if (cond) qDebug(__VA_ARGS__); } while(0)
#define ASSERT(cond) ((!(cond)) ? qt_assert(#cond,__FILE__,__LINE__) : qt_noop())
#if defined(QHYBRIS_DEBUG)
#define DLOG(...) LOG(__VA_ARGS__)
#define DLOG_IF(cond,...) LOG_IF((cond), __VA_ARGS__)
#define DASSERT(cond) ASSERT((cond))
#else
#define DLOG(...) qt_noop()
#define DLOG_IF(cond,...) qt_noop()
#define DASSERT(cond) qt_noop()
#endif

#endif  // QHYBRISLOGGING_H
