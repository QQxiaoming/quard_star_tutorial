/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
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
#include <Qt3DRender/qshaderprogram.h>
#include <Qt3DRender/qshaderprogrambuilder.h>
#include <Qt3DRender/private/qshaderprogrambuilder_p.h>
#include <QObject>
#include <QSignalSpy>
#include <Qt3DCore/qpropertyupdatedchange.h>
#include <Qt3DCore/private/qnodecreatedchangegenerator_p.h>
#include <Qt3DCore/qnodecreatedchange.h>
#include "testpostmanarbiter.h"

class tst_QShaderProgramBuilder : public QObject
{
    Q_OBJECT
public:
    tst_QShaderProgramBuilder()
        : QObject()
    {
        qRegisterMetaType<Qt3DRender::QShaderProgram*>("Qt3DRender::QShaderProgram*");
    }

private Q_SLOTS:
    void checkDefaultConstruction()
    {
        // GIVEN
        Qt3DRender::QShaderProgramBuilder builder;

        // THEN
        QVERIFY(!builder.shaderProgram());
        QVERIFY(builder.enabledLayers().isEmpty());
        QCOMPARE(builder.vertexShaderGraph(), QUrl());
        QCOMPARE(builder.tessellationControlShaderGraph(), QUrl());
        QCOMPARE(builder.tessellationEvaluationShaderGraph(), QUrl());
        QCOMPARE(builder.geometryShaderGraph(), QUrl());
        QCOMPARE(builder.fragmentShaderGraph(), QUrl());
        QCOMPARE(builder.computeShaderGraph(), QUrl());
    }

    void checkPropertyChanges()
    {
        // GIVEN
        Qt3DRender::QShaderProgramBuilder builder;

        {
            // WHEN
            QSignalSpy spy(&builder, SIGNAL(shaderProgramChanged(Qt3DRender::QShaderProgram*)));
            auto newValue = new Qt3DRender::QShaderProgram(&builder);
            builder.setShaderProgram(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QCOMPARE(builder.shaderProgram(), newValue);
            QCOMPARE(spy.count(), 1);

            // WHEN
            spy.clear();
            builder.setShaderProgram(newValue);

            // THEN
            QCOMPARE(builder.shaderProgram(), newValue);
            QCOMPARE(spy.count(), 0);
        }
        {
            // WHEN
            QSignalSpy spy(&builder, SIGNAL(enabledLayersChanged(QStringList)));
            const auto newValue = QStringList() << "foo" << "bar";
            builder.setEnabledLayers(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QCOMPARE(builder.enabledLayers(), newValue);
            QCOMPARE(spy.count(), 1);

            // WHEN
            spy.clear();
            builder.setEnabledLayers(newValue);

            // THEN
            QCOMPARE(builder.enabledLayers(), newValue);
            QCOMPARE(spy.count(), 0);
        }
        {
            // WHEN
            QSignalSpy spy(&builder, SIGNAL(vertexShaderGraphChanged(QUrl)));
            const auto newValue = QUrl::fromEncoded("qrc:/vertex.json");
            builder.setVertexShaderGraph(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QCOMPARE(builder.vertexShaderGraph(), newValue);
            QCOMPARE(spy.count(), 1);

            // WHEN
            spy.clear();
            builder.setVertexShaderGraph(newValue);

            // THEN
            QCOMPARE(builder.vertexShaderGraph(), newValue);
            QCOMPARE(spy.count(), 0);
        }
        {
            // WHEN
            QSignalSpy spy(&builder, SIGNAL(tessellationControlShaderGraphChanged(QUrl)));
            const auto newValue = QUrl::fromEncoded("qrc:/tesscontrol.json");
            builder.setTessellationControlShaderGraph(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QCOMPARE(builder.tessellationControlShaderGraph(), newValue);
            QCOMPARE(spy.count(), 1);

            // WHEN
            spy.clear();
            builder.setTessellationControlShaderGraph(newValue);

            // THEN
            QCOMPARE(builder.tessellationControlShaderGraph(), newValue);
            QCOMPARE(spy.count(), 0);
        }
        {
            // WHEN
            QSignalSpy spy(&builder, SIGNAL(tessellationEvaluationShaderGraphChanged(QUrl)));
            const auto newValue = QUrl::fromEncoded("qrc:/tesseval.json");
            builder.setTessellationEvaluationShaderGraph(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QCOMPARE(builder.tessellationEvaluationShaderGraph(), newValue);
            QCOMPARE(spy.count(), 1);

            // WHEN
            spy.clear();
            builder.setTessellationEvaluationShaderGraph(newValue);

            // THEN
            QCOMPARE(builder.tessellationEvaluationShaderGraph(), newValue);
            QCOMPARE(spy.count(), 0);
        }
        {
            // WHEN
            QSignalSpy spy(&builder, SIGNAL(geometryShaderGraphChanged(QUrl)));
            const auto newValue = QUrl::fromEncoded("qrc:/geometry.json");
            builder.setGeometryShaderGraph(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QCOMPARE(builder.geometryShaderGraph(), newValue);
            QCOMPARE(spy.count(), 1);

            // WHEN
            spy.clear();
            builder.setGeometryShaderGraph(newValue);

            // THEN
            QCOMPARE(builder.geometryShaderGraph(), newValue);
            QCOMPARE(spy.count(), 0);
        }
        {
            // WHEN
            QSignalSpy spy(&builder, SIGNAL(fragmentShaderGraphChanged(QUrl)));
            const auto newValue = QUrl::fromEncoded("qrc:/fragment.json");
            builder.setFragmentShaderGraph(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QCOMPARE(builder.fragmentShaderGraph(), newValue);
            QCOMPARE(spy.count(), 1);

            // WHEN
            spy.clear();
            builder.setFragmentShaderGraph(newValue);

            // THEN
            QCOMPARE(builder.fragmentShaderGraph(), newValue);
            QCOMPARE(spy.count(), 0);
        }
        {
            // WHEN
            QSignalSpy spy(&builder, SIGNAL(computeShaderGraphChanged(QUrl)));
            const auto newValue = QUrl::fromEncoded("qrc:/compute.json");
            builder.setComputeShaderGraph(newValue);

            // THEN
            QVERIFY(spy.isValid());
            QCOMPARE(builder.computeShaderGraph(), newValue);
            QCOMPARE(spy.count(), 1);

            // WHEN
            spy.clear();
            builder.setComputeShaderGraph(newValue);

            // THEN
            QCOMPARE(builder.computeShaderGraph(), newValue);
            QCOMPARE(spy.count(), 0);
        }
    }

    void checkShaderProgramBookkeeping()
    {
        // GIVEN
        QScopedPointer<Qt3DRender::QShaderProgramBuilder> builder(new Qt3DRender::QShaderProgramBuilder);
        {
            // WHEN
            Qt3DRender::QShaderProgram program;
            builder->setShaderProgram(&program);

            // THEN
            QCOMPARE(program.parent(), builder.data());
            QCOMPARE(builder->shaderProgram(), &program);
        }
        // THEN (Should not crash and effect be unset)
        QVERIFY(!builder->shaderProgram());

        {
            // WHEN
            Qt3DRender::QShaderProgramBuilder someOtherBuilder;
            QScopedPointer<Qt3DRender::QShaderProgram> program(new Qt3DRender::QShaderProgram(&someOtherBuilder));
            builder->setShaderProgram(program.data());

            // THEN
            QCOMPARE(program->parent(), &someOtherBuilder);
            QCOMPARE(builder->shaderProgram(), program.data());

            // WHEN
            builder.reset();
            program.reset();

            // THEN Should not crash when the effect is destroyed (tests for failed removal of destruction helper)
        }
    }

    void checkCreationData()
    {
        // GIVEN
        Qt3DRender::QShaderProgramBuilder builder;

        builder.setShaderProgram(new Qt3DRender::QShaderProgram(&builder));
        builder.setEnabledLayers({"foo", "bar"});
        builder.setVertexShaderGraph(QUrl::fromEncoded("qrc:/vertex.json"));
        builder.setTessellationControlShaderGraph(QUrl::fromEncoded("qrc:/tesscontrol.json"));
        builder.setTessellationEvaluationShaderGraph(QUrl::fromEncoded("qrc:/tesseval.json"));
        builder.setGeometryShaderGraph(QUrl::fromEncoded("qrc:/geometry.json"));
        builder.setFragmentShaderGraph(QUrl::fromEncoded("qrc:/fragment.json"));
        builder.setComputeShaderGraph(QUrl::fromEncoded("qrc:/compute.json"));

        // WHEN
        QVector<Qt3DCore::QNodeCreatedChangeBasePtr> creationChanges;

        {
            Qt3DCore::QNodeCreatedChangeGenerator creationChangeGenerator(&builder);
            creationChanges = creationChangeGenerator.creationChanges();
        }

        // THEN
        {
            QCOMPARE(creationChanges.size(), 2); // second one is for the shader program child

            const auto creationChangeData = qSharedPointerCast<Qt3DCore::QNodeCreatedChange<Qt3DRender::QShaderProgramBuilderData>>(creationChanges.first());
            const Qt3DRender::QShaderProgramBuilderData cloneData = creationChangeData->data;

            QCOMPARE(builder.shaderProgram()->id(), cloneData.shaderProgramId);
            QCOMPARE(builder.enabledLayers(), cloneData.enabledLayers);
            QCOMPARE(builder.vertexShaderGraph(), cloneData.vertexShaderGraph);
            QCOMPARE(builder.tessellationControlShaderGraph(), cloneData.tessellationControlShaderGraph);
            QCOMPARE(builder.tessellationEvaluationShaderGraph(), cloneData.tessellationEvaluationShaderGraph);
            QCOMPARE(builder.geometryShaderGraph(), cloneData.geometryShaderGraph);
            QCOMPARE(builder.fragmentShaderGraph(), cloneData.fragmentShaderGraph);
            QCOMPARE(builder.computeShaderGraph(), cloneData.computeShaderGraph);
            QCOMPARE(builder.id(), creationChangeData->subjectId());
            QCOMPARE(builder.isEnabled(), true);
            QCOMPARE(builder.isEnabled(), creationChangeData->isNodeEnabled());
            QCOMPARE(builder.metaObject(), creationChangeData->metaObject());
        }

        // WHEN
        builder.setEnabled(false);

        {
            Qt3DCore::QNodeCreatedChangeGenerator creationChangeGenerator(&builder);
            creationChanges = creationChangeGenerator.creationChanges();
        }

        // THEN
        {
            QCOMPARE(creationChanges.size(), 2); // second one is for the shader program child

            const auto creationChangeData = qSharedPointerCast<Qt3DCore::QNodeCreatedChange<Qt3DRender::QShaderProgramBuilderData>>(creationChanges.first());
            const Qt3DRender::QShaderProgramBuilderData cloneData = creationChangeData->data;

            QCOMPARE(builder.shaderProgram()->id(), cloneData.shaderProgramId);
            QCOMPARE(builder.enabledLayers(), cloneData.enabledLayers);
            QCOMPARE(builder.vertexShaderGraph(), cloneData.vertexShaderGraph);
            QCOMPARE(builder.tessellationControlShaderGraph(), cloneData.tessellationControlShaderGraph);
            QCOMPARE(builder.tessellationEvaluationShaderGraph(), cloneData.tessellationEvaluationShaderGraph);
            QCOMPARE(builder.geometryShaderGraph(), cloneData.geometryShaderGraph);
            QCOMPARE(builder.fragmentShaderGraph(), cloneData.fragmentShaderGraph);
            QCOMPARE(builder.computeShaderGraph(), cloneData.computeShaderGraph);
            QCOMPARE(builder.id(), creationChangeData->subjectId());
            QCOMPARE(builder.isEnabled(), false);
            QCOMPARE(builder.isEnabled(), creationChangeData->isNodeEnabled());
            QCOMPARE(builder.metaObject(), creationChangeData->metaObject());
        }
    }

    void checkShaderProgramUpdate()
    {
        // GIVEN
        TestArbiter arbiter;
        Qt3DRender::QShaderProgramBuilder builder;
        arbiter.setArbiterOnNode(&builder);
        auto program = new Qt3DRender::QShaderProgram(&builder);

        {
            // WHEN
            builder.setShaderProgram(program);
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 1);
            auto change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "shaderProgram");
            QCOMPARE(change->value().value<Qt3DCore::QNodeId>(), builder.shaderProgram()->id());
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

            arbiter.events.clear();
        }

        {
            // WHEN
            builder.setShaderProgram(program);
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }
    }

    void checkEnabledLayersUpdate()
    {
        // GIVEN
        TestArbiter arbiter;
        Qt3DRender::QShaderProgramBuilder builder;
        arbiter.setArbiterOnNode(&builder);
        const auto layers = QStringList() << "foo" << "bar";

        {
            // WHEN
            builder.setEnabledLayers(layers);
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 1);
            auto change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "enabledLayers");
            QCOMPARE(change->value().toStringList(), layers);
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

            arbiter.events.clear();
        }

        {
            // WHEN
            builder.setEnabledLayers(layers);
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }
    }

    void checkVertexShaderGraphUpdate()
    {
        // GIVEN
        TestArbiter arbiter;
        Qt3DRender::QShaderProgramBuilder builder;
        arbiter.setArbiterOnNode(&builder);

        {
            // WHEN
            builder.setVertexShaderGraph(QUrl::fromEncoded("qrc:/vertex.json"));
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 1);
            auto change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "vertexShaderGraph");
            QCOMPARE(change->value().value<QUrl>(), builder.vertexShaderGraph());
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

            arbiter.events.clear();
        }

        {
            // WHEN
            builder.setVertexShaderGraph(QUrl::fromEncoded("qrc:/vertex.json"));
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }

    }

    void checkTessellationControlShaderGraphUpdate()
    {
        // GIVEN
        TestArbiter arbiter;
        Qt3DRender::QShaderProgramBuilder builder;
        arbiter.setArbiterOnNode(&builder);

        {
            // WHEN
            builder.setTessellationControlShaderGraph(QUrl::fromEncoded("qrc:/tesscontrol.json"));
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 1);
            auto change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "tessellationControlShaderGraph");
            QCOMPARE(change->value().value<QUrl>(), builder.tessellationControlShaderGraph());
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

            arbiter.events.clear();
        }

        {
            // WHEN
            builder.setTessellationControlShaderGraph(QUrl::fromEncoded("qrc:/tesscontrol.json"));
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }

    }

    void checkTessellationEvaluationShaderGraphUpdate()
    {
        // GIVEN
        TestArbiter arbiter;
        Qt3DRender::QShaderProgramBuilder builder;
        arbiter.setArbiterOnNode(&builder);

        {
            // WHEN
            builder.setTessellationEvaluationShaderGraph(QUrl::fromEncoded("qrc:/tesseval.json"));
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 1);
            auto change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "tessellationEvaluationShaderGraph");
            QCOMPARE(change->value().value<QUrl>(), builder.tessellationEvaluationShaderGraph());
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

            arbiter.events.clear();
        }

        {
            // WHEN
            builder.setTessellationEvaluationShaderGraph(QUrl::fromEncoded("qrc:/tesseval.json"));
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }

    }

    void checkGeometryShaderGraphUpdate()
    {
        // GIVEN
        TestArbiter arbiter;
        Qt3DRender::QShaderProgramBuilder builder;
        arbiter.setArbiterOnNode(&builder);

        {
            // WHEN
            builder.setGeometryShaderGraph(QUrl::fromEncoded("qrc:/geometry.json"));
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 1);
            auto change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "geometryShaderGraph");
            QCOMPARE(change->value().value<QUrl>(), builder.geometryShaderGraph());
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

            arbiter.events.clear();
        }

        {
            // WHEN
            builder.setGeometryShaderGraph(QUrl::fromEncoded("qrc:/geometry.json"));
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }

    }

    void checkFragmentShaderGraphUpdate()
    {
        // GIVEN
        TestArbiter arbiter;
        Qt3DRender::QShaderProgramBuilder builder;
        arbiter.setArbiterOnNode(&builder);

        {
            // WHEN
            builder.setFragmentShaderGraph(QUrl::fromEncoded("qrc:/fragment.json"));
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 1);
            auto change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "fragmentShaderGraph");
            QCOMPARE(change->value().value<QUrl>(), builder.fragmentShaderGraph());
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

            arbiter.events.clear();
        }

        {
            // WHEN
            builder.setFragmentShaderGraph(QUrl::fromEncoded("qrc:/fragment.json"));
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }

    }

    void checkComputeShaderGraphUpdate()
    {
        // GIVEN
        TestArbiter arbiter;
        Qt3DRender::QShaderProgramBuilder builder;
        arbiter.setArbiterOnNode(&builder);

        {
            // WHEN
            builder.setComputeShaderGraph(QUrl::fromEncoded("qrc:/compute.json"));
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 1);
            auto change = arbiter.events.first().staticCast<Qt3DCore::QPropertyUpdatedChange>();
            QCOMPARE(change->propertyName(), "computeShaderGraph");
            QCOMPARE(change->value().value<QUrl>(), builder.computeShaderGraph());
            QCOMPARE(change->type(), Qt3DCore::PropertyUpdated);

            arbiter.events.clear();
        }

        {
            // WHEN
            builder.setComputeShaderGraph(QUrl::fromEncoded("qrc:/compute.json"));
            QCoreApplication::processEvents();

            // THEN
            QCOMPARE(arbiter.events.size(), 0);
        }

    }
};

QTEST_MAIN(tst_QShaderProgramBuilder)

#include "tst_qshaderprogrambuilder.moc"
