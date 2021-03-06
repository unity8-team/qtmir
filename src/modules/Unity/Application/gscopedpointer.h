/*
 * Copyright (C) 2011-2014 Canonical, Ltd.
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
 *
 * Authors:
 * - Aurélien Gâteau <aurelien.gateau@canonical.com>
 */

#ifndef GSCOPEDPOINTER_H
#define GSCOPEDPOINTER_H

// Local

// Qt
#include <QScopedPointer>

// GLib
#include <glib-object.h>

/**
 * Helper class for GScopedPointer
 */
template <class T, void (*cleanup_fcn)(T*)>
class GScopedPointerDeleter
{
public:
    static void cleanup(T* ptr)
    {
        if (ptr) {
            cleanup_fcn(ptr);
        }
    }
};

/**
 * A GScopedPointer works like a QScopedPointer, except the cleanup static
 * method is replaced with a cleanup function, making it useful to handle C
 * structs whose cleanup function prototype is "void cleanup(T*)"
 *
 * Best way to use it is to define a typedef for your C struct, like this:
 *
 * typedef GScopedPointer<Foo, foo_free> GFooPointer;
 */
template <class T, void (*cleanup)(T*)>
class GScopedPointer : public QScopedPointer<T, GScopedPointerDeleter<T, cleanup> >
{
public:
    GScopedPointer(T* ptr = 0)
    : QScopedPointer<T, GScopedPointerDeleter<T, cleanup> >(ptr)
    {}
};


/**
 * Helper class for GObjectScopedPointer
 */
template <class T, void (*cleanup_fcn)(gpointer)>
class GObjectScopedPointerDeleter
{
public:
    static void cleanup(T* ptr)
    {
        if (ptr) {
            cleanup_fcn(ptr);
        }
    }
};

/**
 * A GObjectScopedPointer is similar to a GScopedPointer. The only difference
 * is its cleanup function signature is "void cleanup(gpointer)" and defaults to
 * g_object_unref(), making it useful for GObject-based classes.
 *
 * You can use it directly like this:
 *
 * GObjectScopedPointer<GFoo> foo;
 *
 * Or define a typedef for your class:
 *
 * typedef GObjectScopedPointer<GFoo> GFooPointer;
 *
 * Note: GObjectScopedPointer does *not* call gobject_ref() when assigned a
 * pointer.
 */
template <class T, void (*cleanup)(gpointer) = g_object_unref>
class GObjectScopedPointer : public QScopedPointer<T, GObjectScopedPointerDeleter<T, cleanup> >
{
public:
    GObjectScopedPointer(T* ptr = 0)
    : QScopedPointer<T, GObjectScopedPointerDeleter<T, cleanup> >(ptr)
    {}
};

// A Generic GObject pointer
typedef GObjectScopedPointer<GObject> GObjectPointer;

// Take advantage of the cleanup signature of GObjectScopedPointer to define
// a GCharPointer
typedef GObjectScopedPointer<gchar, g_free> GCharPointer;

#endif /* GSCOPEDPOINTER_H */

