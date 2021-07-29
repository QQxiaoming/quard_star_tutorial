/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt WebGL module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
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
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWEBGLSCREEN_H
#define QWEBGLSCREEN_H

#include <QtCore/qscopedpointer.h>
#include <QtGui/qpa/qplatformscreen.h>

QT_BEGIN_NAMESPACE

class QWebGLWindow;
class QWebGLScreenPrivate;

class QWebGLScreen : public QPlatformScreen
{
public:
    QWebGLScreen();
    QWebGLScreen(const QSize size, const QSizeF physicalSize);
    ~QWebGLScreen();

    QRect geometry() const override;
    int depth() const override;
    QImage::Format format() const override;

    QSizeF physicalSize() const override;
    QDpi logicalDpi() const override;
    qreal pixelDensity() const override;

    qreal refreshRate() const override;

    void setGeometry(int width, int height, const int physicalWidth, const int physicalHeight);

private:
    friend class QWebGLWindow;

    Q_DISABLE_COPY(QWebGLScreen)
    Q_DECLARE_PRIVATE(QWebGLScreen)
    QScopedPointer<QWebGLScreenPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QWEBGLSCREEN_H
