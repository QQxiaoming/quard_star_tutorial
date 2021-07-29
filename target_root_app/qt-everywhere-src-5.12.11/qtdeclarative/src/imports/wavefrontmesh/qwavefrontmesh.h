/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
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

#ifndef QWAVEFRONTMESH_H
#define QWAVEFRONTMESH_H

#include <QtQuick/private/qquickshadereffectmesh_p.h>

#include <QtCore/qurl.h>
#include <QtGui/qvector3d.h>

QT_BEGIN_NAMESPACE

class QWavefrontMeshPrivate;
class QWavefrontMesh : public QQuickShaderEffectMesh
{
    Q_OBJECT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(Error lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(QVector3D projectionPlaneV READ projectionPlaneV WRITE setProjectionPlaneV NOTIFY projectionPlaneVChanged)
    Q_PROPERTY(QVector3D projectionPlaneW READ projectionPlaneW WRITE setProjectionPlaneW NOTIFY projectionPlaneWChanged)
public:
    enum Error {
        NoError,
        InvalidSourceError,
        UnsupportedFaceShapeError,
        UnsupportedIndexSizeError,
        FileNotFoundError,
        NoAttributesError,
        MissingPositionAttributeError,
        MissingTextureCoordinateAttributeError,
        MissingPositionAndTextureCoordinateAttributesError,
        TooManyAttributesError,
        InvalidPlaneDefinitionError
    };
    Q_ENUMS(Error)

    QWavefrontMesh(QObject *parent = nullptr);
    ~QWavefrontMesh() override;

    QUrl source() const;
    void setSource(const QUrl &url);

    Error lastError() const;
    void setLastError(Error lastError);

    bool validateAttributes(const QVector<QByteArray> &attributes, int *posIndex) override;
    QSGGeometry *updateGeometry(QSGGeometry *geometry, int attrCount, int posIndex,
                                const QRectF &srcRect, const QRectF &rect) override;
    QString log() const override;

    QVector3D projectionPlaneV() const;
    void setProjectionPlaneV(const QVector3D &projectionPlaneV);

    QVector3D projectionPlaneW() const;
    void setProjectionPlaneW(const QVector3D &projectionPlaneW);

Q_SIGNALS:
    void sourceChanged();
    void lastErrorChanged();
    void projectionPlaneVChanged();
    void projectionPlaneWChanged();

protected Q_SLOTS:
    void readData();

private:
    Q_DISABLE_COPY(QWavefrontMesh)
    Q_DECLARE_PRIVATE(QWavefrontMesh)
};

QT_END_NAMESPACE

#endif // QWAVEFRONTGEOMETRYMODEL_H
