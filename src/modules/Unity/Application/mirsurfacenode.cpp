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

#include "mirsurfacenode.h"
#include <QtQuick/QSGMaterial>

// --- Vertex layouts ---

struct Vertex {
    float position[2];
    float textureCoordinate[2];
};

struct OffsetVertex {
    float position[2];
    float textureCoordinate[2];
    float positionOffset[2];
    float textureCoordinateOffset[2];
};

// --- Shader classes ---

class OpaqueFillShader : public QSGMaterialShader
{
public:
    OpaqueFillShader();
    virtual char const *const *attributeNames() const;
    virtual void initialize();
    virtual void updateState(
        const RenderState &state, QSGMaterial *newEffect, QSGMaterial *oldEffect);

private:
    int m_matrixId;
};

class FillShader : public OpaqueFillShader
{
public:
    FillShader();
    virtual void initialize();
    virtual void updateState(
        const RenderState &state, QSGMaterial *newEffect, QSGMaterial *oldEffect);

private:
    int m_opacityId;
};

class OffsetOpaqueFillShader : public OpaqueFillShader
{
public:
    OffsetOpaqueFillShader();
    virtual char const *const *attributeNames() const;
    virtual void initialize();
    virtual void updateState(
        const RenderState &state, QSGMaterial *newEffect, QSGMaterial *oldEffect);

private:
    int m_pixelSizeId;
};

class OffsetFillShader : public FillShader
{
public:
    OffsetFillShader();
    virtual char const *const *attributeNames() const;
    virtual void initialize();
    virtual void updateState(
        const RenderState &state, QSGMaterial *newEffect, QSGMaterial *oldEffect);

private:
    int m_pixelSizeId;
};

class EdgeShader : public OffsetFillShader
{
public:
    EdgeShader();
};

// --- Material classes ---

class OpaqueFillMaterial : public QSGMaterial
{
public:
    OpaqueFillMaterial(QSGTexture *texture, QSGTexture::Filtering filtering);
    virtual QSGMaterialType *type() const;
    virtual QSGMaterialShader *createShader() const;
    virtual int compare(const QSGMaterial *other) const;

    QSGTexture *texture() const { return m_texture; }
    QSGTexture::Filtering filtering() const { return m_filtering; }
    void setFiltering(QSGTexture::Filtering filtering) { m_filtering = filtering; }

private:
    QSGTexture *m_texture;
    QSGTexture::Filtering m_filtering;
};

class FillMaterial : public OpaqueFillMaterial
{
public:
    FillMaterial(QSGTexture *texture, QSGTexture::Filtering filtering);
    virtual QSGMaterialType *type() const;
    virtual QSGMaterialShader *createShader() const;
};

class OffsetOpaqueFillMaterial : public OpaqueFillMaterial
{
public:
    OffsetOpaqueFillMaterial(QSGTexture *texture, QSGTexture::Filtering filtering);
    virtual QSGMaterialType *type() const;
    virtual QSGMaterialShader *createShader() const;
};

class OffsetFillMaterial : public FillMaterial
{
public:
    OffsetFillMaterial(QSGTexture *texture, QSGTexture::Filtering filtering);
    virtual QSGMaterialType *type() const;
    virtual QSGMaterialShader *createShader() const;
};

class EdgeMaterial : public OpaqueFillMaterial
{
public:
    EdgeMaterial(QSGTexture *texture, QSGTexture::Filtering filtering);
    virtual QSGMaterialType *type() const;
    virtual QSGMaterialShader *createShader() const;
};

// --- Node classes ---

class FillNode : public QSGGeometryNode
{
public:
    FillNode(QSGTexture *texture, QSGTexture::Filtering filtering, bool offset);

    void setOffset(bool offset);
    void setFiltering(QSGTexture::Filtering filtering);
    void updateGeometry(float width, float height);

private:
    static const unsigned short *indices();
    static const QSGGeometry::AttributeSet &attributeSet();
    static const QSGGeometry::AttributeSet &offsetAttributeSet();

    enum { Offset = (1 << 0) };

    OpaqueFillMaterial m_opaqueMaterial;
    FillMaterial m_material;
    OffsetOpaqueFillMaterial m_offsetOpaqueMaterial;
    OffsetFillMaterial m_offsetMaterial;
    QSGGeometry m_geometry;
    QSGGeometry m_offsetGeometry;
    quint8 m_flags;
};

class EdgeNode : public QSGGeometryNode
{
public:
    EdgeNode(QSGTexture *texture, QSGTexture::Filtering filtering, bool visible);
    virtual bool isSubtreeBlocked() const { return !!(m_flags & Visible); }

    void setVisible(bool visible);
    void setFiltering(QSGTexture::Filtering filtering);
    void updateGeometry(float width, float height);

private:
    static const unsigned short *indices();
    static const QSGGeometry::AttributeSet &attributeSet();

    enum { Visible = (1 << 0) };

    EdgeMaterial m_material;
    QSGGeometry m_geometry;
    quint8 m_flags;
};

// --- Opaque fill shader ---

OpaqueFillShader::OpaqueFillShader()
{
    setShaderSourceFile(QOpenGLShader::Vertex, QStringLiteral(":/shaders/fill.vert"));
    setShaderSourceFile(QOpenGLShader::Fragment, QStringLiteral(":/shaders/opaquefill.frag"));
}

char const *const *OpaqueFillShader::attributeNames() const
{
    static char const *const attributes[] = {
        "positionAttrib", "textureCoordAttrib", 0
    };
    return attributes;
}

void OpaqueFillShader::initialize()
{
    QSGMaterialShader::initialize();
    program()->bind();
    program()->setUniformValue("texture", 0);
    m_matrixId = program()->uniformLocation("matrix");
}

void OpaqueFillShader::updateState(
    const RenderState &state, QSGMaterial *newEffect, QSGMaterial *oldEffect)
{
    Q_UNUSED(oldEffect);

    // Bind and update texturing states. Note that (as mentioned in the scene
    // graph doc and as it is also expected in the standard texture materials)
    // getting a NULL texture pointer here is the result of a buggy node
    // implementation.
    OpaqueFillMaterial *material = static_cast<OpaqueFillMaterial*>(newEffect);
    QSGTexture *texture = material->texture();
    Q_ASSERT_X(texture, "updateState", "NULL texture pointer, node must be fixed");
    texture->setFiltering(material->filtering());
    texture->bind();

    if (state.isMatrixDirty()) {
        program()->setUniformValue(m_matrixId, state.combinedMatrix());
    }
}

// --- Fill shader ---

FillShader::FillShader()
    : OpaqueFillShader()
{
    setShaderSourceFile(QOpenGLShader::Fragment, QStringLiteral(":/shaders/fill.frag"));
}

void FillShader::initialize()
{
    OpaqueFillShader::initialize();
    m_opacityId = program()->uniformLocation("opacity");
}

void FillShader::updateState(
    const RenderState &state, QSGMaterial *newEffect, QSGMaterial *oldEffect)
{
    OpaqueFillShader::updateState(state, newEffect, oldEffect);
    if (state.isOpacityDirty()) {
        program()->setUniformValue(m_opacityId, state.opacity());
    }
}

// --- Offset opaque fill shader ---

OffsetOpaqueFillShader::OffsetOpaqueFillShader()
    : OpaqueFillShader()
{
    setShaderSourceFile(QOpenGLShader::Vertex, QStringLiteral(":/shaders/offsetfill.vert"));
}

char const *const *OffsetOpaqueFillShader::attributeNames() const
{
    static char const *const attributes[] = {
        "positionAttrib", "textureCoordAttrib", "positionOffsetAttrib", "textureCoordOffsetAttrib",
        0
    };
    return attributes;
}

void OffsetOpaqueFillShader::initialize()
{
    OpaqueFillShader::initialize();
    m_pixelSizeId = program()->uniformLocation("pixelSize");
}

void OffsetOpaqueFillShader::updateState(
    const RenderState &state, QSGMaterial *newEffect, QSGMaterial *oldEffect)
{
    OpaqueFillShader::updateState(state, newEffect, oldEffect);
    if (!oldEffect) {
        // Set the pixel size only once since the viewport is constant.
        const QRect rect = state.viewportRect();
        program()->setUniformValue(m_pixelSizeId, 2.0f / rect.width(), 2.0f / rect.height());
    }
}

// --- Offset fill shader ---

OffsetFillShader::OffsetFillShader()
    : FillShader()
{
    setShaderSourceFile(QOpenGLShader::Vertex, QStringLiteral(":/shaders/offsetfill.vert"));
}

char const *const *OffsetFillShader::attributeNames() const
{
    static char const *const attributes[] = {
        "positionAttrib", "textureCoordAttrib", "positionOffsetAttrib", "textureCoordOffsetAttrib",
        0
    };
    return attributes;
}

void OffsetFillShader::initialize()
{
    FillShader::initialize();
    m_pixelSizeId = program()->uniformLocation("pixelSize");
}

void OffsetFillShader::updateState(
    const RenderState &state, QSGMaterial *newEffect, QSGMaterial *oldEffect)
{
    FillShader::updateState(state, newEffect, oldEffect);
    if (!oldEffect) {
        // Set the pixel size only once since the viewport is constant.
        const QRect rect = state.viewportRect();
        program()->setUniformValue(m_pixelSizeId, 2.0f / rect.width(), 2.0f / rect.height());
    }
}

// --- Edge shader ---

EdgeShader::EdgeShader()
    : OffsetFillShader()
{
    setShaderSourceFile(QOpenGLShader::Vertex, QStringLiteral(":/shaders/edge.vert"));
    setShaderSourceFile(QOpenGLShader::Fragment, QStringLiteral(":/shaders/edge.frag"));
}

// --- Opaque fill material ---

OpaqueFillMaterial::OpaqueFillMaterial(QSGTexture *texture, QSGTexture::Filtering filtering)
    : m_texture(texture)
    , m_filtering(filtering)
{
    Q_ASSERT(texture);

    // It might seem a bit counterintuitive to set the blending flag in an
    // opaque material so here's a quick explanation. A geometry node can have a
    // standard and an opaque material, the latter one being used by the
    // renderer when the accumulated opacity on a node is 1. The point is to
    // simplify the shader used in that particular case. That said, an opaque
    // material can still render translucent fragments coming, for instance,
    // from an RGBA texture. In that case blending must be enabled and this is
    // exactly what happens here.
    setFlag(Blending, texture->hasAlphaChannel());
}

QSGMaterialType *OpaqueFillMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader *OpaqueFillMaterial::createShader() const
{
    return new OpaqueFillShader;
}

int OpaqueFillMaterial::compare(const QSGMaterial *other) const
{
    const OpaqueFillMaterial *otherMaterial = static_cast<const OpaqueFillMaterial*>(other);
    if (const int diff = m_texture->textureId() - otherMaterial->texture()->textureId()) {
        return diff;
    }
    return static_cast<int>(m_filtering) - static_cast<int>(otherMaterial->filtering());
}

// --- Fill material ---

FillMaterial::FillMaterial(QSGTexture *texture, QSGTexture::Filtering filtering)
    : OpaqueFillMaterial(texture, filtering)
{
}

QSGMaterialType *FillMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader *FillMaterial::createShader() const
{
    return new FillShader;
}

/// --- Offset opaque fill material ---

OffsetOpaqueFillMaterial::OffsetOpaqueFillMaterial(
    QSGTexture *texture, QSGTexture::Filtering filtering)
    : OpaqueFillMaterial(texture, filtering)
{
    setFlag(RequiresFullMatrixExceptTranslate, true);
}

QSGMaterialType *OffsetOpaqueFillMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader *OffsetOpaqueFillMaterial::createShader() const
{
    return new OffsetOpaqueFillShader;
}

/// --- Offset fill material ---

OffsetFillMaterial::OffsetFillMaterial(QSGTexture *texture, QSGTexture::Filtering filtering)
    : FillMaterial(texture, filtering)
{
    setFlag(RequiresFullMatrixExceptTranslate, true);
}

QSGMaterialType *OffsetFillMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader *OffsetFillMaterial::createShader() const
{
    return new OffsetFillShader;
}

// --- Edge material ---

EdgeMaterial::EdgeMaterial(QSGTexture *texture, QSGTexture::Filtering filtering)
    : OpaqueFillMaterial(texture, filtering)
{
    setFlag(Blending | RequiresFullMatrixExceptTranslate, true);
}

QSGMaterialType *EdgeMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader *EdgeMaterial::createShader() const
{
    return new EdgeShader;
}

// --- Fill node ---

FillNode::FillNode(QSGTexture *texture, QSGTexture::Filtering filtering, bool offset)
    : QSGGeometryNode()
    , m_opaqueMaterial(texture, filtering)
    , m_material(texture, filtering)
    , m_offsetOpaqueMaterial(texture, filtering)
    , m_offsetMaterial(texture, filtering)
    , m_geometry(attributeSet(), 4, 4, GL_UNSIGNED_SHORT)
    , m_offsetGeometry(offsetAttributeSet(), 4, 4, GL_UNSIGNED_SHORT)
    , m_flags(0)
{
    Q_ASSERT(texture);

    memcpy(m_geometry.indexData(), indices(), 4 * sizeof(unsigned short));
    m_geometry.setIndexDataPattern(QSGGeometry::StaticPattern);
    memcpy(m_offsetGeometry.indexData(), indices(), 4 * sizeof(unsigned short));
    m_offsetGeometry.setIndexDataPattern(QSGGeometry::StaticPattern);
    setOffset(offset);

    qsgnode_set_description(this, QLatin1String("mirsurfacefill"));
}

// static
const unsigned short *FillNode::indices()
{
    // Triangle strip layout:
    // 0 - 1
    // | / |
    // 2 - 3
    static const unsigned short indices[] = { 0, 2, 1, 3 };
    return indices;
}

// static
const QSGGeometry::AttributeSet &FillNode::attributeSet()
{
    static const QSGGeometry::Attribute attributes[] = {
        QSGGeometry::Attribute::create(0, 2, GL_FLOAT, true),
        QSGGeometry::Attribute::create(1, 2, GL_FLOAT),
    };
    static const QSGGeometry::AttributeSet attributeSet = {
        2, sizeof(Vertex), attributes
    };
    return attributeSet;
}

// static
const QSGGeometry::AttributeSet &FillNode::offsetAttributeSet()
{
    static const QSGGeometry::Attribute attributes[] = {
        QSGGeometry::Attribute::create(0, 2, GL_FLOAT, true),
        QSGGeometry::Attribute::create(1, 2, GL_FLOAT),
        QSGGeometry::Attribute::create(2, 2, GL_FLOAT),
        QSGGeometry::Attribute::create(3, 2, GL_FLOAT)
    };
    static const QSGGeometry::AttributeSet attributeSet = {
        4, sizeof(OffsetVertex), attributes
    };
    return attributeSet;
}

void FillNode::setOffset(bool offset)
{
    if (!offset) {
        setGeometry(&m_geometry);
        setOpaqueMaterial(&m_opaqueMaterial);
        setMaterial(&m_material);
        m_flags &= ~Offset;
    } else {
        setGeometry(&m_offsetGeometry);
        setOpaqueMaterial(&m_offsetOpaqueMaterial);
        setMaterial(&m_offsetMaterial);
        m_flags |= Offset;
    }
}

void FillNode::setFiltering(QSGTexture::Filtering filtering)
{
    m_opaqueMaterial.setFiltering(filtering);
    m_material.setFiltering(filtering);
    m_offsetOpaqueMaterial.setFiltering(filtering);
    m_offsetMaterial.setFiltering(filtering);
    markDirty(DirtyMaterial);
}

void FillNode::updateGeometry(float width, float height)
{
    if (!(m_flags & Offset)) {
        Vertex *v = reinterpret_cast<Vertex*>(m_geometry.vertexData());
        v[0].position[0] = 0.0f;
        v[0].position[1] = 0.0f;
        v[0].textureCoordinate[0] = 0.0f;
        v[0].textureCoordinate[1] = 0.0f;
        v[1].position[0] = width;
        v[1].position[1] = 0.0f;
        v[1].textureCoordinate[0] = 1.0f;
        v[1].textureCoordinate[1] = 0.0f;
        v[2].position[0] = 0.0f;
        v[2].position[1] = height;
        v[2].textureCoordinate[0] = 0.0f;
        v[2].textureCoordinate[1] = 1.0f;
        v[3].position[0] = width;
        v[3].position[1] = height;
        v[3].textureCoordinate[0] = 1.0f;
        v[3].textureCoordinate[1] = 1.0f;
    } else {
        OffsetVertex *v = reinterpret_cast<OffsetVertex*>(m_offsetGeometry.vertexData());
        const float delta = qMin(width, height) * 0.5f;
        const float sx = 1.0f / width;
        const float sy = 1.0f / height;
        v[0].position[0] = 0.0f;
        v[0].position[1] = 0.0f;
        v[0].textureCoordinate[0] = 0.0f;
        v[0].textureCoordinate[1] = 0.0f;
        v[0].positionOffset[0] = delta;
        v[0].positionOffset[1] = delta;
        v[0].textureCoordinateOffset[0] = delta * sx;
        v[0].textureCoordinateOffset[1] = delta * sy;
        v[1].position[0] = width;
        v[1].position[1] = 0.0f;
        v[1].textureCoordinate[0] = 1.0f;
        v[1].textureCoordinate[1] = 0.0f;
        v[1].positionOffset[0] = -delta;
        v[1].positionOffset[1] = delta;
        v[1].textureCoordinateOffset[0] = -delta * sx;
        v[1].textureCoordinateOffset[1] = delta * sy;
        v[2].position[0] = 0.0f;
        v[2].position[1] = height;
        v[2].textureCoordinate[0] = 0.0f;
        v[2].textureCoordinate[1] = 1.0f;
        v[2].positionOffset[0] = delta;
        v[2].positionOffset[1] = -delta;
        v[2].textureCoordinateOffset[0] = delta * sx;
        v[2].textureCoordinateOffset[1] = -delta * sy;
        v[3].position[0] = width;
        v[3].position[1] = height;
        v[3].textureCoordinate[0] = 1.0f;
        v[3].textureCoordinate[1] = 1.0f;
        v[3].positionOffset[0] = -delta;
        v[3].positionOffset[1] = -delta;
        v[3].textureCoordinateOffset[0] = -delta * sx;
        v[3].textureCoordinateOffset[1] = -delta * sy;
    }

    markDirty(DirtyGeometry);
}

// --- Edge node ---

EdgeNode::EdgeNode(QSGTexture *texture, QSGTexture::Filtering filtering, bool visible)
    : QSGGeometryNode()
    , m_material(texture, filtering)
    , m_geometry(attributeSet(), 8, 10, GL_UNSIGNED_SHORT)
    , m_flags(0)
{
    Q_ASSERT(texture);

    memcpy(m_geometry.indexData(), indices(), 10 * sizeof(unsigned short));
    m_geometry.setIndexDataPattern(QSGGeometry::StaticPattern);
    setGeometry(&m_geometry);
    setMaterial(&m_material);
    setVisible(visible);

    qsgnode_set_description(this, QLatin1String("mirsurfaceedge"));
}

// static
const unsigned short* EdgeNode::indices()
{
    // Triangle strip layout:
    // 0 ----- 1
    // | 4 - 5 |
    // | |   | |
    // | 6 - 7 |
    // 2 ----- 3
    static const unsigned short indices[] = { 0, 4, 1, 5, 3, 7, 2, 6, 0, 4 };
    return indices;
}

// static
const QSGGeometry::AttributeSet& EdgeNode::attributeSet()
{
    static const QSGGeometry::Attribute attributes[] = {
        QSGGeometry::Attribute::create(0, 2, GL_FLOAT, true),
        QSGGeometry::Attribute::create(1, 2, GL_FLOAT),
        QSGGeometry::Attribute::create(2, 2, GL_FLOAT),
        QSGGeometry::Attribute::create(3, 2, GL_FLOAT)
    };
    static const QSGGeometry::AttributeSet attributeSet = {
        4, sizeof(OffsetVertex), attributes
    };
    return attributeSet;
}

void EdgeNode::setVisible(bool visible)
{
    if (!(m_flags & Visible) != visible) {
        m_flags ^= Visible;
        markDirty(DirtySubtreeBlocked);
    }
}

void EdgeNode::setFiltering(QSGTexture::Filtering filtering)
{
    m_material.setFiltering(filtering);
    markDirty(DirtyMaterial);
}

void EdgeNode::updateGeometry(float width, float height)
{
    OffsetVertex *v = reinterpret_cast<OffsetVertex*>(m_geometry.vertexData());
    const float delta = qMin(width, height) * 0.5f;
    const float sx = 1.0f / width;
    const float sy = 1.0f / height;
    for (int i = 0, d = -1; i <= 4; i += 4, d += 2) {
        v[i+0].position[0] = 0.0f;
        v[i+0].position[1] = 0.0f;
        v[i+0].textureCoordinate[0] = 0.0f;
        v[i+0].textureCoordinate[1] = 0.0f;
        v[i+0].positionOffset[0] = delta * d;
        v[i+0].positionOffset[1] = delta * d;
        v[i+0].textureCoordinateOffset[0] = d < 0 ? 0.0f : delta * d * sx;
        v[i+0].textureCoordinateOffset[1] = d < 0 ? 0.0f : delta * d * sy;
        v[i+1].position[0] = width;
        v[i+1].position[1] = 0.0f;
        v[i+1].textureCoordinate[0] = 1.0f;
        v[i+1].textureCoordinate[1] = 0.0f;
        v[i+1].positionOffset[0] = -delta * d;
        v[i+1].positionOffset[1] = delta * d;
        v[i+1].textureCoordinateOffset[0] = d < 0 ? 0.0f : -delta * d * sx;
        v[i+1].textureCoordinateOffset[1] = d < 0 ? 0.0f : delta * d * sy;
        v[i+2].position[0] = 0.0f;
        v[i+2].position[1] = height;
        v[i+2].textureCoordinate[0] = 0.0f;
        v[i+2].textureCoordinate[1] = 1.0f;
        v[i+2].positionOffset[0] = delta * d;
        v[i+2].positionOffset[1] = -delta * d;
        v[i+2].textureCoordinateOffset[0] = d < 0 ? 0.0f : delta * d * sx;
        v[i+2].textureCoordinateOffset[1] = d < 0 ? 0.0f : -delta * d * sy;
        v[i+3].position[0] = width;
        v[i+3].position[1] = height;
        v[i+3].textureCoordinate[0] = 1.0f;
        v[i+3].textureCoordinate[1] = 1.0f;
        v[i+3].positionOffset[0] = -delta * d;
        v[i+3].positionOffset[1] = -delta * d;
        v[i+3].textureCoordinateOffset[0] = d < 0 ? 0.0f : -delta * d * sx;
        v[i+3].textureCoordinateOffset[1] = d < 0 ? 0.0f : -delta * d * sy;
    }

    markDirty(DirtyGeometry);
}

// --- MirSurface node ---

MirSurfaceNode::MirSurfaceNode(
    QSGTexture *texture, QSGTexture::Filtering filtering, bool antialiasing)
{
    Q_ASSERT(texture);
    appendChildNode(new FillNode(texture, filtering, antialiasing));
    appendChildNode(new EdgeNode(texture, filtering, antialiasing));
}

void MirSurfaceNode::setAntialiasing(bool antialiasing)
{
    static_cast<FillNode*>(firstChild())->setOffset(antialiasing);
    static_cast<EdgeNode*>(lastChild())->setVisible(antialiasing);
}

void MirSurfaceNode::setFiltering(QSGTexture::Filtering filtering)
{
    static_cast<FillNode*>(firstChild())->setFiltering(filtering);
    static_cast<EdgeNode*>(lastChild())->setFiltering(filtering);
}

void MirSurfaceNode::updateGeometry(float width, float height)
{
    static_cast<FillNode*>(firstChild())->updateGeometry(width, height);
    static_cast<EdgeNode*>(lastChild())->updateGeometry(width, height);
}

void MirSurfaceNode::updateMaterial()
{
    firstChild()->markDirty(QSGNode::DirtyMaterial);
    lastChild()->markDirty(QSGNode::DirtyMaterial);
}
