/****************************************************************************
**
** Copyright (C) 2018 Klaralvdalens Datakonsult AB (KDAB).
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

#include <QtTest/QTest>
#include <QtCore/private/qfactoryloader_p.h>
#include <Qt3DRender/private/qgeometryloaderinterface_p.h>
#include <Qt3DRender/private/qgeometryloaderfactory_p.h>

using namespace Qt3DRender;

class tst_ObjGeometryLoader : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void checkOutOfBoundFaceAccess_data()
    {
        QTest::addColumn<QString>("filePath");

        QTest::newRow("invalid position index") << ":invalid_vertex_position.obj";
    }


    void checkOutOfBoundFaceAccess()
    {
        // GIVEN
        QFETCH(QString, filePath);

        QFactoryLoader geometryLoader(QGeometryLoaderFactory_iid,
                                      QLatin1String("/geometryloaders"),
                                      Qt::CaseInsensitive);

        QScopedPointer<QGeometryLoaderInterface> loader;
        loader.reset(qLoadPlugin<QGeometryLoaderInterface, QGeometryLoaderFactory>(&geometryLoader, QLatin1String("obj")));

        if (!loader)
            QSKIP("ObjLoaderPlugin not deployed");

        QFile file(filePath);
        QVERIFY(file.open(QIODevice::ReadOnly));

        // WHEN
        loader->load(&file);

        // THEN
        // -> shouldn't crash
    }
};

QTEST_MAIN(tst_ObjGeometryLoader)

#include "tst_objgeometryloader.moc"
