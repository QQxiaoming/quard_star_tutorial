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

#ifndef AX2_H
#define AX2_H

#include <QWidget>
#include <QPainter>

//! [0]
class QAxWidget2 : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("ClassID", "{58139D56-6BE9-4b17-937D-1B1EDEDD5B71}")
    Q_CLASSINFO("InterfaceID", "{B66280AB-08CC-4dcc-924F-58E6D7975B7D}")
    Q_CLASSINFO("EventsID", "{D72BACBA-03C4-4480-B4BB-DE4FE3AA14A0}")
    Q_CLASSINFO("ToSuperClass", "QAxWidget2")
    Q_CLASSINFO("StockEvents", "yes")
    Q_CLASSINFO("Insertable", "yes")

    Q_PROPERTY(int lineWidth READ lineWidth WRITE setLineWidth)
public:
    explicit QAxWidget2(QWidget *parent = nullptr)
        : QWidget(parent), m_lineWidth(1)
    {
    }

    int lineWidth() const
    {
        return m_lineWidth;
    }

    void setLineWidth(int lw)
    {
        m_lineWidth = lw;
        repaint();
    }

protected:
    void paintEvent(QPaintEvent *e)
    {
        QPainter paint(this);
        QPen pen = paint.pen();
        pen.setWidth(m_lineWidth);
        paint.setPen(pen);

        QRect r = rect();
        r.adjust(10, 10, -10, -10);
        paint.drawEllipse(r);
    }

private:
    int m_lineWidth;
};
//! [0]

#endif // AX2_H
