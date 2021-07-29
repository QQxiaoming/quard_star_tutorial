/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
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

/****************************************************************************
**
** This is a simple QGLWidget displaying an openGL wireframe box
**
****************************************************************************/

#ifndef GLBOX_H
#define GLBOX_H

#include <QtOpenGL>
#include <QOpenGLFunctions_1_1>
//! [0]
#include <QAxBindable>

class GLBox : public QGLWidget,
              public QOpenGLFunctions_1_1,
              public QAxBindable
{
    Q_OBJECT
    Q_CLASSINFO("ClassID",     "{5fd9c22e-ed45-43fa-ba13-1530bb6b03e0}")
    Q_CLASSINFO("InterfaceID", "{33b051af-bb25-47cf-a390-5cfd2987d26a}")
    Q_CLASSINFO("EventsID",    "{8c996c29-eafa-46ac-a6f9-901951e765b5}")
    //! [0] //! [1]

public:
    explicit GLBox(QWidget *parent, const char *name = nullptr);
    virtual ~GLBox();
    QAxAggregated *createAggregate();

public slots:
    void                setXRotation(int degrees);
//! [1]
    void                setYRotation(int degrees);
    void                setZRotation(int degrees);

protected:
    void                initializeGL();
    void                paintGL();
    void                resizeGL(int w, int h);
    virtual GLuint      makeObject();

private:
    GLuint  m_object;
    GLfloat m_xRot;
    GLfloat m_yRot;
    GLfloat m_zRot;
    GLfloat m_scale;
};

#endif // GLBOX_H
