/****************************************************************************
**
** Copyright (C) 2014 Klaralvdalens Datakonsult AB (KDAB).
** Copyright (C) 2016 The Qt Company Ltd and/or its subsidiary(-ies).
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

#include "renderview_p.h"
#include <Qt3DRender/qmaterial.h>
#include <Qt3DRender/qrenderaspect.h>
#include <Qt3DRender/qrendertarget.h>
#include <Qt3DRender/qabstractlight.h>
#include <Qt3DRender/private/sphere_p.h>

#include <Qt3DRender/private/cameraselectornode_p.h>
#include <Qt3DRender/private/framegraphnode_p.h>
#include <Qt3DRender/private/layerfilternode_p.h>
#include <Qt3DRender/private/qparameter_p.h>
#include <Qt3DRender/private/cameralens_p.h>
#include <Qt3DRender/private/rendercommand_p.h>
#include <Qt3DRender/private/effect_p.h>
#include <Qt3DRender/private/entity_p.h>
#include <Qt3DRender/private/renderer_p.h>
#include <Qt3DRender/private/nodemanagers_p.h>
#include <Qt3DRender/private/layer_p.h>
#include <Qt3DRender/private/renderlogging_p.h>
#include <Qt3DRender/private/renderpassfilternode_p.h>
#include <Qt3DRender/private/renderpass_p.h>
#include <Qt3DRender/private/geometryrenderer_p.h>
#include <Qt3DRender/private/renderstateset_p.h>
#include <Qt3DRender/private/techniquefilternode_p.h>
#include <Qt3DRender/private/viewportnode_p.h>
#include <Qt3DRender/private/buffermanager_p.h>
#include <Qt3DRender/private/geometryrenderermanager_p.h>
#include <Qt3DRender/private/rendercapture_p.h>
#include <Qt3DRender/private/buffercapture_p.h>
#include <Qt3DRender/private/stringtoint_p.h>
#include <Qt3DCore/qentity.h>
#include <QtGui/qsurface.h>
#include <algorithm>

#include <QDebug>
#if defined(QT3D_RENDER_VIEW_JOB_TIMINGS)
#include <QElapsedTimer>
#endif

QT_BEGIN_NAMESPACE

namespace Qt3DRender {
namespace Render {


namespace  {

// register our QNodeId's as a metatype during program loading
const int Q_DECL_UNUSED qNodeIdTypeId = qMetaTypeId<Qt3DCore::QNodeId>();

const int MAX_LIGHTS = 8;

#define LIGHT_POSITION_NAME  QLatin1String(".position")
#define LIGHT_TYPE_NAME      QLatin1String(".type")
#define LIGHT_COLOR_NAME     QLatin1String(".color")
#define LIGHT_INTENSITY_NAME QLatin1String(".intensity")

int LIGHT_COUNT_NAME_ID = 0;
int LIGHT_POSITION_NAMES[MAX_LIGHTS];
int LIGHT_TYPE_NAMES[MAX_LIGHTS];
int LIGHT_COLOR_NAMES[MAX_LIGHTS];
int LIGHT_INTENSITY_NAMES[MAX_LIGHTS];
QString LIGHT_STRUCT_NAMES[MAX_LIGHTS];

bool wasInitialized = false;

} // anonymous namespace

RenderView::StandardUniformsNameToTypeHash RenderView::ms_standardUniformSetters;


RenderView::StandardUniformsNameToTypeHash RenderView::initializeStandardUniformSetters()
{
    RenderView::StandardUniformsNameToTypeHash setters;

    setters.insert(StringToInt::lookupId(QLatin1String("modelMatrix")), ModelMatrix);
    setters.insert(StringToInt::lookupId(QLatin1String("viewMatrix")), ViewMatrix);
    setters.insert(StringToInt::lookupId(QLatin1String("projectionMatrix")), ProjectionMatrix);
    setters.insert(StringToInt::lookupId(QLatin1String("modelView")), ModelViewMatrix);
    setters.insert(StringToInt::lookupId(QLatin1String("viewProjectionMatrix")), ViewProjectionMatrix);
    setters.insert(StringToInt::lookupId(QLatin1String("modelViewProjection")), ModelViewProjectionMatrix);
    setters.insert(StringToInt::lookupId(QLatin1String("mvp")), ModelViewProjectionMatrix);
    setters.insert(StringToInt::lookupId(QLatin1String("inverseModelMatrix")), InverseModelMatrix);
    setters.insert(StringToInt::lookupId(QLatin1String("inverseViewMatrix")), InverseViewMatrix);
    setters.insert(StringToInt::lookupId(QLatin1String("inverseProjectionMatrix")), InverseProjectionMatrix);
    setters.insert(StringToInt::lookupId(QLatin1String("inverseModelView")), InverseModelViewMatrix);
    setters.insert(StringToInt::lookupId(QLatin1String("inverseViewProjectionMatrix")), InverseViewProjectionMatrix);
    setters.insert(StringToInt::lookupId(QLatin1String("inverseModelViewProjection")), InverseModelViewProjectionMatrix);
    setters.insert(StringToInt::lookupId(QLatin1String("modelNormalMatrix")), ModelNormalMatrix);
    setters.insert(StringToInt::lookupId(QLatin1String("modelViewNormal")), ModelViewNormalMatrix);
    setters.insert(StringToInt::lookupId(QLatin1String("viewportMatrix")), ViewportMatrix);
    setters.insert(StringToInt::lookupId(QLatin1String("inverseViewportMatrix")), InverseViewportMatrix);
    setters.insert(StringToInt::lookupId(QLatin1String("aspectRatio")), AspectRatio);
    setters.insert(StringToInt::lookupId(QLatin1String("exposure")), Exposure);
    setters.insert(StringToInt::lookupId(QLatin1String("gamma")), Gamma);
    setters.insert(StringToInt::lookupId(QLatin1String("time")), Time);
    setters.insert(StringToInt::lookupId(QLatin1String("eyePosition")), EyePosition);
    setters.insert(StringToInt::lookupId(QLatin1String("skinningPalette[0]")), SkinningPalette);

    return setters;
}

// TODO: Move this somewhere global where GraphicsContext::setViewport() can use it too
static QRectF resolveViewport(const QRectF &fractionalViewport, const QSize &surfaceSize)
{
    return QRectF(fractionalViewport.x() * surfaceSize.width(),
                  (1.0 - fractionalViewport.y() - fractionalViewport.height()) * surfaceSize.height(),
                  fractionalViewport.width() * surfaceSize.width(),
                  fractionalViewport.height() * surfaceSize.height());
}

static Matrix4x4 getProjectionMatrix(const CameraLens *lens)
{
    if (!lens)
        qWarning() << "[Qt3D Renderer] No Camera Lens found. Add a CameraSelector to your Frame Graph or make sure that no entities will be rendered.";
    return lens ? lens->projection() : Matrix4x4();
}

UniformValue RenderView::standardUniformValue(RenderView::StandardUniform standardUniformType,
                                              Entity *entity,
                                              const Matrix4x4 &model) const
{
    switch (standardUniformType) {
    case ModelMatrix:
        return UniformValue(model);
    case ViewMatrix:
        return UniformValue(m_data.m_viewMatrix);
    case ProjectionMatrix:
        return UniformValue(getProjectionMatrix(m_data.m_renderCameraLens));
    case ModelViewMatrix:
        return UniformValue(m_data.m_viewMatrix * model);
    case ViewProjectionMatrix:
        return UniformValue(getProjectionMatrix(m_data.m_renderCameraLens) * m_data.m_viewMatrix);
    case ModelViewProjectionMatrix:
        return UniformValue(m_data.m_viewProjectionMatrix * model);
    case InverseModelMatrix:
        return UniformValue(model.inverted());
    case InverseViewMatrix:
        return UniformValue(m_data.m_viewMatrix.inverted());
    case InverseProjectionMatrix: {
        return UniformValue(getProjectionMatrix(m_data.m_renderCameraLens).inverted());
    }
    case InverseModelViewMatrix:
        return UniformValue((m_data.m_viewMatrix * model).inverted());
    case InverseViewProjectionMatrix: {
        const Matrix4x4 viewProjectionMatrix = getProjectionMatrix(m_data.m_renderCameraLens) * m_data.m_viewMatrix;
        return UniformValue(viewProjectionMatrix.inverted());
    }
    case InverseModelViewProjectionMatrix:
        return UniformValue((m_data.m_viewProjectionMatrix * model).inverted());
    case ModelNormalMatrix:
        return UniformValue(convertToQMatrix4x4(model).normalMatrix());
    case ModelViewNormalMatrix:
        return UniformValue(convertToQMatrix4x4(m_data.m_viewMatrix * model).normalMatrix());
    case ViewportMatrix: {
        QMatrix4x4 viewportMatrix;
        // TO DO: Implement on Matrix4x4
        viewportMatrix.viewport(resolveViewport(m_viewport, m_surfaceSize));
        return UniformValue(Matrix4x4(viewportMatrix));
    }
    case InverseViewportMatrix: {
        QMatrix4x4 viewportMatrix;
        // TO DO: Implement on Matrix4x4
        viewportMatrix.viewport(resolveViewport(m_viewport, m_surfaceSize));
        return UniformValue(Matrix4x4(viewportMatrix.inverted()));
    }
    case AspectRatio:
        return float(m_surfaceSize.width()) / float(m_surfaceSize.height());
    case Exposure:
        return UniformValue(m_data.m_renderCameraLens ? m_data.m_renderCameraLens->exposure() : 0.0f);
    case Gamma:
        return UniformValue(m_gamma);
    case Time:
        return UniformValue(float(m_renderer->time() / 1000000000.0f));
    case EyePosition:
        return UniformValue(m_data.m_eyePos);
    case SkinningPalette: {
        const Armature *armature = entity->renderComponent<Armature>();
        if (!armature) {
            qCWarning(Jobs, "Requesting skinningPalette uniform but no armature set on entity");
            return UniformValue();
        }
        return armature->skinningPaletteUniform();
    }
    default:
        Q_UNREACHABLE();
        return UniformValue();
    }
}

RenderView::RenderView()
    : m_isDownloadBuffersEnable(false)
    , m_hasBlitFramebufferInfo(false)
    , m_renderer(nullptr)
    , m_devicePixelRatio(1.)
    , m_viewport(QRectF(0.0f, 0.0f, 1.0f, 1.0f))
    , m_gamma(2.2f)
    , m_surface(nullptr)
    , m_clearBuffer(QClearBuffers::None)
    , m_stateSet(nullptr)
    , m_noDraw(false)
    , m_compute(false)
    , m_frustumCulling(false)
    , m_memoryBarrier(QMemoryBarrier::None)
    , m_environmentLight(nullptr)
{
    m_workGroups[0] = 1;
    m_workGroups[1] = 1;
    m_workGroups[2] = 1;

    if (Q_UNLIKELY(!wasInitialized)) {
        // Needed as we can control the init order of static/global variables across compile units
        // and this hash relies on the static StringToInt class
        wasInitialized = true;
        RenderView::ms_standardUniformSetters = RenderView::initializeStandardUniformSetters();
        LIGHT_COUNT_NAME_ID = StringToInt::lookupId(QLatin1String("lightCount"));
        for (int i = 0; i < MAX_LIGHTS; ++i) {
            Q_STATIC_ASSERT_X(MAX_LIGHTS < 10, "can't use the QChar trick anymore");
            LIGHT_STRUCT_NAMES[i] = QLatin1String("lights[") + QLatin1Char(char('0' + i)) + QLatin1Char(']');
            LIGHT_POSITION_NAMES[i] = StringToInt::lookupId(LIGHT_STRUCT_NAMES[i] + LIGHT_POSITION_NAME);
            LIGHT_TYPE_NAMES[i] = StringToInt::lookupId(LIGHT_STRUCT_NAMES[i] + LIGHT_TYPE_NAME);
            LIGHT_COLOR_NAMES[i] = StringToInt::lookupId(LIGHT_STRUCT_NAMES[i] + LIGHT_COLOR_NAME);
            LIGHT_INTENSITY_NAMES[i] = StringToInt::lookupId(LIGHT_STRUCT_NAMES[i] + LIGHT_INTENSITY_NAME);
        }
    }
}

RenderView::~RenderView()
{
    delete m_stateSet;
    for (RenderCommand *command : qAsConst(m_commands)) {
        delete command->m_stateSet;
        delete command;
    }
}

namespace {

template<int SortType>
struct AdjacentSubRangeFinder
{
    static bool adjacentSubRange(RenderCommand *, RenderCommand *)
    {
        Q_UNREACHABLE();
        return false;
    }
};

template<>
struct AdjacentSubRangeFinder<QSortPolicy::StateChangeCost>
{
    static bool adjacentSubRange(RenderCommand *a, RenderCommand *b)
    {
        return a->m_changeCost == b->m_changeCost;
    }
};

template<>
struct AdjacentSubRangeFinder<QSortPolicy::BackToFront>
{
    static bool adjacentSubRange(RenderCommand *a, RenderCommand *b)
    {
        return a->m_depth == b->m_depth;
    }
};

template<>
struct AdjacentSubRangeFinder<QSortPolicy::Material>
{
    static bool adjacentSubRange(RenderCommand *a, RenderCommand *b)
    {
        return a->m_shaderDna == b->m_shaderDna;
    }
};

template<>
struct AdjacentSubRangeFinder<QSortPolicy::FrontToBack>
{
    static bool adjacentSubRange(RenderCommand *a, RenderCommand *b)
    {
        return a->m_depth == b->m_depth;
    }
};

template<typename Predicate>
int advanceUntilNonAdjacent(const QVector<RenderCommand *> &commands,
                            const int beg, const int end, Predicate pred)
{
    int i = beg + 1;
    while (i < end) {
        if (!pred(*(commands.begin() + beg), *(commands.begin() + i)))
            break;
        ++i;
    }
    return i;
}


using CommandIt = QVector<RenderCommand *>::iterator;

template<int SortType>
struct SubRangeSorter
{
    static void sortSubRange(CommandIt begin, const CommandIt end)
    {
        Q_UNUSED(begin);
        Q_UNUSED(end);
        Q_UNREACHABLE();
    }
};

template<>
struct SubRangeSorter<QSortPolicy::StateChangeCost>
{
    static void sortSubRange(CommandIt begin, const CommandIt end)
    {
        std::stable_sort(begin, end, [] (RenderCommand *a, RenderCommand *b) {
            return a->m_changeCost > b->m_changeCost;
        });
    }
};

template<>
struct SubRangeSorter<QSortPolicy::BackToFront>
{
    static void sortSubRange(CommandIt begin, const CommandIt end)
    {
        std::stable_sort(begin, end, [] (RenderCommand *a, RenderCommand *b) {
            return a->m_depth > b->m_depth;
        });
    }
};

template<>
struct SubRangeSorter<QSortPolicy::Material>
{
    static void sortSubRange(CommandIt begin, const CommandIt end)
    {
        // First we sort by shaderDNA
        std::stable_sort(begin, end, [] (RenderCommand *a, RenderCommand *b) {
            return a->m_shaderDna > b->m_shaderDna;
        });
    }
};

template<>
struct SubRangeSorter<QSortPolicy::FrontToBack>
{
    static void sortSubRange(CommandIt begin, const CommandIt end)
    {
        std::stable_sort(begin, end, [] (RenderCommand *a, RenderCommand *b) {
            return a->m_depth < b->m_depth;
        });
    }
};

int findSubRange(const QVector<RenderCommand *> &commands,
                 const int begin, const int end,
                 const QSortPolicy::SortType sortType)
{
    switch (sortType) {
    case QSortPolicy::StateChangeCost:
        return advanceUntilNonAdjacent(commands, begin, end, AdjacentSubRangeFinder<QSortPolicy::StateChangeCost>::adjacentSubRange);
    case QSortPolicy::BackToFront:
        return advanceUntilNonAdjacent(commands, begin, end, AdjacentSubRangeFinder<QSortPolicy::BackToFront>::adjacentSubRange);
    case QSortPolicy::Material:
        return advanceUntilNonAdjacent(commands, begin, end, AdjacentSubRangeFinder<QSortPolicy::Material>::adjacentSubRange);
    case QSortPolicy::FrontToBack:
        return advanceUntilNonAdjacent(commands, begin, end, AdjacentSubRangeFinder<QSortPolicy::FrontToBack>::adjacentSubRange);
    default:
        Q_UNREACHABLE();
        return end;
    }
}

void sortByMaterial(QVector<RenderCommand *> &commands, int begin, const int end)
{
    // We try to arrange elements so that their rendering cost is minimized for a given shader
    int rangeEnd = advanceUntilNonAdjacent(commands, begin, end, AdjacentSubRangeFinder<QSortPolicy::Material>::adjacentSubRange);
    while (begin != end) {
        if (begin + 1 < rangeEnd) {
            std::stable_sort(commands.begin() + begin + 1, commands.begin() + rangeEnd, [] (RenderCommand *a, RenderCommand *b){
                return a->m_material.handle() < b->m_material.handle();
            });
        }
        begin = rangeEnd;
        rangeEnd = advanceUntilNonAdjacent(commands, begin, end, AdjacentSubRangeFinder<QSortPolicy::Material>::adjacentSubRange);
    }
}

void sortCommandRange(QVector<RenderCommand *> &commands, int begin, const int end, const int level,
                      const QVector<Qt3DRender::QSortPolicy::SortType> &sortingTypes)
{
    if (level >= sortingTypes.size())
        return;

    switch (sortingTypes.at(level)) {
    case QSortPolicy::StateChangeCost:
        SubRangeSorter<QSortPolicy::StateChangeCost>::sortSubRange(commands.begin() + begin, commands.begin() + end);
        break;
    case QSortPolicy::BackToFront:
        SubRangeSorter<QSortPolicy::BackToFront>::sortSubRange(commands.begin() + begin, commands.begin() + end);
        break;
    case QSortPolicy::Material:
        // Groups all same shader DNA together
        SubRangeSorter<QSortPolicy::Material>::sortSubRange(commands.begin() + begin, commands.begin() + end);
        // Group all same material together (same parameters most likely)
        sortByMaterial(commands, begin, end);
        break;
    case QSortPolicy::FrontToBack:
        SubRangeSorter<QSortPolicy::FrontToBack>::sortSubRange(commands.begin() + begin, commands.begin() + end);
        break;
    default:
        Q_UNREACHABLE();
    }

    // For all sub ranges of adjacent item for sortType[i]
    // Perform filtering with sortType[i + 1]
    int rangeEnd = findSubRange(commands, begin, end, sortingTypes.at(level));
    while (begin != end) {
        sortCommandRange(commands, begin, rangeEnd, level + 1, sortingTypes);
        begin = rangeEnd;
        rangeEnd = findSubRange(commands, begin, end, sortingTypes.at(level));
    }
}

} // anonymous

void RenderView::sort()
{
     sortCommandRange(m_commands, 0, m_commands.size(), 0, m_data.m_sortingTypes);

    // For RenderCommand with the same shader
    // We compute the adjacent change cost

    // Minimize uniform changes
    int i = 0;
    while (i < m_commands.size()) {
        int j = i;

        // Advance while commands share the same shader
        while (i < m_commands.size() && m_commands[j]->m_shaderDna == m_commands[i]->m_shaderDna)
            ++i;

        if (i - j > 0) { // Several commands have the same shader, so we minimize uniform changes
            PackUniformHash cachedUniforms = m_commands[j++]->m_parameterPack.uniforms();

            while (j < i) {
                // We need the reference here as we are modifying the original container
                // not the copy
                PackUniformHash &uniforms = m_commands.at(j)->m_parameterPack.m_uniforms;
                PackUniformHash::iterator it = uniforms.begin();
                const PackUniformHash::iterator end = uniforms.end();

                while (it != end) {
                    // We are comparing the values:
                    // - raw uniform values
                    // - the texture Node id if the uniform represents a texture
                    // since all textures are assigned texture units before the RenderCommands
                    // sharing the same material (shader) are rendered, we can't have the case
                    // where two uniforms, referencing the same texture eventually have 2 different
                    // texture unit values
                    const UniformValue refValue = cachedUniforms.value(it.key());
                    if (it.value() == refValue) {
                        it = uniforms.erase(it);
                    } else {
                        cachedUniforms.insert(it.key(), it.value());
                        ++it;
                    }
                }
                ++j;
            }
        }
    }
}

void RenderView::setRenderer(Renderer *renderer)
{
    m_renderer = renderer;
    m_manager = renderer->nodeManagers();
}

void RenderView::addClearBuffers(const ClearBuffers *cb) {
    QClearBuffers::BufferTypeFlags type = cb->type();

    if (type & QClearBuffers::StencilBuffer) {
        m_clearStencilValue = cb->clearStencilValue();
        m_clearBuffer |= QClearBuffers::StencilBuffer;
    }
    if (type & QClearBuffers::DepthBuffer) {
        m_clearDepthValue = cb->clearDepthValue();
        m_clearBuffer |= QClearBuffers::DepthBuffer;
    }
    // keep track of global ClearColor (if set) and collect all DrawBuffer-specific
    // ClearColors
    if (type & QClearBuffers::ColorBuffer) {
        ClearBufferInfo clearBufferInfo;
        clearBufferInfo.clearColor = cb->clearColor();

        if (cb->clearsAllColorBuffers()) {
            m_globalClearColorBuffer = clearBufferInfo;
            m_clearBuffer |= QClearBuffers::ColorBuffer;
        } else {
            if (cb->bufferId()) {
                const RenderTargetOutput *targetOutput = m_manager->attachmentManager()->lookupResource(cb->bufferId());
                if (targetOutput) {
                    clearBufferInfo.attchmentPoint = targetOutput->point();
                    // Note: a job is later performed to find the drawIndex from the buffer attachment point
                    // using the AttachmentPack
                    m_specificClearColorBuffers.push_back(clearBufferInfo);
                }
            }
        }
    }
}

// If we are there, we know that entity had a GeometryRenderer + Material
QVector<RenderCommand *> RenderView::buildDrawRenderCommands(const QVector<Entity *> &entities) const
{
    // Note: since many threads can be building render commands
    // we need to ensure that the UniformBlockValueBuilder they are using
    // is only accessed from the same thread
    UniformBlockValueBuilder *builder = new UniformBlockValueBuilder();
    builder->shaderDataManager = m_manager->shaderDataManager();
    builder->textureManager = m_manager->textureManager();
    m_localData.setLocalData(builder);

    QVector<RenderCommand *> commands;
    commands.reserve(entities.size());

    for (Entity *entity : entities) {
        GeometryRenderer *geometryRenderer = nullptr;
        HGeometryRenderer geometryRendererHandle = entity->componentHandle<GeometryRenderer>();

        // There is a geometry renderer with geometry
        if ((geometryRenderer = m_manager->geometryRendererManager()->data(geometryRendererHandle)) != nullptr
                && geometryRenderer->isEnabled()
                && !geometryRenderer->geometryId().isNull()) {

            const Qt3DCore::QNodeId materialComponentId = entity->componentUuid<Material>();
            const HMaterial materialHandle = entity->componentHandle<Material>();
            const  QVector<RenderPassParameterData> renderPassData = m_parameters.value(materialComponentId);
            HGeometry geometryHandle = m_manager->lookupHandle<Geometry, GeometryManager, HGeometry>(geometryRenderer->geometryId());
            Geometry *geometry = m_manager->data<Geometry, GeometryManager>(geometryHandle);

            // 1 RenderCommand per RenderPass pass on an Entity with a Mesh
            for (const RenderPassParameterData &passData : renderPassData) {
                // Add the RenderPass Parameters
                RenderCommand *command = new RenderCommand();

                // Project the camera-to-object-center vector onto the camera
                // view vector. This gives a depth value suitable as the key
                // for BackToFront sorting.
                command->m_depth = Vector3D::dotProduct(entity->worldBoundingVolume()->center() - m_data.m_eyePos, m_data.m_eyeViewDir);

                command->m_geometry = geometryHandle;
                command->m_geometryRenderer = geometryRendererHandle;
                command->m_material = materialHandle;
                // For RenderPass based states we use the globally set RenderState
                // if no renderstates are defined as part of the pass. That means:
                // RenderPass { renderStates: [] } will use the states defined by
                // StateSet in the FrameGraph
                RenderPass *pass = passData.pass;
                if (pass->hasRenderStates()) {
                    command->m_stateSet = new RenderStateSet();
                    addStatesToRenderStateSet(command->m_stateSet, pass->renderStates(), m_manager->renderStateManager());
                    if (m_stateSet != nullptr)
                        command->m_stateSet->merge(m_stateSet);
                    command->m_changeCost = m_renderer->defaultRenderState()->changeCost(command->m_stateSet);
                }

                // Pick which lights to take in to account.
                // For now decide based on the distance by taking the MAX_LIGHTS closest lights.
                // Replace with more sophisticated mechanisms later.
                // Copy vector so that we can sort it concurrently and we only want to sort the one for the current command
                QVector<LightSource> lightSources = m_lightSources;
                if (lightSources.size() > 1) {
                    const Vector3D entityCenter = entity->worldBoundingVolume()->center();
                    std::sort(lightSources.begin(), lightSources.end(),
                              [&] (const LightSource &a, const LightSource &b) {
                        const float distA = entityCenter.distanceToPoint(a.entity->worldBoundingVolume()->center());
                        const float distB = entityCenter.distanceToPoint(b.entity->worldBoundingVolume()->center());
                        return distA < distB;
                    });
                }

                ParameterInfoList globalParameters = passData.parameterInfo;
                // setShaderAndUniforms can initialize a localData
                // make sure this is cleared before we leave this function
                setShaderAndUniforms(command,
                                     pass,
                                     globalParameters,
                                     entity,
                                     lightSources.mid(0, std::max(lightSources.size(), MAX_LIGHTS)),
                                     m_environmentLight);

                // Store all necessary information for actual drawing if command is valid
                command->m_isValid = !command->m_attributes.empty();
                if (command->m_isValid) {
                    // Update the draw command with what's going to be needed for the drawing
                    uint primitiveCount = geometryRenderer->vertexCount();
                    uint estimatedCount = 0;
                    Attribute *indexAttribute = nullptr;

                    const QVector<Qt3DCore::QNodeId> attributeIds = geometry->attributes();
                    for (Qt3DCore::QNodeId attributeId : attributeIds) {
                        Attribute *attribute = m_manager->attributeManager()->lookupResource(attributeId);
                        if (attribute->attributeType() == QAttribute::IndexAttribute)
                            indexAttribute = attribute;
                        else if (command->m_attributes.contains(attribute->nameId()))
                            estimatedCount = qMax(attribute->count(), estimatedCount);
                    }

                    // Update the draw command with all the information required for the drawing
                    command->m_drawIndexed = (indexAttribute != nullptr);
                    if (command->m_drawIndexed) {
                        command->m_indexAttributeDataType = GraphicsContext::glDataTypeFromAttributeDataType(indexAttribute->vertexBaseType());
                        command->m_indexAttributeByteOffset = indexAttribute->byteOffset();
                    }

                    // Use the count specified by the GeometryRender
                    // If not specified use the indexAttribute count if present
                    // Otherwise tries to use the count from the attribute with the highest count
                    if (primitiveCount == 0) {
                        if (indexAttribute)
                            primitiveCount = indexAttribute->count();
                        else
                            primitiveCount = estimatedCount;
                    }

                    command->m_primitiveCount = primitiveCount;
                    command->m_primitiveType = geometryRenderer->primitiveType();
                    command->m_primitiveRestartEnabled = geometryRenderer->primitiveRestartEnabled();
                    command->m_restartIndexValue = geometryRenderer->restartIndexValue();
                    command->m_firstInstance = geometryRenderer->firstInstance();
                    command->m_instanceCount = geometryRenderer->instanceCount();
                    command->m_firstVertex = geometryRenderer->firstVertex();
                    command->m_indexOffset = geometryRenderer->indexOffset();
                    command->m_verticesPerPatch = geometryRenderer->verticesPerPatch();
                }

                commands.append(command);
            }
        }
    }

    // We reset the local data once we are done with it
    m_localData.setLocalData(nullptr);

    return commands;
}

QVector<RenderCommand *> RenderView::buildComputeRenderCommands(const QVector<Entity *> &entities) const
{
    // Note: since many threads can be building render commands
    // we need to ensure that the UniformBlockValueBuilder they are using
    // is only accessed from the same thread
    UniformBlockValueBuilder *builder = new UniformBlockValueBuilder();
    builder->shaderDataManager = m_manager->shaderDataManager();
    builder->textureManager = m_manager->textureManager();
    m_localData.setLocalData(builder);

    // If the RenderView contains only a ComputeDispatch then it cares about
    // A ComputeDispatch is also implicitely a NoDraw operation
    // enabled flag
    // layer component
    // material/effect/technique/parameters/filters/
    QVector<RenderCommand *> commands;
    commands.reserve(entities.size());
    for (Entity *entity : entities) {
        ComputeCommand *computeJob = nullptr;
        if ((computeJob = entity->renderComponent<ComputeCommand>()) != nullptr
                && computeJob->isEnabled()) {

            const Qt3DCore::QNodeId materialComponentId = entity->componentUuid<Material>();
            const  QVector<RenderPassParameterData> renderPassData = m_parameters.value(materialComponentId);

            // 1 RenderCommand per RenderPass pass on an Entity with a Mesh
            for (const RenderPassParameterData &passData : renderPassData) {
                // Add the RenderPass Parameters
                RenderCommand *command = new RenderCommand();
                RenderPass *pass = passData.pass;

                if (pass->hasRenderStates()) {
                    command->m_stateSet = new RenderStateSet();
                    addStatesToRenderStateSet(command->m_stateSet, pass->renderStates(), m_manager->renderStateManager());

                    // Merge per pass stateset with global stateset
                    // so that the local stateset only overrides
                    if (m_stateSet != nullptr)
                        command->m_stateSet->merge(m_stateSet);
                    command->m_changeCost = m_renderer->defaultRenderState()->changeCost(command->m_stateSet);
                }

                command->m_type = RenderCommand::Compute;
                command->m_workGroups[0] = std::max(m_workGroups[0], computeJob->x());
                command->m_workGroups[1] = std::max(m_workGroups[1], computeJob->y());
                command->m_workGroups[2] = std::max(m_workGroups[2], computeJob->z());

                ParameterInfoList globalParameters = passData.parameterInfo;
                setShaderAndUniforms(command,
                                     pass,
                                     globalParameters,
                                     entity,
                                     QVector<LightSource>(),
                                     nullptr);
                commands.append(command);
            }
        }
    }

    // We reset the local data once we are done with it
    m_localData.setLocalData(nullptr);

    return commands;
}

void RenderView::updateMatrices()
{
    if (m_data.m_renderCameraNode && m_data.m_renderCameraLens && m_data.m_renderCameraLens->isEnabled()) {
        const Matrix4x4 cameraWorld = *(m_data.m_renderCameraNode->worldTransform());
        setViewMatrix(m_data.m_renderCameraLens->viewMatrix(cameraWorld));

        setViewProjectionMatrix(m_data.m_renderCameraLens->projection() * viewMatrix());
        //To get the eyePosition of the camera, we need to use the inverse of the
        //camera's worldTransform matrix.
        const Matrix4x4 inverseWorldTransform = viewMatrix().inverted();
        const Vector3D eyePosition(inverseWorldTransform.column(3));
        setEyePosition(eyePosition);

        // Get the viewing direction of the camera. Use the normal matrix to
        // ensure non-uniform scale works too.
        const QMatrix3x3 normalMat = convertToQMatrix4x4(m_data.m_viewMatrix).normalMatrix();
        // dir = normalize(QVector3D(0, 0, -1) * normalMat)
        setEyeViewDirection(Vector3D(-normalMat(2, 0), -normalMat(2, 1), -normalMat(2, 2)).normalized());
    }
}

void RenderView::setUniformValue(ShaderParameterPack &uniformPack, int nameId, const UniformValue &value) const
{
    // At this point a uniform value can only be a scalar type
    // or a Qt3DCore::QNodeId corresponding to a Texture
    // ShaderData/Buffers would be handled as UBO/SSBO and would therefore
    // not be in the default uniform block
    if (value.valueType() == UniformValue::NodeId) {
        const Qt3DCore::QNodeId *nodeIds = value.constData<Qt3DCore::QNodeId>();

        const int uniformArraySize = value.byteSize() / sizeof(Qt3DCore::QNodeId);
        for (int i = 0; i < uniformArraySize; ++i) {
            const Qt3DCore::QNodeId texId = nodeIds[i];
            const Texture *tex =  m_manager->textureManager()->lookupResource(texId);
            if (tex != nullptr)
                uniformPack.setTexture(nameId, i, texId);
        }

        UniformValue textureValue(uniformArraySize * sizeof(int), UniformValue::TextureValue);
        std::fill(textureValue.data<int>(), textureValue.data<int>() + uniformArraySize, -1);
        uniformPack.setUniform(nameId, textureValue);
    } else {
        uniformPack.setUniform(nameId, value);
    }
}

void RenderView::setStandardUniformValue(ShaderParameterPack &uniformPack,
                                         int glslNameId,
                                         int nameId,
                                         Entity *entity,
                                         const Matrix4x4 &worldTransform) const
{
    uniformPack.setUniform(glslNameId, standardUniformValue(ms_standardUniformSetters[nameId], entity, worldTransform));
}

void RenderView::setUniformBlockValue(ShaderParameterPack &uniformPack,
                                      Shader *shader,
                                      const ShaderUniformBlock &block,
                                      const UniformValue &value) const
{
    Q_UNUSED(shader)

    if (value.valueType() == UniformValue::NodeId) {

        Buffer *buffer = nullptr;
        if ((buffer = m_manager->bufferManager()->lookupResource(*value.constData<Qt3DCore::QNodeId>())) != nullptr) {
            BlockToUBO uniformBlockUBO;
            uniformBlockUBO.m_blockIndex = block.m_index;
            uniformBlockUBO.m_bufferID = buffer->peerId();
            uniformBlockUBO.m_needsUpdate = false;
            uniformPack.setUniformBuffer(std::move(uniformBlockUBO));
            // Buffer update to GL buffer will be done at render time
        }


        //ShaderData *shaderData = nullptr;
        // if ((shaderData = m_manager->shaderDataManager()->lookupResource(value.value<Qt3DCore::QNodeId>())) != nullptr) {
        // UBO are indexed by <ShaderId, ShaderDataId> so that a same QShaderData can be used among different shaders
        // while still making sure that if they have a different layout everything will still work
        // If two shaders define the same block with the exact same layout, in that case the UBO could be shared
        // but how do we know that ? We'll need to compare ShaderUniformBlocks

        // Note: we assume that if a buffer is shared across multiple shaders
        // then it implies that they share the same layout

        // Temporarly disabled

        //        BufferShaderKey uboKey(shaderData->peerId(),
        //                               shader->peerId());

        //        BlockToUBO uniformBlockUBO;
        //        uniformBlockUBO.m_blockIndex = block.m_index;
        //        uniformBlockUBO.m_shaderDataID = shaderData->peerId();
        //        bool uboNeedsUpdate = false;

        //        // build UBO at uboId if not created before
        //        if (!m_manager->glBufferManager()->contains(uboKey)) {
        //            m_manager->glBufferManager()->getOrCreateResource(uboKey);
        //            uboNeedsUpdate = true;
        //        }

        //        // If shaderData  has been updated (property has changed or one of the nested properties has changed)
        //        // foreach property defined in the QShaderData, we try to fill the value of the corresponding active uniform(s)
        //        // for all the updated properties (all the properties if the UBO was just created)
        //        if (shaderData->updateViewTransform(*m_data->m_viewMatrix) || uboNeedsUpdate) {
        //            // Clear previous values remaining in the hash
        //            m_data->m_uniformBlockBuilder.activeUniformNamesToValue.clear();
        //            // Update only update properties if uboNeedsUpdate is true, otherwise update the whole block
        //            m_data->m_uniformBlockBuilder.updatedPropertiesOnly = uboNeedsUpdate;
        //            // Retrieve names and description of each active uniforms in the uniform block
        //            m_data->m_uniformBlockBuilder.uniforms = shader->activeUniformsForUniformBlock(block.m_index);
        //            // Builds the name-value map for the block
        //            m_data->m_uniformBlockBuilder.buildActiveUniformNameValueMapStructHelper(shaderData, block.m_name);
        //            if (!uboNeedsUpdate)
        //                shaderData->markDirty();
        //            // copy the name-value map into the BlockToUBO
        //            uniformBlockUBO.m_updatedProperties = m_data->m_uniformBlockBuilder.activeUniformNamesToValue;
        //            uboNeedsUpdate = true;
        //        }

        //        uniformBlockUBO.m_needsUpdate = uboNeedsUpdate;
        //        uniformPack.setUniformBuffer(std::move(uniformBlockUBO));
        // }
    }
}

void RenderView::setShaderStorageValue(ShaderParameterPack &uniformPack,
                                       Shader *shader,
                                       const ShaderStorageBlock &block,
                                       const UniformValue &value) const
{
    Q_UNUSED(shader)
    if (value.valueType() == UniformValue::NodeId) {
        Buffer *buffer = nullptr;
        if ((buffer = m_manager->bufferManager()->lookupResource(*value.constData<Qt3DCore::QNodeId>())) != nullptr) {
            BlockToSSBO shaderStorageBlock;
            shaderStorageBlock.m_blockIndex = block.m_index;
            shaderStorageBlock.m_bufferID = buffer->peerId();
            shaderStorageBlock.m_bindingIndex = block.m_binding;
            uniformPack.setShaderStorageBuffer(shaderStorageBlock);
            // Buffer update to GL buffer will be done at render time
        }
    }
}

void RenderView::setDefaultUniformBlockShaderDataValue(ShaderParameterPack &uniformPack, Shader *shader, ShaderData *shaderData, const QString &structName) const
{
    UniformBlockValueBuilder *builder = m_localData.localData();
    builder->activeUniformNamesToValue.clear();

    // Set the view matrix to be used to transform "Transformed" properties in the ShaderData
    builder->viewMatrix = m_data.m_viewMatrix;
    // Force to update the whole block
    builder->updatedPropertiesOnly = false;
    // Retrieve names and description of each active uniforms in the uniform block
    builder->uniforms = shader->activeUniformsForUniformBlock(-1);
    // Build name-value map for the block
    builder->buildActiveUniformNameValueMapStructHelper(shaderData, structName);
    // Set uniform values for each entrie of the block name-value map
    QHash<int, QVariant>::const_iterator activeValuesIt = builder->activeUniformNamesToValue.constBegin();
    const QHash<int, QVariant>::const_iterator activeValuesEnd = builder->activeUniformNamesToValue.constEnd();

    // TO DO: Make the ShaderData store UniformValue
    while (activeValuesIt != activeValuesEnd) {
        setUniformValue(uniformPack, activeValuesIt.key(), UniformValue::fromVariant(activeValuesIt.value()));
        ++activeValuesIt;
    }
}

void RenderView::setShaderAndUniforms(RenderCommand *command,
                                      RenderPass *rPass,
                                      ParameterInfoList &parameters,
                                      Entity *entity,
                                      const QVector<LightSource> &activeLightSources,
                                      EnvironmentLight *environmentLight) const
{
    // The VAO Handle is set directly in the renderer thread so as to avoid having to use a mutex here
    // Set shader, technique, and effect by basically doing :
    // ShaderProgramManager[MaterialManager[frontentEntity->id()]->Effect->Techniques[TechniqueFilter->name]->RenderPasses[RenderPassFilter->name]];
    // The Renderer knows that if one of those is null, a default material / technique / effect as to be used

    // Find all RenderPasses (in order) matching values set in the RenderPassFilter
    // Get list of parameters for the Material, Effect, and Technique
    // For each ParameterBinder in the RenderPass -> create a QUniformPack
    // Once that works, improve that to try and minimize QUniformPack updates

    if (rPass != nullptr) {
        // Index Shader by Shader UUID
        command->m_shader = m_manager->lookupHandle<Shader, ShaderManager, HShader>(rPass->shaderProgram());
        Shader *shader = m_manager->data<Shader, ShaderManager>(command->m_shader);
        if (shader != nullptr && shader->isLoaded()) {
            command->m_shaderDna = shader->dna();

            // Builds the QUniformPack, sets shader standard uniforms and store attributes name / glname bindings
            // If a parameter is defined and not found in the bindings it is assumed to be a binding of Uniform type with the glsl name
            // equals to the parameter name
            const QVector<int> uniformNamesIds = shader->uniformsNamesIds();
            const QVector<int> uniformBlockNamesIds = shader->uniformBlockNamesIds();
            const QVector<int> shaderStorageBlockNamesIds = shader->storageBlockNamesIds();
            const QVector<int> attributeNamesIds = shader->attributeNamesIds();

            // Set fragData Name and index
            // Later on we might want to relink the shader if attachments have changed
            // But for now we set them once and for all
            QHash<QString, int> fragOutputs;
            if (!m_renderTarget.isNull() && !shader->isLoaded()) {
                const auto atts = m_attachmentPack.attachments();
                for (const Attachment &att : atts) {
                    if (att.m_point <= QRenderTargetOutput::Color15)
                        fragOutputs.insert(att.m_name, att.m_point);
                }
            }

            if (!uniformNamesIds.isEmpty() || !attributeNamesIds.isEmpty() ||
                    !shaderStorageBlockNamesIds.isEmpty() || !attributeNamesIds.isEmpty()) {

                // Set default standard uniforms without bindings
                const Matrix4x4 worldTransform = *(entity->worldTransform());
                for (const int uniformNameId : uniformNamesIds) {
                    if (ms_standardUniformSetters.contains(uniformNameId))
                        setStandardUniformValue(command->m_parameterPack, uniformNameId, uniformNameId, entity, worldTransform);
                }

                // Set default attributes
                for (const int attributeNameId : attributeNamesIds)
                    command->m_attributes.push_back(attributeNameId);

                // Parameters remaining could be
                // -> uniform scalar / vector
                // -> uniform struct / arrays
                // -> uniform block / array (4.3)
                // -> ssbo block / array (4.3)

                ParameterInfoList::const_iterator it = parameters.cbegin();
                const ParameterInfoList::const_iterator parametersEnd = parameters.cend();

                while (it != parametersEnd) {
                    Parameter *param = m_manager->data<Parameter, ParameterManager>(it->handle);
                    const UniformValue &uniformValue = param->uniformValue();
                    if (uniformNamesIds.contains(it->nameId)) { // Parameter is a regular uniform
                        setUniformValue(command->m_parameterPack, it->nameId, uniformValue);
                    } else if (uniformBlockNamesIds.indexOf(it->nameId) != -1) { // Parameter is a uniform block
                        setUniformBlockValue(command->m_parameterPack, shader, shader->uniformBlockForBlockNameId(it->nameId), uniformValue);
                    } else if (shaderStorageBlockNamesIds.indexOf(it->nameId) != -1) { // Parameters is a SSBO
                        setShaderStorageValue(command->m_parameterPack, shader, shader->storageBlockForBlockNameId(it->nameId), uniformValue);
                    } else { // Parameter is a struct
                        ShaderData *shaderData = nullptr;
                        if (uniformValue.valueType() == UniformValue::NodeId &&
                                (shaderData = m_manager->shaderDataManager()->lookupResource(*uniformValue.constData<Qt3DCore::QNodeId>())) != nullptr) {
                            // Try to check if we have a struct or array matching a QShaderData parameter
                            setDefaultUniformBlockShaderDataValue(command->m_parameterPack, shader, shaderData, StringToInt::lookupString(it->nameId));
                        }
                        // Otherwise: param unused by current shader
                    }
                    ++it;
                }

                // Lights

                int lightIdx = 0;
                for (const LightSource &lightSource : activeLightSources) {
                    if (lightIdx == MAX_LIGHTS)
                        break;
                    Entity *lightEntity = lightSource.entity;
                    const Vector3D worldPos = lightEntity->worldBoundingVolume()->center();
                    for (Light *light : lightSource.lights) {
                        if (!light->isEnabled())
                            continue;

                        ShaderData *shaderData = m_manager->shaderDataManager()->lookupResource(light->shaderData());
                        if (!shaderData)
                            continue;

                        if (lightIdx == MAX_LIGHTS)
                            break;

                        // Note: implicit conversion of values to UniformValue
                        setUniformValue(command->m_parameterPack, LIGHT_POSITION_NAMES[lightIdx], worldPos);
                        setUniformValue(command->m_parameterPack, LIGHT_TYPE_NAMES[lightIdx], int(QAbstractLight::PointLight));
                        setUniformValue(command->m_parameterPack, LIGHT_COLOR_NAMES[lightIdx], Vector3D(1.0f, 1.0f, 1.0f));
                        setUniformValue(command->m_parameterPack, LIGHT_INTENSITY_NAMES[lightIdx], 0.5f);

                        // There is no risk in doing that even if multithreaded
                        // since we are sure that a shaderData is unique for a given light
                        // and won't ever be referenced as a Component either
                        Matrix4x4 *worldTransform = lightEntity->worldTransform();
                        if (worldTransform)
                            shaderData->updateWorldTransform(*worldTransform);

                        setDefaultUniformBlockShaderDataValue(command->m_parameterPack, shader, shaderData, LIGHT_STRUCT_NAMES[lightIdx]);
                        ++lightIdx;
                    }
                }

                if (uniformNamesIds.contains(LIGHT_COUNT_NAME_ID))
                    setUniformValue(command->m_parameterPack, LIGHT_COUNT_NAME_ID, UniformValue(qMax((environmentLight ? 0 : 1), lightIdx)));

                // If no active light sources and no environment light, add a default light
                if (activeLightSources.isEmpty() && !environmentLight) {
                    // Note: implicit conversion of values to UniformValue
                    setUniformValue(command->m_parameterPack, LIGHT_POSITION_NAMES[0], Vector3D(10.0f, 10.0f, 0.0f));
                    setUniformValue(command->m_parameterPack, LIGHT_TYPE_NAMES[0], int(QAbstractLight::PointLight));
                    setUniformValue(command->m_parameterPack, LIGHT_COLOR_NAMES[0], Vector3D(1.0f, 1.0f, 1.0f));
                    setUniformValue(command->m_parameterPack, LIGHT_INTENSITY_NAMES[0], 0.5f);
                }

                // Environment Light
                int envLightCount = 0;
                if (environmentLight && environmentLight->isEnabled()) {
                    ShaderData *shaderData = m_manager->shaderDataManager()->lookupResource(environmentLight->shaderData());
                    if (shaderData) {
                        setDefaultUniformBlockShaderDataValue(command->m_parameterPack, shader, shaderData, QStringLiteral("envLight"));
                        envLightCount = 1;
                    }
                } else {
                    // with some drivers, samplers (like the envbox sampler) need to be bound even though
                    // they may not be actually used, otherwise draw calls can fail
                    static const int irradianceId = StringToInt::lookupId(QLatin1String("envLight.irradiance"));
                    static const int specularId = StringToInt::lookupId(QLatin1String("envLight.specular"));
                    setUniformValue(command->m_parameterPack, irradianceId, m_renderer->submissionContext()->maxTextureUnitsCount());
                    setUniformValue(command->m_parameterPack, specularId, m_renderer->submissionContext()->maxTextureUnitsCount());
                }
                setUniformValue(command->m_parameterPack, StringToInt::lookupId(QStringLiteral("envLightCount")), envLightCount);
            }
            // Set frag outputs in the shaders if hash not empty
            if (!fragOutputs.isEmpty())
                shader->setFragOutputs(fragOutputs);
        }
    }
    else {
        qCWarning(Render::Backend) << Q_FUNC_INFO << "Using default effect as none was provided";
    }
}

bool RenderView::hasBlitFramebufferInfo() const
{
    return m_hasBlitFramebufferInfo;
}

void RenderView::setHasBlitFramebufferInfo(bool hasBlitFramebufferInfo)
{
    m_hasBlitFramebufferInfo = hasBlitFramebufferInfo;
}

BlitFramebufferInfo RenderView::blitFrameBufferInfo() const
{
    return m_blitFrameBufferInfo;
}

void RenderView::setBlitFrameBufferInfo(const BlitFramebufferInfo &blitFrameBufferInfo)
{
    m_blitFrameBufferInfo = blitFrameBufferInfo;
}

bool RenderView::isDownloadBuffersEnable() const
{
    return m_isDownloadBuffersEnable;
}

void RenderView::setIsDownloadBuffersEnable(bool isDownloadBuffersEnable)
{
    m_isDownloadBuffersEnable = isDownloadBuffersEnable;
}

} // namespace Render
} // namespace Qt3DRender

QT_END_NAMESPACE
