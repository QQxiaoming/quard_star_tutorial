/****************************************************************************
**
** Copyright (C) 2016 Paul Lemire <paul.lemire350@gmail.com>
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

#include <QtTest/QtTest>
#include <Qt3DCore/private/matrix4x4_sse_p.h>

using namespace Qt3DCore;

class tst_Matrix4x4_SSE: public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void defaultConstruction()
    {
        // GIVEN
        Matrix4x4_SSE mat4;

        // THEN
        QCOMPARE(mat4.m11(), 1.0f);
        QCOMPARE(mat4.m12(), 0.0f);
        QCOMPARE(mat4.m13(), 0.0f);
        QCOMPARE(mat4.m14(), 0.0f);

        QCOMPARE(mat4.m21(), 0.0f);
        QCOMPARE(mat4.m22(), 1.0f);
        QCOMPARE(mat4.m23(), 0.0f);
        QCOMPARE(mat4.m24(), 0.0f);

        QCOMPARE(mat4.m31(), 0.0f);
        QCOMPARE(mat4.m32(), 0.0f);
        QCOMPARE(mat4.m33(), 1.0f);
        QCOMPARE(mat4.m34(), 0.0f);

        QCOMPARE(mat4.m41(), 0.0f);
        QCOMPARE(mat4.m42(), 0.0f);
        QCOMPARE(mat4.m43(), 0.0f);
        QCOMPARE(mat4.m44(), 1.0f);

    }

    void checkExplicitConstruction()
    {
        // GIVEN
        Matrix4x4_SSE mat4(11.0f, 12.0f, 13.0f, 14.0f,
                           21.0f, 22.0f, 23.0f, 24.0f,
                           31.0f, 32.0f, 33.0f, 34.0f,
                           41.0f, 42.0f, 43.0f, 44.0f);

        // THEN
        QCOMPARE(mat4.m11(), 11.0f);
        QCOMPARE(mat4.m12(), 12.0f);
        QCOMPARE(mat4.m13(), 13.0f);
        QCOMPARE(mat4.m14(), 14.0f);

        QCOMPARE(mat4.m21(), 21.0f);
        QCOMPARE(mat4.m22(), 22.0f);
        QCOMPARE(mat4.m23(), 23.0f);
        QCOMPARE(mat4.m24(), 24.0f);

        QCOMPARE(mat4.m31(), 31.0f);
        QCOMPARE(mat4.m32(), 32.0f);
        QCOMPARE(mat4.m33(), 33.0f);
        QCOMPARE(mat4.m34(), 34.0f);

        QCOMPARE(mat4.m41(), 41.0f);
        QCOMPARE(mat4.m42(), 42.0f);
        QCOMPARE(mat4.m43(), 43.0f);
        QCOMPARE(mat4.m44(), 44.0f);
    }

    void checkTransposed()
    {
        // GIVEN
        Matrix4x4_SSE mat4(11.0f, 12.0f, 13.0f, 14.0f,
                           21.0f, 22.0f, 23.0f, 24.0f,
                           31.0f, 32.0f, 33.0f, 34.0f,
                           41.0f, 42.0f, 43.0f, 44.0f);

        // WHEN
        mat4 = mat4.transposed();

        // THEN
        QCOMPARE(mat4.m11(), 11.0f);
        QCOMPARE(mat4.m12(), 21.0f);
        QCOMPARE(mat4.m13(), 31.0f);
        QCOMPARE(mat4.m14(), 41.0f);

        QCOMPARE(mat4.m21(), 12.0f);
        QCOMPARE(mat4.m22(), 22.0f);
        QCOMPARE(mat4.m23(), 32.0f);
        QCOMPARE(mat4.m24(), 42.0f);

        QCOMPARE(mat4.m31(), 13.0f);
        QCOMPARE(mat4.m32(), 23.0f);
        QCOMPARE(mat4.m33(), 33.0f);
        QCOMPARE(mat4.m34(), 43.0f);

        QCOMPARE(mat4.m41(), 14.0f);
        QCOMPARE(mat4.m42(), 24.0f);
        QCOMPARE(mat4.m43(), 34.0f);
        QCOMPARE(mat4.m44(), 44.0f);
    }

    void checkMultiplication()
    {
        {
            // GIVEN
            QMatrix4x4 mat1;
            QMatrix4x4 mat2;

            mat1.rotate(45.0f, QVector3D(1.0f, 0.0f, 0.0f));
            mat2.translate(5.0f, 12.0f, 11.0f);

            const Matrix4x4_SSE mat1fast(mat1);
            const Matrix4x4_SSE mat2fast(mat2);

            // WHEN
            const Matrix4x4_SSE ret = mat1fast * mat2fast;

            // THEN
            QCOMPARE(ret.toQMatrix4x4(), mat1 * mat2);
        }
        {
            // GIVEN
            QMatrix4x4 mat1;
            QMatrix4x4 mat2;

            mat1.rotate(45.0f, QVector3D(1.0f, 0.0f, 0.0f));
            mat2.translate(5.0f, 12.0f, 11.0f);

            const Matrix4x4_SSE mat1fast(mat1);
            const Matrix4x4_SSE mat2fast(mat2);

            // WHEN
            const Matrix4x4_SSE ret = mat2fast * mat1fast;

            // THEN
            QCOMPARE(ret.toQMatrix4x4(), mat2 * mat1);
        }
    }

    void checkAddition()
    {
        {
            // GIVEN
            QMatrix4x4 mat1;
            QMatrix4x4 mat2;

            mat1.rotate(45.0f, QVector3D(1.0f, 0.0f, 0.0f));
            mat2.translate(5.0f, 12.0f, 11.0f);

            const Matrix4x4_SSE mat1fast(mat1);
            const Matrix4x4_SSE mat2fast(mat2);

            // WHEN
            const Matrix4x4_SSE ret = mat1fast + mat2fast;

            // THEN
            QCOMPARE(ret.toQMatrix4x4(), mat1 + mat2);
        }
        {
            // GIVEN
            QMatrix4x4 mat1;
            QMatrix4x4 mat2;

            mat1.rotate(45.0f, QVector3D(1.0f, 0.0f, 0.0f));
            mat2.translate(5.0f, 12.0f, 11.0f);

            const Matrix4x4_SSE mat1fast(mat1);
            const Matrix4x4_SSE mat2fast(mat2);

            // WHEN
            const Matrix4x4_SSE ret = mat2fast + mat1fast;

            // THEN
            QCOMPARE(ret.toQMatrix4x4(), mat2 + mat1);
        }
    }

    void checkSubstraction()
    {
        {
            // GIVEN
            QMatrix4x4 mat1;
            QMatrix4x4 mat2;

            mat1.rotate(45.0f, QVector3D(1.0f, 0.0f, 0.0f));
            mat2.translate(5.0f, 12.0f, 11.0f);

            const Matrix4x4_SSE mat1fast(mat1);
            const Matrix4x4_SSE mat2fast(mat2);

            // WHEN
            const Matrix4x4_SSE ret = mat1fast - mat2fast;

            // THEN
            QCOMPARE(ret.toQMatrix4x4(), mat1 - mat2);
        }
        {
            // GIVEN
            QMatrix4x4 mat1;
            QMatrix4x4 mat2;

            mat1.rotate(45.0f, QVector3D(1.0f, 0.0f, 0.0f));
            mat2.translate(5.0f, 12.0f, 11.0f);

            const Matrix4x4_SSE mat1fast(mat1);
            const Matrix4x4_SSE mat2fast(mat2);

            // WHEN
            const Matrix4x4_SSE ret = mat2fast - mat1fast;

            // THEN
            QCOMPARE(ret.toQMatrix4x4(), mat2 - mat1);
        }
    }

    void checkEquality()
    {
        {
            // GIVEN
            Matrix4x4_SSE c1;
            Matrix4x4_SSE c2;

            // THEN
            QVERIFY(c1 == c2);
        }
        {
            QMatrix4x4 tmp;
            tmp.translate(5.0f, 8.0f, 14.0f);

            // GIVEN
            Matrix4x4_SSE c1(tmp);
            Matrix4x4_SSE c2(tmp);

            // THEN
            QVERIFY(c1 == c2);
        }
        {
            QMatrix4x4 tmp;
            tmp.translate(5.0f, 8.0f, 14.0f);

            // GIVEN
            Matrix4x4_SSE c1;
            Matrix4x4_SSE c2(tmp);

            // THEN
            QVERIFY(!(c1 == c2));
        }
    }

    void checkInequality()
    {
        {
            // GIVEN
            Matrix4x4_SSE c1;
            Matrix4x4_SSE c2;

            // THEN
            QVERIFY(!(c1 != c2));
        }
        {
            // GIVEN
            QMatrix4x4 tmp;
            tmp.translate(5.0f, 8.0f, 14.0f);

            Matrix4x4_SSE c1(tmp);
            tmp.translate(3.0f, 9.0f, -4.0f);
            Matrix4x4_SSE c2(tmp);

            // THEN
            QVERIFY(c1 != c2);
        }
    }

    void checkMatrixVector4DMultiplication()
    {

        // GIVEN
        QMatrix4x4 tmpMat;
        QVector4D tmpVec4(1.0f, 2.0f, 3.0f, 4.0f);
        tmpMat.translate(5.0f, 8.0f, 14.0f);

        Matrix4x4_SSE mat(tmpMat);
        Vector4D vec4(tmpVec4);

        // WHEN
        const Vector4D resultingVec = mat * vec4;

        // THEN
        QCOMPARE(resultingVec.toQVector4D(), tmpMat * tmpVec4);
    }

    void checkVector4DMatrixMultiplication()
    {

        // GIVEN
        QMatrix4x4 tmpMat;
        QVector4D tmpVec4(1.0f, 2.0f, 3.0f, 4.0f);
        tmpMat.translate(5.0f, 8.0f, 14.0f);

        Matrix4x4_SSE mat(tmpMat);
        Vector4D vec4(tmpVec4);

        // WHEN
        const Vector4D resultingVec = vec4 * mat;

        // THEN
        QCOMPARE(resultingVec.toQVector4D(), tmpVec4 * tmpMat);
    }

    void checkMatrixVector3DMultiplication()
    {

        // GIVEN
        QMatrix4x4 tmpMat;
        QVector3D tmpVec3(1.0f, 2.0f, 3.0f);
        tmpMat.translate(5.0f, 8.0f, 14.0f);

        Matrix4x4_SSE mat(tmpMat);
        Vector3D vec3(tmpVec3);

        // WHEN
        const Vector3D resultingVec = mat * vec3;

        // THEN
        QCOMPARE(resultingVec.toQVector3D(), tmpMat * tmpVec3);
    }

    void checkVector3DMatrixMultiplication()
    {

        // GIVEN
        QMatrix4x4 tmpMat;
        QVector3D tmpVec3(1.0f, 2.0f, 3.0f);
        tmpMat.translate(5.0f, 8.0f, 14.0f);

        Matrix4x4_SSE mat(tmpMat);
        Vector3D vec3(tmpVec3);

        // WHEN
        const Vector3D resultingVec = vec3 * mat;

        // THEN
        QCOMPARE(resultingVec.toQVector3D(), tmpVec3 * tmpMat);
    }

    void checkRows()
    {
        // GIVEN
        const Matrix4x4_SSE mat4(11.0f, 12.0f, 13.0f, 14.0f,
                                 21.0f, 22.0f, 23.0f, 24.0f,
                                 31.0f, 32.0f, 33.0f, 34.0f,
                                 41.0f, 42.0f, 43.0f, 44.0f);

        {
            // WHEN
            const Vector4D row = mat4.row(0);

            // THEN
            QCOMPARE(row.x(), 11.0f);
            QCOMPARE(row.y(), 12.0f);
            QCOMPARE(row.z(), 13.0f);
            QCOMPARE(row.w(), 14.0f);
        }
        {
            // WHEN
            const Vector4D row = mat4.row(1);

            // THEN
            QCOMPARE(row.x(), 21.0f);
            QCOMPARE(row.y(), 22.0f);
            QCOMPARE(row.z(), 23.0f);
            QCOMPARE(row.w(), 24.0f);
        }
        {
            // WHEN
            const Vector4D row = mat4.row(2);

            // THEN
            QCOMPARE(row.x(), 31.0f);
            QCOMPARE(row.y(), 32.0f);
            QCOMPARE(row.z(), 33.0f);
            QCOMPARE(row.w(), 34.0f);
        }
        {
            // WHEN
            const Vector4D row = mat4.row(3);

            // THEN
            QCOMPARE(row.x(), 41.0f);
            QCOMPARE(row.y(), 42.0f);
            QCOMPARE(row.z(), 43.0f);
            QCOMPARE(row.w(), 44.0f);
        }
    }

    void checkColumns()
    {
        // GIVEN
        const Matrix4x4_SSE mat4(11.0f, 12.0f, 13.0f, 14.0f,
                                 21.0f, 22.0f, 23.0f, 24.0f,
                                 31.0f, 32.0f, 33.0f, 34.0f,
                                 41.0f, 42.0f, 43.0f, 44.0f);

        {
            // WHEN
            const Vector4D row = mat4.column(0);

            // THEN
            QCOMPARE(row.x(), 11.0f);
            QCOMPARE(row.y(), 21.0f);
            QCOMPARE(row.z(), 31.0f);
            QCOMPARE(row.w(), 41.0f);
        }
        {
            // WHEN
            const Vector4D row = mat4.column(1);

            // THEN
            QCOMPARE(row.x(), 12.0f);
            QCOMPARE(row.y(), 22.0f);
            QCOMPARE(row.z(), 32.0f);
            QCOMPARE(row.w(), 42.0f);
        }
        {
            // WHEN
            const Vector4D row = mat4.column(2);

            // THEN
            QCOMPARE(row.x(), 13.0f);
            QCOMPARE(row.y(), 23.0f);
            QCOMPARE(row.z(), 33.0f);
            QCOMPARE(row.w(), 43.0f);
        }
        {
            // WHEN
            const Vector4D row = mat4.column(3);

            // THEN
            QCOMPARE(row.x(), 14.0f);
            QCOMPARE(row.y(), 24.0f);
            QCOMPARE(row.z(), 34.0f);
            QCOMPARE(row.w(), 44.0f);
        }
    }

    void checkVectorMapVector()
    {
        {
            // GIVEN
            QMatrix4x4 tmpMat;
            QVector3D tmpVec3(1.0f, 0.0f, 0.0f);
            tmpMat.rotate(90.f, 0.f, 1.f, 0.f);

            Matrix4x4_SSE mat(tmpMat);
            Vector3D vec3(tmpVec3);

            // WHEN
            const Vector3D resultingVec = mat.mapVector(vec3);

            // THEN
            QCOMPARE(resultingVec.toQVector3D(), tmpMat.mapVector(tmpVec3));
        }
        {
            // GIVEN
            QMatrix4x4 tmpMat;
            QVector3D tmpVec3(0.0f, 0.0f, -1.0f);
            tmpMat.rotate(90.f, 0.f, 1.f, 0.f);

            Matrix4x4_SSE mat(tmpMat);
            Vector3D vec3(tmpVec3);

            // WHEN
            const Vector3D resultingVec = mat.mapVector(vec3);

            // THEN
            QCOMPARE(resultingVec.toQVector3D(), tmpMat.mapVector(tmpVec3));
        }
        {
            // GIVEN
            QMatrix4x4 tmpMat;
            QVector3D tmpVec3(3.0f, -3.0f, -1.0f);
            tmpMat.rotate(90.f, 0.33f, 0.33f, 0.33f);

            Matrix4x4_SSE mat(tmpMat);
            Vector3D vec3(tmpVec3);

            // WHEN
            const Vector3D resultingVec = mat.mapVector(vec3);

            // THEN
            QCOMPARE(resultingVec.toQVector3D(), tmpMat.mapVector(tmpVec3));
        }
    }
};

QTEST_MAIN(tst_Matrix4x4_SSE)

#include "tst_matrix4x4_sse.moc"
