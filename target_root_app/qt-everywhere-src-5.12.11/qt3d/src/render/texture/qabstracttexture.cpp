/****************************************************************************
**
** Copyright (C) 2015 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qabstracttexture.h"
#include "qabstracttexture_p.h"
#include <Qt3DRender/qabstracttextureimage.h>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/qpropertynodeaddedchange.h>
#include <Qt3DCore/qpropertynoderemovedchange.h>

QT_BEGIN_NAMESPACE

using namespace Qt3DCore;

namespace Qt3DRender {

QAbstractTexturePrivate::QAbstractTexturePrivate()
    : QNodePrivate()
    , m_target(QAbstractTexture::Target2D)
    , m_format(QAbstractTexture::Automatic)
    , m_width(1)
    , m_height(1)
    , m_depth(1)
    , m_autoMipMap(false)
    , m_minFilter(QAbstractTexture::Nearest)
    , m_magFilter(QAbstractTexture::Nearest)
    , m_status(QAbstractTexture::None)
    , m_maximumAnisotropy(1.0f)
    , m_comparisonFunction(QAbstractTexture::CompareLessEqual)
    , m_comparisonMode(QAbstractTexture::CompareNone)
    , m_layers(1)
    , m_samples(1)
{
}

QTextureGeneratorPtr QAbstractTexturePrivate::dataFunctor() const
{
    return m_dataFunctor;
}

void QAbstractTexturePrivate::setDataFunctor(const QTextureGeneratorPtr &generator)
{
    if (generator != m_dataFunctor) {
        m_dataFunctor = generator;
        auto change = Qt3DCore::QPropertyUpdatedChangePtr::create(m_id);
        change->setPropertyName("generator");
        change->setValue(QVariant::fromValue(generator));
        notifyObservers(change);
    }
}

/*!
    \class Qt3DRender::QAbstractTexture
    \inmodule Qt3DRender
    \since 5.5
    \brief A base class to be used to provide textures.

    The QAbstractTexture class shouldn't be used directly but rather
    through one of its subclasses. Each subclass implements a given texture
    target (2D, 2DArray, 3D, CubeMap ...) Each subclass provides a set of
    functors for each layer, cube map face and mipmap level. In turn the
    backend uses those functor to properly fill a corresponding OpenGL texture
    with data.
 */

/*!
    \enum Qt3DRender::QAbstractTexture::CubeMapFace

    This enum identifies the faces of a cube map texture
    \value CubeMapPositiveX     Specify the positive X face of a cube map
    \value CubeMapNegativeX     Specify the negative X face of a cube map
    \value CubeMapPositiveY     Specify the positive Y face of a cube map
    \value CubeMapNegativeY     Specify the negative Y face of a cube map
    \value CubeMapPositiveZ     Specify the positive Z face of a cube map
    \value CubeMapNegativeZ     Specify the negative Z face of a cube map
    \value AllFaces             Specify all the faces of a cube map

    \note AllFaces should only be used when a behavior needs to be applied to
    all the faces of a cubemap. This is the case for example when using a cube
    map as a texture attachment. Using AllFaces in the attachment specfication
    would result in all faces being bound to the attachment point. On the other
    hand, if a specific face is specified, the attachment would only be using
    the specified face.
*/

/*!
    \enum Qt3DRender::QAbstractTexture::TextureFormat

    This list describes all possible texture formats

    \value NoFormat
          GL_NONE
    \value Automatic
          automatically_determines_format
    \value R8_UNorm
          GL_R8
    \value RG8_UNorm
          GL_RG8
    \value RGB8_UNorm
          GL_RGB8
    \value RGBA8_UNorm
          GL_RGBA8
    \value R16_UNorm
          GL_R16
    \value RG16_UNorm
          GL_RG16
    \value RGB16_UNorm
          GL_RGB16
    \value RGBA16_UNorm
          GL_RGBA16
    \value R8_SNorm
          GL_R8_SNORM
    \value RG8_SNorm
          GL_RG8_SNORM
    \value RGB8_SNorm
          GL_RGB8_SNORM
    \value RGBA8_SNorm
          GL_RGBA8_SNORM
    \value R16_SNorm
          GL_R16_SNORM
    \value RG16_SNorm
          GL_RG16_SNORM
    \value RGB16_SNorm
          GL_RGB16_SNORM
    \value RGBA16_SNorm
          GL_RGBA16_SNORM
    \value R8U
          GL_R8UI
    \value RG8U
          GL_RG8UI
    \value RGB8U
          GL_RGB8UI
    \value RGBA8U
          GL_RGBA8UI
    \value R16U
          GL_R16UI
    \value RG16U
          GL_RG16UI
    \value RGB16U
          GL_RGB16UI
    \value RGBA16U
          GL_RGBA16UI
    \value R32U
          GL_R32UI
    \value RG32U
          GL_RG32UI
    \value RGB32U
          GL_RGB32UI
    \value RGBA32U
          GL_RGBA32UI
    \value R8I
          GL_R8I
    \value RG8I
          GL_RG8I
    \value RGB8I
          GL_RGB8I
    \value RGBA8I
          GL_RGBA8I
    \value R16I
          GL_R16I
    \value RG16I
          GL_RG16I
    \value RGB16I
          GL_RGB16I
    \value RGBA16I
          GL_RGBA16I
    \value R32I
          GL_R32I
    \value RG32I
          GL_RG32I
    \value RGB32I
          GL_RGB32I
    \value RGBA32I
          GL_RGBA32I
    \value R16F
          GL_R16F
    \value RG16F
          GL_RG16F
    \value RGB16F
          GL_RGB16F
    \value RGBA16F
          GL_RGBA16F
    \value R32F
          GL_R32F
    \value RG32F
          GL_RG32F
    \value RGB32F
          GL_RGB32F
    \value RGBA32F
          GL_RGBA32F
    \value RGB9E5
          GL_RGB9_E5
    \value RG11B10F
          GL_R11F_G11F_B10F
    \value RG3B2
          GL_R3_G3_B2
    \value R5G6B5
          GL_RGB565
    \value RGB5A1
          GL_RGB5_A1
    \value RGBA4
          GL_RGBA4
    \value RGB10A2
          GL_RGB10_A2UI
    \value D16
          GL_DEPTH_COMPONENT16
    \value D24
          GL_DEPTH_COMPONENT24
    \value D24S8
          GL_DEPTH24_STENCIL8
    \value D32
          GL_DEPTH_COMPONENT32
    \value D32F
          GL_DEPTH_COMPONENT32F
    \value D32FS8X24
          GL_DEPTH32F_STENCIL8
    \value RGB_DXT1
          GL_COMPRESSED_RGB_S3TC_DXT1_EXT
    \value RGBA_DXT1
          GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
    \value RGBA_DXT3
          GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
    \value RGBA_DXT5
          GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
    \value R_ATI1N_UNorm
          GL_COMPRESSED_RED_RGTC1
    \value R_ATI1N_SNorm
          GL_COMPRESSED_SIGNED_RED_RGTC1
    \value RG_ATI2N_UNorm
          GL_COMPRESSED_RG_RGTC2
    \value RG_ATI2N_SNorm
          GL_COMPRESSED_SIGNED_RG_RGTC2
    \value RGB_BP_UNSIGNED_FLOAT
          GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB
    \value RGB_BP_SIGNED_FLOAT
          GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB
    \value RGB_BP_UNorm
          GL_COMPRESSED_RGBA_BPTC_UNORM_ARB
    \value R11_EAC_UNorm
          GL_COMPRESSED_R11_EAC
    \value R11_EAC_SNorm
          GL_COMPRESSED_SIGNED_R11_EAC
    \value RG11_EAC_UNorm
          GL_COMPRESSED_RG11_EAC
    \value RG11_EAC_SNorm
          GL_COMPRESSED_SIGNED_RG11_EAC
    \value RGB8_ETC2
          GL_COMPRESSED_RGB8_ETC2
    \value SRGB8_ETC2
          GL_COMPRESSED_SRGB8_ETC2
    \value RGB8_PunchThrough_Alpha1_ETC2
          GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2
    \value SRGB8_PunchThrough_Alpha1_ETC2
          GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2
    \value RGBA8_ETC2_EAC
          GL_COMPRESSED_RGBA8_ETC2_EAC
    \value SRGB8_Alpha8_ETC2_EAC
          GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC
    \value RGB8_ETC1
          GL_ETC1_RGB8_OES
    \value SRGB8
          GL_SRGB8
    \value SRGB8_Alpha8
          GL_SRGB8_ALPHA8
    \value SRGB_DXT1
          GL_COMPRESSED_SRGB_S3TC_DXT1_EXT
    \value SRGB_Alpha_DXT1
          GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT
    \value SRGB_Alpha_DXT3
          GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT
    \value SRGB_Alpha_DXT5
          GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT
    \value SRGB_BP_UNorm
          GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB
    \value DepthFormat
          GL_DEPTH_COMPONENT
    \value AlphaFormat
          GL_ALPHA
    \value RGBFormat
          GL_RGB
    \value RGBAFormat
          GL_RGBA
    \value LuminanceFormat
          GL_LUMINANCE
    \value LuminanceAlphaFormat
          0x190A
*/

/*!
 * The constructor creates a new QAbstractTexture::QAbstractTexture
 * instance with the specified \a parent.
 */
QAbstractTexture::QAbstractTexture(QNode *parent)
    : QNode(*new QAbstractTexturePrivate, parent)
{
}

/*!
 * The constructor creates a new QAbstractTexture::QAbstractTexture
 * instance with the specified \a target and \a parent.
 */
QAbstractTexture::QAbstractTexture(Target target, QNode *parent)
    : QNode(*new QAbstractTexturePrivate, parent)
{
    d_func()->m_target = target;
}

/*! \internal */
QAbstractTexture::~QAbstractTexture()
{
}

/*! \internal */
QAbstractTexture::QAbstractTexture(QAbstractTexturePrivate &dd, QNode *parent)
    : QNode(dd, parent)
{
}

/*!
    Sets the size of the texture provider to width \a w, height \a h and depth \a d.
 */
void QAbstractTexture::setSize(int w, int h, int d)
{
    setWidth(w);
    setHeight(h);
    setDepth(d);
}

/*!
    \property Qt3DRender::QAbstractTexture::width

    Holds the width of the texture provider.
 */
/*!
    Set the width of the texture provider to \a width.
*/
void QAbstractTexture::setWidth(int width)
{
    Q_D(QAbstractTexture);
    if (d->m_width != width) {
        d->m_width = width;
        emit widthChanged(width);
    }
}

/*!
    \property Qt3DRender::QAbstractTexture::height

    Holds the height of the texture provider.
 */
/*!
    Set the height to \a height.
*/
void QAbstractTexture::setHeight(int height)
{
    Q_D(QAbstractTexture);
    if (d->m_height != height) {
        d->m_height = height;
        emit heightChanged(height);
    }
}

/*!
    \property Qt3DRender::QAbstractTexture::depth

    Holds the depth of the texture provider.
 */
/*!
    Set the depth of the texture to \a depth.
*/
void QAbstractTexture::setDepth(int depth)
{
    Q_D(QAbstractTexture);
    if (d->m_depth != depth) {
        d->m_depth = depth;
        emit depthChanged(depth);
    }
}

/*!
 * \return the width of the texture
 */
int QAbstractTexture::width() const
{
    Q_D(const QAbstractTexture);
    return d->m_width;
}

/*!
 * \return the height of the texture
 */
int QAbstractTexture::height() const
{
    Q_D(const QAbstractTexture);
    return d->m_height;
}

/*!
 * \return the depth of the texture
 */
int QAbstractTexture::depth() const
{
    Q_D(const QAbstractTexture);
    return d->m_depth;
}

/*!
    \property Qt3DRender::QAbstractTexture::layers

    Holds the maximum layer count of the texture provider. By default, the
    maximum layer count is 1.

    \note this has a meaning only for texture providers that have 3D or
    array target formats.
 */
/*!
    Set the maximum layer count to \a layers.
*/
void QAbstractTexture::setLayers(int layers)
{
    Q_D(QAbstractTexture);
    if (d->m_layers != layers) {
        d->m_layers = layers;
        emit layersChanged(layers);
    }
}

/*!
    \return the maximum number of layers for the texture provider.

    \note this has a meaning only for texture providers that have 3D or
     array target formats.
 */
int QAbstractTexture::layers() const
{
    Q_D(const QAbstractTexture);
    return d->m_layers;
}

/*!
    \property Qt3DRender::QAbstractTexture::samples

    Holds the number of samples per texel for the texture provider.
    By default, the number of samples is 1.

    \note this has a meaning only for texture providers that have multisample
    formats.
 */
/*!
    Set the number of samples per texel to \a samples.
*/
void QAbstractTexture::setSamples(int samples)
{
    Q_D(QAbstractTexture);
    if (d->m_samples != samples) {
        d->m_samples = samples;
        emit samplesChanged(samples);
    }
}

/*!
    \return the number of samples per texel for the texture provider.

    \note this has a meaning only for texture providers that have multisample
    formats.
 */
int QAbstractTexture::samples() const
{
    Q_D(const QAbstractTexture);
    return d->m_samples;
}

/*!
    \property Qt3DRender::QAbstractTexture::format

    Holds the format of the texture provider.
 */
/*!
    Set the texture format to \a format.
*/
void QAbstractTexture::setFormat(TextureFormat format)
{
    Q_D(QAbstractTexture);
    if (d->m_format != format) {
        d->m_format = format;
        emit formatChanged(format);
    }
}

/*!
    Returns the texture provider's format.
 */
QAbstractTexture::TextureFormat QAbstractTexture::format() const
{
    Q_D(const QAbstractTexture);
    return d->m_format;
}

/*!
    \property Qt3DRender::QAbstractTexture::status readonly

    Holds the current status of the texture provider.
 */

/*!
    \enum Qt3DRender::QAbstractTexture::Status

    Contains the status of the texture provider.

    \value None
    \value Loading
    \value Ready
    \value Error
*/
/*!
    Set the status of the texture provider to the specified \a status.
*/
void QAbstractTexture::setStatus(Status status)
{
    Q_D(QAbstractTexture);
    if (status != d->m_status) {
        d->m_status = status;
        emit statusChanged(status);
    }
}

/*!
 * \return the current status of the texture provider.
 */
QAbstractTexture::Status QAbstractTexture::status() const
{
    Q_D(const QAbstractTexture);
    return d->m_status;
}

/*!
    \property Qt3DRender::QAbstractTexture::target readonly

    Holds the target format of the texture provider.

    \note The target format can only be set once.
 */
/*!
    \enum Qt3DRender::QAbstractTexture::Target

    \value TargetAutomatic
           Target will be determined by the Qt3D engine
    \value Target1D
           GL_TEXTURE_1D
    \value Target1DArray
           GL_TEXTURE_1D_ARRAY
    \value Target2D
           GL_TEXTURE_2D
    \value Target2DArray
           GL_TEXTURE_2D_ARRAY
    \value Target3D
           GL_TEXTURE_3D
    \value TargetCubeMap
           GL_TEXTURE_CUBE_MAP
    \value TargetCubeMapArray
           GL_TEXTURE_CUBE_MAP_ARRAY
    \value Target2DMultisample
           GL_TEXTURE_2D_MULTISAMPLE
    \value Target2DMultisampleArray
           GL_TEXTURE_2D_MULTISAMPLE_ARRAY
    \value TargetRectangle
           GL_TEXTURE_RECTANGLE
    \value TargetBuffer
           GL_TEXTURE_BUFFER
*/

/*!
    Returns the target format of the texture provider.
*/
QAbstractTexture::Target QAbstractTexture::target() const
{
    Q_D(const QAbstractTexture);
    return d->m_target;
}

/*!
    Adds a new Qt3DCore::QAbstractTextureImage \a textureImage to the texture provider.

    \note Qt3DRender::QAbstractTextureImage should never be shared between multiple
    Qt3DRender::QAbstractTexture instances.
 */
void QAbstractTexture::addTextureImage(QAbstractTextureImage *textureImage)
{
    Q_ASSERT(textureImage);
    Q_D(QAbstractTexture);
    if (!d->m_textureImages.contains(textureImage)) {
        d->m_textureImages.append(textureImage);

        // Ensures proper bookkeeping
        d->registerDestructionHelper(textureImage, &QAbstractTexture::removeTextureImage, d->m_textureImages);

        // We need to add it as a child of the current node if it has been declared inline
        // Or not previously added as a child of the current node so that
        // 1) The backend gets notified about it's creation
        // 2) When the current node is destroyed, it gets destroyed as well
        if (!textureImage->parent())
            textureImage->setParent(this);

        if (d->m_changeArbiter != nullptr) {
            const auto change = QPropertyNodeAddedChangePtr::create(id(), textureImage);
            change->setPropertyName("textureImage");
            d->notifyObservers(change);
        }
    }
}

/*!
    Removes a Qt3DCore::QAbstractTextureImage \a textureImage from the texture provider.
 */
void QAbstractTexture::removeTextureImage(QAbstractTextureImage *textureImage)
{
    Q_ASSERT(textureImage);
    Q_D(QAbstractTexture);
    if (d->m_changeArbiter != nullptr) {
        const auto change = QPropertyNodeRemovedChangePtr::create(id(), textureImage);
        change->setPropertyName("textureImage");
        d->notifyObservers(change);
    }
    d->m_textureImages.removeOne(textureImage);
    // Remove bookkeeping connection
    d->unregisterDestructionHelper(textureImage);
}

/*!
    \return a list of pointers to QAbstractTextureImage objects contained in
    the texture provider.
 */
QVector<QAbstractTextureImage *> QAbstractTexture::textureImages() const
{
    Q_D(const QAbstractTexture);
    return d->m_textureImages;
}

/*!
    \property Qt3DRender::QAbstractTexture::generateMipMaps

    Holds whether the texture provider should auto generate mipmaps.
 */
/*!
    Boolean parameter \a gen sets a flag indicating whether the
    texture provider should generate mipmaps or not.
*/
void QAbstractTexture::setGenerateMipMaps(bool gen)
{
    Q_D(QAbstractTexture);
    if (d->m_autoMipMap != gen) {
        d->m_autoMipMap = gen;
        emit generateMipMapsChanged(gen);
    }
}

bool QAbstractTexture::generateMipMaps() const
{
    Q_D(const QAbstractTexture);
    return d->m_autoMipMap;
}

/*!
    \property Qt3DRender::QAbstractTexture::minificationFilter

    Holds the minification filter of the texture provider.
 */
/*!
    Set the minification filter to the specified value \a f.
*/
void QAbstractTexture::setMinificationFilter(Filter f)
{
    Q_D(QAbstractTexture);
    if (d->m_minFilter != f) {
        d->m_minFilter = f;
        emit minificationFilterChanged(f);
    }
}
/*!
    \enum Qt3DRender::QAbstractTexture::Filter

    Holds the filter type of the texture provider.

    \value Nearest
           GL_NEAREST
    \value Linear
           GL_LINEAR
    \value NearestMipMapNearest
           GL_NEAREST_MIPMAP_NEAREST
    \value NearestMipMapLinear
           GL_NEAREST_MIPMAP_LINEAR
    \value LinearMipMapNearest
           GL_LINEAR_MIPMAP_NEAREST
    \value LinearMipMapLinear
           GL_LINEAR_MIPMAP_LINEAR
*/
/*!
    \property Qt3DRender::QAbstractTexture::magnificationFilter

    Holds the magnification filter of the texture provider.
 */
/*!
    Set the magnification filter to \a f.
*/
void QAbstractTexture::setMagnificationFilter(Filter f)
{
    Q_D(QAbstractTexture);
    if (d->m_magFilter != f) {
        d->m_magFilter = f;
        emit magnificationFilterChanged(f);
    }
}

QAbstractTexture::Filter QAbstractTexture::minificationFilter() const
{
    Q_D(const QAbstractTexture);
    return d->m_minFilter;
}

QAbstractTexture::Filter QAbstractTexture::magnificationFilter() const
{
    Q_D(const QAbstractTexture);
    return d->m_magFilter;
}

/*!
    \property Qt3DRender::QAbstractTexture::wrapMode

    Holds the wrap mode of the texture provider.
 */
/*!
    Set the wrapmode to the value specified in \a wrapMode.
*/
void QAbstractTexture::setWrapMode(const QTextureWrapMode &wrapMode)
{
    Q_D(QAbstractTexture);
    if (d->m_wrapMode.x() != wrapMode.x()) {
        d->m_wrapMode.setX(wrapMode.x());
        auto e = QPropertyUpdatedChangePtr::create(d->m_id);
        e->setPropertyName("wrapModeX");
        e->setValue(static_cast<int>(d->m_wrapMode.x()));
        d->notifyObservers(e);
    }
    if (d->m_wrapMode.y() != wrapMode.y()) {
        d->m_wrapMode.setY(wrapMode.y());
        auto e = QPropertyUpdatedChangePtr::create(d->m_id);
        e->setPropertyName("wrapModeY");
        e->setValue(static_cast<int>(d->m_wrapMode.y()));
        d->notifyObservers(e);
    }
    if (d->m_wrapMode.z() != wrapMode.z()) {
        d->m_wrapMode.setZ(wrapMode.z());
        auto e = QPropertyUpdatedChangePtr::create(d->m_id);
        e->setPropertyName("wrapModeZ");
        e->setValue(static_cast<int>(d->m_wrapMode.z()));
        d->notifyObservers(e);
    }
}

QTextureWrapMode *QAbstractTexture::wrapMode()
{
    Q_D(QAbstractTexture);
    return &d->m_wrapMode;
}

/*!
    \property Qt3DRender::QAbstractTexture::maximumAnisotropy

    Holds the maximum anisotropy of the texture provider.
 */
/*!
    Sets the maximum anisotropy to \a anisotropy.
*/
void QAbstractTexture::setMaximumAnisotropy(float anisotropy)
{
    Q_D(QAbstractTexture);
    if (!qFuzzyCompare(d->m_maximumAnisotropy, anisotropy)) {
        d->m_maximumAnisotropy = anisotropy;
        emit maximumAnisotropyChanged(anisotropy);
    }
}

/*!
 * \return the current maximum anisotropy
 */
float QAbstractTexture::maximumAnisotropy() const
{
    Q_D(const QAbstractTexture);
    return d->m_maximumAnisotropy;
}

/*!
    \property Qt3DRender::QAbstractTexture::comparisonFunction

    Holds the comparison function of the texture provider.
 */
/*!
    Set the comparison function to \a function.
*/
void QAbstractTexture::setComparisonFunction(QAbstractTexture::ComparisonFunction function)
{
    Q_D(QAbstractTexture);
    if (d->m_comparisonFunction != function) {
        d->m_comparisonFunction = function;
        emit comparisonFunctionChanged(function);
    }
}

/*!
 * \return the current comparison function.
 */
QAbstractTexture::ComparisonFunction QAbstractTexture::comparisonFunction() const
{
    Q_D(const QAbstractTexture);
    return d->m_comparisonFunction;
}

/*!
    \property Qt3DRender::QAbstractTexture::comparisonMode

    Holds the comparison mode of the texture provider.
 */
/*!
    Set the comparison mode to \a mode.
*/
void QAbstractTexture::setComparisonMode(QAbstractTexture::ComparisonMode mode)
{
    Q_D(QAbstractTexture);
    if (d->m_comparisonMode != mode) {
        d->m_comparisonMode = mode;
        emit comparisonModeChanged(mode);
    }
}

/*!
 * \return the current comparison mode.
 */
QAbstractTexture::ComparisonMode QAbstractTexture::comparisonMode() const
{
    Q_D(const QAbstractTexture);
    return d->m_comparisonMode;
}

/*!
 * \return the current data generator.
 */
QTextureGeneratorPtr QAbstractTexture::dataGenerator() const
{
    Q_D(const QAbstractTexture);
    return d->m_dataFunctor;
}

Qt3DCore::QNodeCreatedChangeBasePtr QAbstractTexture::createNodeCreationChange() const
{
    auto creationChange = Qt3DCore::QNodeCreatedChangePtr<QAbstractTextureData>::create(this);
    auto &data = creationChange->data;
    Q_D(const QAbstractTexture);
    data.target = d->m_target;
    data.format = d->m_format;
    data.width = d->m_width;
    data.height = d->m_height;
    data.depth = d->m_depth;
    data.autoMipMap = d->m_autoMipMap;
    data.minFilter = d->m_minFilter;
    data.magFilter = d->m_magFilter;
    data.wrapModeX = d->m_wrapMode.x();
    data.wrapModeY = d->m_wrapMode.y();
    data.wrapModeZ = d->m_wrapMode.z();
    data.maximumAnisotropy = d->m_maximumAnisotropy;
    data.comparisonFunction = d->m_comparisonFunction;
    data.comparisonMode = d->m_comparisonMode;
    data.textureImageIds = qIdsForNodes(d->m_textureImages);
    data.layers = d->m_layers;
    data.samples = d->m_samples;
    data.dataFunctor = d->m_dataFunctor;
    return creationChange;
}

void QAbstractTexture::sceneChangeEvent(const Qt3DCore::QSceneChangePtr &change)
{
    switch (change->type()) {
    case PropertyUpdated: {
        Qt3DCore::QPropertyUpdatedChangePtr propertyChange = qSharedPointerCast<Qt3DCore::QPropertyUpdatedChange>(change);
        if (propertyChange->propertyName() == QByteArrayLiteral("width")) {
            bool blocked = blockNotifications(true);
            setWidth(propertyChange->value().toInt());
            blockNotifications(blocked);
        } else if (propertyChange->propertyName() == QByteArrayLiteral("height")) {
            bool blocked = blockNotifications(true);
            setHeight(propertyChange->value().toInt());
            blockNotifications(blocked);
        } else if (propertyChange->propertyName() == QByteArrayLiteral("depth")) {
            bool blocked = blockNotifications(true);
            setDepth(propertyChange->value().toInt());
            blockNotifications(blocked);
        } else if (propertyChange->propertyName() == QByteArrayLiteral("layers")) {
            bool blocked = blockNotifications(true);
            setLayers(propertyChange->value().toInt());
            blockNotifications(blocked);
        } else if (propertyChange->propertyName() == QByteArrayLiteral("format")) {
            bool blocked = blockNotifications(true);
            setFormat(static_cast<QAbstractTexture::TextureFormat>(propertyChange->value().toInt()));
            blockNotifications(blocked);
        } else if (propertyChange->propertyName() == QByteArrayLiteral("status")) {
            bool blocked = blockNotifications(true);
            setStatus(static_cast<QAbstractTexture::Status>(propertyChange->value().toInt()));
            blockNotifications(blocked);
        }
        // TODO handle target changes, it's a CONSTANT property but can be affected by loader
        break;
    }
    default:
        break;
    };
}


} // namespace Qt3DRender

QT_END_NAMESPACE
