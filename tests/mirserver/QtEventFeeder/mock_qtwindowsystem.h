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
 *
 */

#ifndef MOCK_QTWINDOWSYSTEM_H
#define MOCK_QTWINDOWSYSTEM_H

#include <qteventfeeder.h>
#include <gmock/gmock-generated-function-mockers.h>
#include <QWindow>

#define GMOCK_METHOD11_(tn, constness, ct, Method, ...) \
  GMOCK_RESULT_(tn, __VA_ARGS__) ct Method( \
      GMOCK_ARG_(tn, 1, __VA_ARGS__) gmock_a1, \
      GMOCK_ARG_(tn, 2, __VA_ARGS__) gmock_a2, \
      GMOCK_ARG_(tn, 3, __VA_ARGS__) gmock_a3, \
      GMOCK_ARG_(tn, 4, __VA_ARGS__) gmock_a4, \
      GMOCK_ARG_(tn, 5, __VA_ARGS__) gmock_a5, \
      GMOCK_ARG_(tn, 6, __VA_ARGS__) gmock_a6, \
      GMOCK_ARG_(tn, 7, __VA_ARGS__) gmock_a7, \
      GMOCK_ARG_(tn, 8, __VA_ARGS__) gmock_a8, \
      GMOCK_ARG_(tn, 9, __VA_ARGS__) gmock_a9, \
      GMOCK_ARG_(tn, 10, __VA_ARGS__) gmock_a10, \
      GMOCK_ARG_(tn, 11, __VA_ARGS__) gmock_a11) constness { \
    GTEST_COMPILE_ASSERT_((::std::tr1::tuple_size<                          \
        tn ::testing::internal::Function<__VA_ARGS__>::ArgumentTuple>::value \
            == 11), \
        this_method_does_not_take_11_arguments); \
    GMOCK_MOCKER_(11, constness, Method).SetOwnerAndName(this, #Method); \
    return GMOCK_MOCKER_(11, constness, Method).Invoke(gmock_a1, gmock_a2, \
        gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, \
        gmock_a9, gmock_a10, gmock_a11); \
  } \
    ::testing::MockSpec<__VA_ARGS__>& \
        gmock_##Method(GMOCK_MATCHER_(tn, 1, __VA_ARGS__) gmock_a1, \
                       GMOCK_MATCHER_(tn, 2, __VA_ARGS__) gmock_a2, \
                       GMOCK_MATCHER_(tn, 3, __VA_ARGS__) gmock_a3, \
                       GMOCK_MATCHER_(tn, 4, __VA_ARGS__) gmock_a4, \
                       GMOCK_MATCHER_(tn, 5, __VA_ARGS__) gmock_a5, \
                       GMOCK_MATCHER_(tn, 6, __VA_ARGS__) gmock_a6, \
                       GMOCK_MATCHER_(tn, 7, __VA_ARGS__) gmock_a7, \
                       GMOCK_MATCHER_(tn, 8, __VA_ARGS__) gmock_a8, \
                       GMOCK_MATCHER_(tn, 9, __VA_ARGS__) gmock_a9, \
                       GMOCK_MATCHER_(tn, 10, __VA_ARGS__) gmock_a10, \
                       GMOCK_MATCHER_(tn, 11, __VA_ARGS__) gmock_a11) constness { \
      GMOCK_MOCKER_(11, constness, Method).RegisterOwner(this); \
      return GMOCK_MOCKER_(11, constness, Method).With(gmock_a1, gmock_a2, \
          gmock_a3, gmock_a4, gmock_a5, gmock_a6, gmock_a7, gmock_a8, gmock_a9, \
          gmock_a10, gmock_a10, gmock_a11); \
    } \
    mutable ::testing::FunctionMocker<__VA_ARGS__> GMOCK_MOCKER_(11, constness, \
        Method)

#define MOCK_METHOD11(m, ...) GMOCK_METHOD11_(, , , m, __VA_ARGS__)


class MockQtWindowSystem : public QtEventFeeder::QtWindowSystemInterface {
public:
    MOCK_CONST_METHOD0(ready, bool());
    MOCK_METHOD1(setScreenController, void(const QSharedPointer<ScreenController> &));
    MOCK_METHOD1(getWindowForTouchPoint, QWindow*(const QPoint &point));
    MOCK_METHOD0(lastWindow, QWindow*());
    MOCK_METHOD0(focusedWindow, QWindow*());
    MOCK_METHOD1(registerTouchDevice, void(QTouchDevice* device));

    void handleExtendedKeyEvent(QWindow */*window*/, ulong /*timestamp*/, QEvent::Type /*type*/, int /*key*/,
            Qt::KeyboardModifiers /*modifiers*/,
            quint32 /*nativeScanCode*/, quint32 /*nativeVirtualKey*/,
            quint32 /*nativeModifiers*/,
            const QString& /*text*/ = QString(), bool /*autorep*/ = false,
            ushort /*count*/ = 1) {}

//    MOCK_METHOD11(handleExtendedKeyEvent, void(QWindow *window, ulong timestamp, QEvent::Type type, int key,
//            Qt::KeyboardModifiers modifiers,
//            quint32 nativeScanCode, quint32 nativeVirtualKey,
//            quint32 nativeModifiers,
//            const QString& text, bool autorep,
//            ushort count));
    MOCK_METHOD5(handleTouchEvent, void(QWindow *window, ulong timestamp, QTouchDevice *device,
            const QList<struct QWindowSystemInterface::TouchPoint> &points,
            Qt::KeyboardModifiers mods));
    MOCK_METHOD5(handleMouseEvent, void(QWindow *window, ulong, QPointF, Qt::MouseButton, Qt::KeyboardModifiers));
};

namespace testing
{

MATCHER(IsPressed, std::string(negation ? "isn't" : "is") + " pressed")
{
    return arg.state == Qt::TouchPointPressed;
}

MATCHER(IsReleased, std::string(negation ? "isn't" : "is") + " released")
{
    return arg.state == Qt::TouchPointReleased;
}

MATCHER(IsStationary, std::string(negation ? "isn't" : "is") + " stationary")
{
    return arg.state == Qt::TouchPointStationary;
}

MATCHER(StateIsMoved, "state " + std::string(negation ? "isn't" : "is") + " 'moved'")
{
    return arg.state == Qt::TouchPointMoved;
}

MATCHER_P(HasId, expectedId, "id " + std::string(negation ? "isn't " : "is ") + PrintToString(expectedId))
{
    return arg.id == expectedId;
}

} // namespace testing


#endif // MOCK_QTWINDOWSYSTEM_H
