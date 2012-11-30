// Copyright © 2012 Canonical Ltd
// FIXME(loicm) Add copyright notice here.

#ifndef UBUNTU_APPLICATION_PLUGIN_LOGGING_H
#define UBUNTU_APPLICATION_PLUGIN_LOGGING_H

// Logging and assertion macros.
#define LOG(...) qDebug(__VA_ARGS__)
#define LOG_IF(cond,...) do { if (cond) qDebug(__VA_ARGS__); } while(0)
#define ASSERT(cond) ((!(cond)) ? qt_assert(#cond,__FILE__,__LINE__) : qt_noop())
#define NOT_REACHED() qt_assert("Not reached!",__FILE__,__LINE__)

// Logging and assertion macros are compiled out for release builds.
#if !defined(QT_NO_DEBUG)
#define DLOG(...) LOG(__VA_ARGS__)
#define DLOG_IF(cond,...) LOG_IF((cond), __VA_ARGS__)
#define DASSERT(cond) ASSERT((cond))
#define DNOT_REACHED() NOT_REACHED()
#else
#define DLOG(...) qt_noop()
#define DLOG_IF(cond,...) qt_noop()
#define DASSERT(cond) qt_noop()
#define DNOT_REACHED() qt_noop()
#endif

#endif  // UBUNTU_APPLICATION_PLUGIN_LOGGING_H
