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

#ifndef QWEBGLWINDOW_P_H
#define QWEBGLWINDOW_P_H

#include <QtCore/qatomic.h>
#include <QtCore/qpointer.h>
#include <QtGui/qsurfaceformat.h>

#if (defined(Q_CC_MSVC) && _MSC_VER <= 1800)
// https://connect.microsoft.com/VisualStudio/feedback/details/811347/compiling-vc-12-0-with-has-exceptions-0-and-including-concrt-h-causes-a-compiler-error
_CRTIMP bool __cdecl __uncaught_exception();
#endif

#include <future>

QT_BEGIN_NAMESPACE

class QWebGLWindow;

class QWebGLWindowPrivate
{
public:
    QWebGLWindowPrivate(QWebGLWindow *p);

    bool raster = false;
    QSurfaceFormat format;

    enum Flag {
        Created = 0x01,
        HasNativeWindow = 0x02,
        IsFullScreen = 0x04
    };
    Q_DECLARE_FLAGS(Flags, Flag)
    Flags flags;

    std::promise<QMap<unsigned int, QVariant>> defaults;
    int id = -1;
    static QAtomicInt nextId;

private:
    Q_DECLARE_PUBLIC(QWebGLWindow)
    QWebGLWindow *q_ptr = nullptr;
};

QT_END_NAMESPACE

#endif // QWEBGLWINDOW_P_H
