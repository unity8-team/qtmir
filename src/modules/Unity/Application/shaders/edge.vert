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

// Uses the vertex antialiasing code from QtQuick.

uniform highp mat4 matrix;
uniform lowp float opacity;
uniform highp vec2 pixelSize;
attribute highp vec4 positionAttrib;
attribute mediump vec2 textureCoordAttrib;
attribute highp vec2 positionOffsetAttrib;
attribute highp vec2 textureCoordOffsetAttrib;
varying mediump vec2 textureCoord;
varying lowp float vertexOpacity;

void main()
{
    textureCoord = textureCoordAttrib;
    highp vec4 pos = matrix * positionAttrib;
    gl_Position = pos;

    if (positionOffsetAttrib.x != 0.0) {
        highp vec4 delta = matrix[0] * positionOffsetAttrib.x;
        highp vec2 dir = delta.xy * pos.w - pos.xy * delta.w;
        highp vec2 ndir = 0.5 * pixelSize * normalize(dir / pixelSize);
        dir -= ndir * delta.w * pos.w;
        highp float numerator = dot(dir, ndir * pos.w * pos.w);
        highp float scale = 0.0;
        if (numerator < 0.0) {
            scale = 1.0;
        } else {
            scale = min(1.0, numerator / dot(dir, dir));
        }
        gl_Position += scale * delta;
        textureCoord.x += scale * textureCoordOffsetAttrib.x;
    }

    if (positionOffsetAttrib.y != 0.0) {
        highp vec4 delta = matrix[1] * positionOffsetAttrib.y;
        highp vec2 dir = delta.xy * pos.w - pos.xy * delta.w;
        highp vec2 ndir = 0.5 * pixelSize * normalize(dir / pixelSize);
        dir -= ndir * delta.w * pos.w;
        highp float numerator = dot(dir, ndir * pos.w * pos.w);
        highp float scale = 0.0;
        if (numerator < 0.0) {
            scale = 1.0;
        } else {
            scale = min(1.0, numerator / dot(dir, dir));
        }
        gl_Position += scale * delta;
        textureCoord.y += scale * textureCoordOffsetAttrib.y;
    }

    bool onEdge = any(notEqual(positionOffsetAttrib, vec2(0.0)));
    bool outerEdge = all(equal(textureCoordOffsetAttrib, vec2(0.0)));
    if (onEdge && outerEdge) {
        vertexOpacity = 0.0;
    } else {
        vertexOpacity = opacity;
    }
}
