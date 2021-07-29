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

#include "qwebglscreen.h"

#include "qwebglwindow.h"

#include <QtCore/qtextstream.h>
#include <QtGui/qpa/qwindowsysteminterface.h>
#include <QtGui/qpa/qplatformcursor.h>
#include <QtGui/qwindow.h>

QT_BEGIN_NAMESPACE

class QWebGLScreenPrivate
{
public:
    QSize size = QSize(1920, 1080);
    QSizeF physicalSize = QSizeF(531.3, 298.9);
};

QWebGLScreen::QWebGLScreen() :
    d_ptr(new QWebGLScreenPrivate)
{}

QWebGLScreen::QWebGLScreen(const QSize size, const QSizeF physicalSize) :
    QWebGLScreen()
{
    Q_D(QWebGLScreen);
    d->size = size;
    d->physicalSize = physicalSize;
}

QWebGLScreen::~QWebGLScreen()
{}

QRect QWebGLScreen::geometry() const
{
    Q_D(const QWebGLScreen);
    return QRect(QPoint(0, 0), d->size);
}

int QWebGLScreen::depth() const
{
    return 32;
}

QImage::Format QWebGLScreen::format() const
{
    return QImage::Format_ARGB32;
}

QSizeF QWebGLScreen::physicalSize() const
{
    Q_D(const QWebGLScreen);
    return d->physicalSize;
}

QDpi QWebGLScreen::logicalDpi() const
{
    return QPlatformScreen::logicalDpi();
}

qreal QWebGLScreen::pixelDensity() const
{
    return QPlatformScreen::pixelDensity();
}

qreal QWebGLScreen::refreshRate() const
{
    return 60;
}

void QWebGLScreen::setGeometry(int width, int height, const int physicalWidth,
                               const int physicalHeight)
{
    Q_D(QWebGLScreen);
    d->size = QSize(width, height);
    d->physicalSize = QSize(physicalWidth, physicalHeight);
    resizeMaximizedWindows();
}

QT_END_NAMESPACE
