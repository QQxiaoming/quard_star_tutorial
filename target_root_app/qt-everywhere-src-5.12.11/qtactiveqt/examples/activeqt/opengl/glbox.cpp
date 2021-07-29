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
** The OpenGL code is mostly borrowed from Brian Pauls "spin" example
** in the Mesa distribution
**
****************************************************************************/

#include "glbox.h"
#include <QAxAggregated>
#include <QUuid>
//! [0]
#include <objsafe.h>
//! [0]

#if defined(Q_CC_MSVC)
#pragma warning(disable:4305) // init: truncation from const double to float
#endif

/*!
  Create a GLBox widget
*/

GLBox::GLBox(QWidget *parent, const char *name)
    : QGLWidget(parent)
{
    m_xRot = m_yRot = m_zRot = 0.0;       // default object rotation
    m_scale = 1.25;                       // default object scale
    m_object = 0;
}


/*!
  Release allocated resources
*/

GLBox::~GLBox()
{
    makeCurrent();

    if (m_object)
        glDeleteLists(m_object, 1);
}


/*!
  Paint the box. The actual openGL commands for drawing the box are
  performed here.
*/

void GLBox::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glLoadIdentity();
    glTranslatef(0.0, 0.0, -10.0);
    glScalef(m_scale, m_scale, m_scale);

    glRotatef(m_xRot, 1.0, 0.0, 0.0);
    glRotatef(m_yRot, 0.0, 1.0, 0.0);
    glRotatef(m_zRot, 0.0, 0.0, 1.0);

    glCallList(m_object);
}


/*!
  Set up the OpenGL rendering state, and define display list
*/

void GLBox::initializeGL()
{
    initializeOpenGLFunctions();

    qglClearColor(Qt::black);           // Let OpenGL clear to black
    m_object = makeObject();            // Generate an OpenGL display list
    glShadeModel(GL_FLAT);
}



/*!
  Set up the OpenGL view port, matrix mode, etc.
*/

void GLBox::resizeGL(int w, int h)
{
    glViewport(0, 0, (GLint)w, (GLint)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -1.0, 1.0, 5.0, 15.0);
    glMatrixMode(GL_MODELVIEW);
}


/*!
  Generate an OpenGL display list for the object to be shown, i.e. the box
*/

GLuint GLBox::makeObject()
{
    GLuint list;

    list = glGenLists(1);

    glNewList(list, GL_COMPILE);

    qglColor(Qt::white);                      // Shorthand for glColor3f or glIndex

    glLineWidth(2.0);

    glBegin(GL_LINE_LOOP);
    glVertex3f( 1.0,  0.5, -0.4);
    glVertex3f( 1.0, -0.5, -0.4);
    glVertex3f(-1.0, -0.5, -0.4);
    glVertex3f(-1.0,  0.5, -0.4);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex3f( 1.0,  0.5, 0.4);
    glVertex3f( 1.0, -0.5, 0.4);
    glVertex3f(-1.0, -0.5, 0.4);
    glVertex3f(-1.0,  0.5, 0.4);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f( 1.0,  0.5, -0.4);   glVertex3f( 1.0,  0.5, 0.4);
    glVertex3f( 1.0, -0.5, -0.4);   glVertex3f( 1.0, -0.5, 0.4);
    glVertex3f(-1.0, -0.5, -0.4);   glVertex3f(-1.0, -0.5, 0.4);
    glVertex3f(-1.0,  0.5, -0.4);   glVertex3f(-1.0,  0.5, 0.4);
    glEnd();

    glEndList();

    return list;
}


/*!
  Set the rotation angle of the object to \e degrees around the X axis.
*/

void GLBox::setXRotation(int degrees)
{
    m_xRot = (GLfloat)(degrees % 360);
    updateGL();
}


/*!
  Set the rotation angle of the object to \e degrees around the Y axis.
*/

void GLBox::setYRotation(int degrees)
{
    m_yRot = (GLfloat)(degrees % 360);
    updateGL();
}


/*!
  Set the rotation angle of the object to \e degrees around the Z axis.
*/

void GLBox::setZRotation(int degrees)
{
    m_zRot = (GLfloat)(degrees % 360);
    updateGL();
}

//! [1]
class ObjectSafetyImpl : public QAxAggregated,
                         public IObjectSafety
{
public:
//! [1] //! [2]
    explicit ObjectSafetyImpl() {}

    long queryInterface(const QUuid &iid, void **iface)
    {
        *iface = nullptr;
        if (iid == IID_IObjectSafety)
            *iface = (IObjectSafety*)this;
        else
            return E_NOINTERFACE;

        AddRef();
        return S_OK;
    }

//! [2] //! [3]
    QAXAGG_IUNKNOWN;

//! [3] //! [4]
    HRESULT WINAPI GetInterfaceSafetyOptions(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions)
    {
        *pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_DATA | INTERFACESAFE_FOR_UNTRUSTED_CALLER;
        *pdwEnabledOptions = INTERFACESAFE_FOR_UNTRUSTED_DATA | INTERFACESAFE_FOR_UNTRUSTED_CALLER;
        return S_OK;
    }

    HRESULT WINAPI SetInterfaceSafetyOptions(REFIID riid, DWORD pdwSupportedOptions, DWORD pdwEnabledOptions)
    {
        return S_OK;
    }
};
//! [4] //! [5]

QAxAggregated *GLBox::createAggregate()
{
    return new ObjectSafetyImpl();
}
//! [5]
