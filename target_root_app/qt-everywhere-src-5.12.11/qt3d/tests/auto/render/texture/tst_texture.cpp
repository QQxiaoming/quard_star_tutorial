/****************************************************************************
**
** Copyright (C) 2016 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QTest>
#include <qbackendnodetester.h>
#include <Qt3DCore/qdynamicpropertyupdatedchange.h>
#include <Qt3DCore/qpropertynodeaddedchange.h>
#include <Qt3DCore/qpropertynoderemovedchange.h>
#include <Qt3DRender/private/texture_p.h>

#include "testpostmanarbiter.h"
#include "testrenderer.h"

class DummyTexture : public Qt3DRender::QAbstractTexture
{
    Q_OBJECT

public:
    explicit DummyTexture(Qt3DCore::QNode *parent = nullptr)
        : QAbstractTexture(TargetAutomatic, parent)
    {
    }
};

class FakeGenerator : public Qt3DRender::QTextureGenerator
{
public:
    QT3D_FUNCTOR(FakeGenerator)

    Qt3DRender::QTextureDataPtr operator ()() override
    {
        return {};
    }

    bool operator ==(const QTextureGenerator &) const override
    {
        return true;
    }
};

class tst_RenderTexture : public Qt3DCore::QBackendNodeTester
{
    Q_OBJECT

private:
    template <typename FrontendTextureType, Qt3DRender::QAbstractTexture::Target Target>
    void checkPropertyMirroring();

private slots:
    void checkDefaults();
    void checkFrontendPropertyNotifications();
    void checkPropertyMirroring();
    void checkPropertyChanges();
    void checkTextureImageBookeeping();
};

void tst_RenderTexture::checkDefaults()
{
    Qt3DRender::Render::Texture backend;

    QCOMPARE(backend.properties().format, Qt3DRender::QAbstractTexture::NoFormat);
    QCOMPARE(backend.properties().width, 1);
    QCOMPARE(backend.properties().height, 1);
    QCOMPARE(backend.properties().depth, 1);
    QCOMPARE(backend.properties().layers, 1);
    QCOMPARE(backend.properties().mipLevels, 1);
    QCOMPARE(backend.properties().samples, 1);
    QCOMPARE(backend.properties().generateMipMaps, false);
    QCOMPARE(backend.parameters().magnificationFilter, Qt3DRender::QAbstractTexture::Nearest);
    QCOMPARE(backend.parameters().minificationFilter, Qt3DRender::QAbstractTexture::Nearest);
    QCOMPARE(backend.parameters().wrapModeX, Qt3DRender::QTextureWrapMode::ClampToEdge);
    QCOMPARE(backend.parameters().wrapModeY, Qt3DRender::QTextureWrapMode::ClampToEdge);
    QCOMPARE(backend.parameters().wrapModeZ, Qt3DRender::QTextureWrapMode::ClampToEdge);
    QCOMPARE(backend.parameters().maximumAnisotropy, 1.0f);
    QCOMPARE(backend.parameters().comparisonFunction, Qt3DRender::QAbstractTexture::CompareLessEqual);
    QCOMPARE(backend.parameters().comparisonMode, Qt3DRender::QAbstractTexture::CompareNone);
}

void tst_RenderTexture::checkFrontendPropertyNotifications()
{
    // GIVEN
    TestArbiter arbiter;
    DummyTexture texture;
    arbiter.setArbiterOnNode(&texture);

    // WHEN
    texture.setWidth(512);
    QCoreApplication::processEvents();

    // THEN
    QCOMPARE(arbiter.events.size(), 1);
    Qt3DCore::QPropertyUpdatedChangePtr change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
    QCOMPARE(change->propertyName(), "width");
    QCOMPARE(change->value().value<int>(), 512);
    QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

    arbiter.events.clear();

    // WHEN
    texture.setWidth(512);
    QCoreApplication::processEvents();

    // THEN
    QCOMPARE(arbiter.events.size(), 0);

    // WHEN
    texture.setHeight(256);
    QCoreApplication::processEvents();

    // THEN
    QCOMPARE(arbiter.events.size(), 1);
    change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
    QCOMPARE(change->propertyName(), "height");
    QCOMPARE(change->value().value<int>(), 256);
    QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

    arbiter.events.clear();

    // WHEN
    texture.setHeight(256);
    QCoreApplication::processEvents();

    // THEN
    QCOMPARE(arbiter.events.size(), 0);

    // WHEN
    texture.setDepth(128);
    QCoreApplication::processEvents();

    // THEN
    QCOMPARE(arbiter.events.size(), 1);
    change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
    QCOMPARE(change->propertyName(), "depth");
    QCOMPARE(change->value().value<int>(), 128);
    QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

    arbiter.events.clear();

    // WHEN
    texture.setDepth(128);
    QCoreApplication::processEvents();

    // THEN
    QCOMPARE(arbiter.events.size(), 0);

    // WHEN
    texture.setLayers(16);
    QCoreApplication::processEvents();

    // THEN
    QCOMPARE(arbiter.events.size(), 1);
    change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
    QCOMPARE(change->propertyName(), "layers");
    QCOMPARE(change->value().value<int>(), 16);
    QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

    arbiter.events.clear();

    // WHEN
    texture.setLayers(16);
    QCoreApplication::processEvents();

    // THEN
    QCOMPARE(arbiter.events.size(), 0);

    // WHEN
    texture.setSamples(32);
    QCoreApplication::processEvents();

    // THEN
    QCOMPARE(arbiter.events.size(), 1);
    change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
    QCOMPARE(change->propertyName(), "samples");
    QCOMPARE(change->value().value<int>(), 32);
    QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

    arbiter.events.clear();

    // WHEN
    texture.setSamples(32);
    QCoreApplication::processEvents();

    // THEN
    QCOMPARE(arbiter.events.size(), 0);

    // WHEN
    Qt3DRender::QTextureImage img;
    texture.addTextureImage(&img);

    // THEN
    QCOMPARE(arbiter.events.size(), 1);
    const auto addedChange = arbiter.events.first().staticCast<Qt3DCore::QPropertyNodeAddedChange>();
    QCOMPARE(addedChange->propertyName(), "textureImage");
    QCOMPARE(addedChange->addedNodeId(), img.id());
    QCOMPARE(addedChange->type(), Qt3DCore::PropertyValueAdded);

    arbiter.events.clear();

    // WHEN
    texture.removeTextureImage(&img);

    // THEN
    QCOMPARE(arbiter.events.size(), 1);
    const auto removedChange = arbiter.events.first().staticCast<Qt3DCore::QPropertyNodeRemovedChange>();
    QCOMPARE(removedChange->propertyName(), "textureImage");
    QCOMPARE(removedChange->removedNodeId(), img.id());
    QCOMPARE(removedChange->type(), Qt3DCore::PropertyValueRemoved);

    arbiter.events.clear();
}

template <typename FrontendTextureType, Qt3DRender::QAbstractTexture::Target Target>
void tst_RenderTexture::checkPropertyMirroring()
{
    // GIVEN
    Qt3DRender::Render::Texture backend;

    FrontendTextureType frontend;
    frontend.setWidth(256);
    frontend.setHeight(128);
    frontend.setDepth(16);
    frontend.setLayers(8);
    frontend.setSamples(32);

    // WHEN
    simulateInitialization(&frontend, &backend);

    // THEN
    QCOMPARE(backend.peerId(), frontend.id());
    QCOMPARE(backend.properties().target, Target);
    QCOMPARE(backend.properties().width, frontend.width());
    QCOMPARE(backend.properties().height, frontend.height());
    QCOMPARE(backend.properties().depth, frontend.depth());
    QCOMPARE(backend.properties().layers, frontend.layers());
    QCOMPARE(backend.properties().samples, frontend.samples());
}

void tst_RenderTexture::checkPropertyMirroring()
{
    checkPropertyMirroring<Qt3DRender::QTexture1D, Qt3DRender::QAbstractTexture::Target1D>();
    checkPropertyMirroring<Qt3DRender::QTexture1DArray, Qt3DRender::QAbstractTexture::Target1DArray>();
    checkPropertyMirroring<Qt3DRender::QTexture2D, Qt3DRender::QAbstractTexture::Target2D>();
    checkPropertyMirroring<Qt3DRender::QTexture2DArray, Qt3DRender::QAbstractTexture::Target2DArray>();
    checkPropertyMirroring<Qt3DRender::QTexture3D, Qt3DRender::QAbstractTexture::Target3D>();
    checkPropertyMirroring<Qt3DRender::QTextureCubeMap, Qt3DRender::QAbstractTexture::TargetCubeMap>();
    checkPropertyMirroring<Qt3DRender::QTextureCubeMapArray, Qt3DRender::QAbstractTexture::TargetCubeMapArray>();
    checkPropertyMirroring<Qt3DRender::QTexture2DMultisample, Qt3DRender::QAbstractTexture::Target2DMultisample>();
    checkPropertyMirroring<Qt3DRender::QTexture2DMultisampleArray, Qt3DRender::QAbstractTexture::Target2DMultisampleArray>();
    checkPropertyMirroring<Qt3DRender::QTextureRectangle, Qt3DRender::QAbstractTexture::TargetRectangle>();
    checkPropertyMirroring<Qt3DRender::QTextureBuffer, Qt3DRender::QAbstractTexture::TargetBuffer>();
}

void tst_RenderTexture::checkPropertyChanges()
{
    // GIVEN
    TestRenderer renderer;
    Qt3DRender::Render::Texture backend;
    backend.setRenderer(&renderer);
    backend.unsetDirty();

    // WHEN
    Qt3DCore::QPropertyUpdatedChangePtr updateChange(new Qt3DCore::QPropertyUpdatedChange(Qt3DCore::QNodeId()));
    updateChange->setValue(256);
    updateChange->setPropertyName("width");
    backend.sceneChangeEvent(updateChange);

    // THEN
    QCOMPARE(backend.properties().width, 256);
    QVERIFY(renderer.dirtyBits() & Qt3DRender::Render::AbstractRenderer::TexturesDirty);
    QVERIFY(backend.dirtyFlags() == Qt3DRender::Render::Texture::DirtyProperties);
    renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
    backend.unsetDirty();

    // WHEN
    updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
    updateChange->setValue(128);
    updateChange->setPropertyName("height");
    backend.sceneChangeEvent(updateChange);

    // THEN
    QCOMPARE(backend.properties().height, 128);
    QVERIFY(renderer.dirtyBits() & Qt3DRender::Render::AbstractRenderer::TexturesDirty);
    QVERIFY(backend.dirtyFlags() == Qt3DRender::Render::Texture::DirtyProperties);
    renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
    backend.unsetDirty();

    // WHEN
    updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
    updateChange->setValue(16);
    updateChange->setPropertyName("depth");
    backend.sceneChangeEvent(updateChange);

    // THEN
    QCOMPARE(backend.properties().depth, 16);
    QVERIFY(renderer.dirtyBits() & Qt3DRender::Render::AbstractRenderer::TexturesDirty);
    QVERIFY(backend.dirtyFlags() == Qt3DRender::Render::Texture::DirtyProperties);
    renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
    backend.unsetDirty();

    // WHEN
    updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
    updateChange->setValue(QVariant::fromValue(Qt3DRender::QAbstractTexture::RGB16F));
    updateChange->setPropertyName("format");
    backend.sceneChangeEvent(updateChange);

    // THEN
    QCOMPARE(backend.properties().format, Qt3DRender::QAbstractTexture::RGB16F);
    QVERIFY(renderer.dirtyBits() & Qt3DRender::Render::AbstractRenderer::TexturesDirty);
    QVERIFY(backend.dirtyFlags() == Qt3DRender::Render::Texture::DirtyProperties);
    renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
    backend.unsetDirty();

    // WHEN
    updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
    updateChange->setValue(QVariant::fromValue(Qt3DRender::QAbstractTexture::Target1DArray));
    updateChange->setPropertyName("target");
    backend.sceneChangeEvent(updateChange);

    // THEN
    QCOMPARE(backend.properties().target, Qt3DRender::QAbstractTexture::Target1DArray);
    QVERIFY(renderer.dirtyBits() & Qt3DRender::Render::AbstractRenderer::TexturesDirty);
    QVERIFY(backend.dirtyFlags() == Qt3DRender::Render::Texture::DirtyProperties);
    renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
    backend.unsetDirty();

    // WHEN
    updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
    updateChange->setValue(true);
    updateChange->setPropertyName("mipmaps");
    backend.sceneChangeEvent(updateChange);

    // THEN
    QCOMPARE(backend.properties().generateMipMaps, true);
    QVERIFY(renderer.dirtyBits() & Qt3DRender::Render::AbstractRenderer::TexturesDirty);
    QVERIFY(backend.dirtyFlags() == Qt3DRender::Render::Texture::DirtyProperties);
    renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
    backend.unsetDirty();

    // WHEN
    updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
    updateChange->setValue(QVariant::fromValue(Qt3DRender::QAbstractTexture::LinearMipMapLinear));
    updateChange->setPropertyName("minificationFilter");
    backend.sceneChangeEvent(updateChange);

    // THEN
    QCOMPARE(backend.parameters().minificationFilter, Qt3DRender::QAbstractTexture::LinearMipMapLinear);
    QVERIFY(renderer.dirtyBits() & Qt3DRender::Render::AbstractRenderer::TexturesDirty);
    QVERIFY(backend.dirtyFlags() == Qt3DRender::Render::Texture::DirtyParameters);
    renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
    backend.unsetDirty();

    // WHEN
    updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
    updateChange->setValue(QVariant::fromValue(Qt3DRender::QAbstractTexture::Linear));
    updateChange->setPropertyName("magnificationFilter");
    backend.sceneChangeEvent(updateChange);

    // THEN
    QCOMPARE(backend.parameters().magnificationFilter, Qt3DRender::QAbstractTexture::Linear);
    QVERIFY(renderer.dirtyBits() & Qt3DRender::Render::AbstractRenderer::TexturesDirty);
    QVERIFY(backend.dirtyFlags() == Qt3DRender::Render::Texture::DirtyParameters);
    renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
    backend.unsetDirty();

    // WHEN
    updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
    updateChange->setValue(QVariant::fromValue(Qt3DRender::QTextureWrapMode::Repeat));
    updateChange->setPropertyName("wrapModeX");
    backend.sceneChangeEvent(updateChange);

    // THEN
    QCOMPARE(backend.parameters().wrapModeX, Qt3DRender::QTextureWrapMode::Repeat);
    QVERIFY(renderer.dirtyBits() & Qt3DRender::Render::AbstractRenderer::TexturesDirty);
    QVERIFY(backend.dirtyFlags() == Qt3DRender::Render::Texture::DirtyParameters);
    renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
    backend.unsetDirty();

    // WHEN
    updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
    updateChange->setValue(QVariant::fromValue(Qt3DRender::QTextureWrapMode::Repeat));
    updateChange->setPropertyName("wrapModeY");
    backend.sceneChangeEvent(updateChange);

    // THEN
    QCOMPARE(backend.parameters().wrapModeY, Qt3DRender::QTextureWrapMode::Repeat);
    QVERIFY(renderer.dirtyBits() & Qt3DRender::Render::AbstractRenderer::TexturesDirty);
    QVERIFY(backend.dirtyFlags() == Qt3DRender::Render::Texture::DirtyParameters);
    renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
    backend.unsetDirty();

    // WHEN
    updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
    updateChange->setValue(QVariant::fromValue(Qt3DRender::QTextureWrapMode::Repeat));
    updateChange->setPropertyName("wrapModeZ");
    backend.sceneChangeEvent(updateChange);

    // THEN
    QCOMPARE(backend.parameters().wrapModeZ, Qt3DRender::QTextureWrapMode::Repeat);
    QVERIFY(renderer.dirtyBits() & Qt3DRender::Render::AbstractRenderer::TexturesDirty);
    QVERIFY(backend.dirtyFlags() == Qt3DRender::Render::Texture::DirtyParameters);
    renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
    backend.unsetDirty();

    // WHEN
    updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
    updateChange->setValue(16.0f);
    updateChange->setPropertyName("maximumAnisotropy");
    backend.sceneChangeEvent(updateChange);

    // THEN
    QCOMPARE(backend.parameters().maximumAnisotropy, 16.0f);
    QVERIFY(renderer.dirtyBits() & Qt3DRender::Render::AbstractRenderer::TexturesDirty);
    QVERIFY(backend.dirtyFlags() == Qt3DRender::Render::Texture::DirtyParameters);
    renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
    backend.unsetDirty();

    // WHEN
    updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
    updateChange->setValue(QVariant::fromValue(Qt3DRender::QAbstractTexture::CompareEqual));
    updateChange->setPropertyName("comparisonFunction");
    backend.sceneChangeEvent(updateChange);

    // THEN
    QCOMPARE(backend.parameters().comparisonFunction, Qt3DRender::QAbstractTexture::CompareEqual);
    QVERIFY(renderer.dirtyBits() & Qt3DRender::Render::AbstractRenderer::TexturesDirty);
    QVERIFY(backend.dirtyFlags() == Qt3DRender::Render::Texture::DirtyParameters);
    renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
    backend.unsetDirty();

    // WHEN
    updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
    updateChange->setValue(QVariant::fromValue(Qt3DRender::QAbstractTexture::CompareRefToTexture));
    updateChange->setPropertyName("comparisonMode");
    backend.sceneChangeEvent(updateChange);

    // THEN
    QCOMPARE(backend.parameters().comparisonMode, Qt3DRender::QAbstractTexture::CompareRefToTexture);
    QVERIFY(renderer.dirtyBits() & Qt3DRender::Render::AbstractRenderer::TexturesDirty);
    QVERIFY(backend.dirtyFlags() == Qt3DRender::Render::Texture::DirtyParameters);
    renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
    backend.unsetDirty();

    // WHEN
    updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
    updateChange->setValue(32);
    updateChange->setPropertyName("layers");
    backend.sceneChangeEvent(updateChange);

    // THEN
    QCOMPARE(backend.properties().layers, 32);
    QVERIFY(renderer.dirtyBits() & Qt3DRender::Render::AbstractRenderer::TexturesDirty);
    QVERIFY(backend.dirtyFlags() == Qt3DRender::Render::Texture::DirtyProperties);
    renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
    backend.unsetDirty();

    // WHEN
    updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
    updateChange->setValue(64);
    updateChange->setPropertyName("samples");
    backend.sceneChangeEvent(updateChange);

    // THEN
    QCOMPARE(backend.properties().samples, 64);
    QVERIFY(renderer.dirtyBits() & Qt3DRender::Render::AbstractRenderer::TexturesDirty);
    QVERIFY(backend.dirtyFlags() == Qt3DRender::Render::Texture::DirtyProperties);
    renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
    backend.unsetDirty();

    // WHEN
    updateChange = QSharedPointer<Qt3DCore::QPropertyUpdatedChange>::create(Qt3DCore::QNodeId());
    Qt3DRender::QTextureGeneratorPtr gen = QSharedPointer<FakeGenerator>::create();
    updateChange->setValue(QVariant::fromValue(gen));
    updateChange->setPropertyName("generator");
    backend.sceneChangeEvent(updateChange);

    // THEN
    QCOMPARE(backend.dataGenerator(), gen);
    QVERIFY(renderer.dirtyBits() & Qt3DRender::Render::AbstractRenderer::TexturesDirty);
    QVERIFY(backend.dirtyFlags() == Qt3DRender::Render::Texture::DirtyDataGenerator);
    renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
    backend.unsetDirty();

    // WHEN
    Qt3DRender::QTextureImage img;
    const auto imageAddChange = Qt3DCore::QPropertyNodeAddedChangePtr::create(Qt3DCore::QNodeId(), &img);
    imageAddChange->setPropertyName("textureImage");
    backend.sceneChangeEvent(imageAddChange);

    // THEN
    QCOMPARE(backend.textureImageIds().size(), 1);
    QCOMPARE(backend.textureImageIds().first(), img.id());
    QVERIFY(renderer.dirtyBits() & Qt3DRender::Render::AbstractRenderer::TexturesDirty);
    QVERIFY(backend.dirtyFlags() == Qt3DRender::Render::Texture::DirtyImageGenerators);
    renderer.clearDirtyBits(Qt3DRender::Render::AbstractRenderer::AllDirty);
    backend.unsetDirty();
}

void tst_RenderTexture::checkTextureImageBookeeping()
{
    // GIVEN
    TestArbiter arbiter;
    DummyTexture texture;
    arbiter.setArbiterOnNode(&texture);

    QCoreApplication::processEvents();

    {
        // WHEN
        Qt3DRender::QTextureImage img;
        texture.addTextureImage(&img);

        // THEN
        QCOMPARE(img.parent(), &texture);
        QCOMPARE(texture.textureImages().size(), 1);
        QCOMPARE(texture.textureImages().first()->id(), img.id());
    }
    // WHEN -> img is destroyed

    // THEN
    QCOMPARE(texture.textureImages().size(), 0);

}

QTEST_APPLESS_MAIN(tst_RenderTexture)

#include "tst_texture.moc"
