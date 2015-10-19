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

#include <QtQuick/QSGNode>
#include <QtQuick/QSGTexture>

// MirSurfaceNode is a Qt scene graph node that renders a texture with
// a given filtering. It can be compared to the image node provided by
// QtQuick (although simpler) with the addition of a more efficient
// antialiasing technique that keeps the same quality.
//
// QtQuick provides 2 different antialiasing techniques. The first one
// is called multisampling and is proposed by the underlying
// hardware. It can be enabled at window creation time by defining a
// number of samples on the QSurfaceFormat associated to the created
// QtQuick scene (window). The efficiency of that technique is highly
// dependent on the hardware and often not adapted. The second
// technique is called vertex antialiasing, when the antialiasing
// property is set to true on a QtQuick item, QtQuick switches to that
// technique which adds 4 new vertices with additional attributes and
// lets the vertex shader accurately set their position and opacity
// fall-off. The drawback of this technique is that it forces the
// whole node to be rendered in the translucent pass of the renderer
// even though the whole content of the node is opaque, another
// drawback is that it oten prevents batching.
//
// The antialiasing technique provided by this node prevents both
// issues by creating 2 geometry nodes, a FillNode that is made to
// fill the content and an EdgeNode that is made to render the
// antialiased edge. The edge node uses exactly the same vertex-based
// algorithm than the default QtQuick nodes.
//
// This technique is perfectly adapted to the rendering of the Spread,
// where there is quite a lot of overdraw, by bringing back the
// ability for GPUs to use early-Z cull optimisations.
class MirSurfaceNode : public QSGNode
{
public:
    MirSurfaceNode(QSGTexture *texture, QSGTexture::Filtering filtering, bool antialiasing);

    void setFiltering(QSGTexture::Filtering filtering);
    void setAntialiasing(bool antialiasing);
    void updateGeometry(float width, float height);
    void updateMaterial();
};
