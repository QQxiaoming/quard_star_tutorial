/****************************************************************************
 **
 ** Copyright (C) 2017 The Qt Company Ltd.
 ** Contact: https://www.qt.io/licensing/
 **
 ** This file is part of the examples of the Qt Wayland module
 **
 ** $QT_BEGIN_LICENSE:BSD$
 ** Commercial License Usage
 ** Licensees holding valid commercial Qt licenses may use this file in
 ** accordance with the commercial license agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in
 ** a written agreement between you and The Qt Company. For licensing terms
 ** and conditions see https://www.qt.io/terms-conditions. For further
 ** information use the contact form at https://www.qt.io/contact-us.
 **
 ** BSD License Usage
 ** Alternatively, you may use this file under the terms of the BSD license
 ** as follows:
 **
 ** "Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions are
 ** met:
 **   * Redistributions of source code must retain the above copyright
 **     notice, this list of conditions and the following disclaimer.
 **   * Redistributions in binary form must reproduce the above copyright
 **     notice, this list of conditions and the following disclaimer in
 **     the documentation and/or other materials provided with the
 **     distribution.
 **   * Neither the name of The Qt Company Ltd nor the names of its
 **     contributors may be used to endorse or promote products derived
 **     from this software without specific prior written permission.
 **
 **
 ** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 ** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 ** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 ** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 ** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 ** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 ** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 ** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 ** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

#include <QGuiApplication>
#include <QtGui/private/qguiapplication_p.h>
#include <QOpenGLWindow>
#include <QOpenGLTexture>
#include <QOpenGLTextureBlitter>
#include <QPainter>
#include <QMouseEvent>
#include <QPlatformSurfaceEvent>

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandintegration_p.h>
#include <QtWaylandClient/private/qwaylandserverbufferintegration_p.h>
#include "sharebufferextension.h"

#include <QDebug>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QTimer>
#include <QMap>

class TestWindow : public QOpenGLWindow
{
    Q_OBJECT

public:
    TestWindow()
    {
        m_extension = new ShareBufferExtension;
        connect(m_extension, SIGNAL(bufferReceived(QtWaylandClient::QWaylandServerBuffer*)), this, SLOT(receiveBuffer(QtWaylandClient::QWaylandServerBuffer*)));
    }

public slots:
    void receiveBuffer(QtWaylandClient::QWaylandServerBuffer *buffer)
    {
        m_buffers.append(buffer);
        update();
    }

protected:

    void initializeGL() override
    {
        m_blitter = new QOpenGLTextureBlitter;
        m_blitter->create();
    }

    void paintGL() override {
        glClearColor(.5, .45, .42, 1.);
        glClear(GL_COLOR_BUFFER_BIT);
        int x = 0;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (auto buffer: m_buffers) {
            m_blitter->bind();
            QPointF pos(x,0);
            QSize s(buffer->size());
            QRectF targetRect(pos, s);
            QOpenGLTexture *texture = buffer->toOpenGlTexture();
            auto surfaceOrigin = QOpenGLTextureBlitter::OriginTopLeft;
            QMatrix4x4 targetTransform = QOpenGLTextureBlitter::targetTransform(targetRect, QRect(QPoint(), size()));
            m_blitter->blit(texture->textureId(), targetTransform, surfaceOrigin);
            m_blitter->release();
            x += s.width() + 10;
        }
    }

private:

    QOpenGLTextureBlitter *m_blitter = nullptr;
    ShareBufferExtension *m_extension = nullptr;
    QList<QtWaylandClient::QWaylandServerBuffer*> m_buffers;

};

int main (int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    TestWindow window;
    window.show();

    return app.exec();
}

#include "main.moc"
