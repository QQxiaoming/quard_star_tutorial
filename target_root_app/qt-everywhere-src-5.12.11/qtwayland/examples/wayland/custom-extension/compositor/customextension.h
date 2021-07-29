/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt-project.org/legal
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

#ifndef CUSTOMEXTENSION_H
#define CUSTOMEXTENSION_H

#include "wayland-util.h"

#include <QtWaylandCompositor/QWaylandCompositorExtensionTemplate>
#include <QtWaylandCompositor/QWaylandQuickExtension>
#include <QtWaylandCompositor/QWaylandCompositor>
#include "qwayland-server-custom.h"

class CustomExtensionObject;

class CustomExtension  : public QWaylandCompositorExtensionTemplate<CustomExtension>
        , public QtWaylandServer::qt_example_extension
{
    Q_OBJECT
public:
    CustomExtension(QWaylandCompositor *compositor = nullptr);
    void initialize() override;

signals:
    void surfaceAdded(QWaylandSurface *surface);
    void bounce(QWaylandSurface *surface, uint ms);
    void spin(QWaylandSurface *surface, uint ms);

    void customObjectCreated(CustomExtensionObject *obj);

public slots:
    void setFontSize(QWaylandSurface *surface, uint pixelSize);
    void showDecorations(QWaylandClient *client, bool);
    void close(QWaylandSurface *surface);

protected:
    void example_extension_bounce(Resource *resource, wl_resource *surface, uint32_t duration) override;
    void example_extension_spin(Resource *resource, wl_resource *surface, uint32_t duration) override;
    void example_extension_register_surface(Resource *resource, wl_resource *surface) override;

    void example_extension_create_local_object(Resource *resource, uint32_t id, const QString &color, const QString &text) override;
};


class CustomExtensionObject : public QWaylandCompositorExtensionTemplate<CustomExtensionObject>
        , public QtWaylandServer::qt_example_local_object
{
    Q_OBJECT
    Q_PROPERTY(QString color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
public:
    CustomExtensionObject(const QString &color, const QString &text, struct ::wl_client *client, int id, int version);

    QString color() const
    {
        return m_color;
    }

    QString text() const
    {
        return m_text;
    }

public slots:
    void setColor(const QString &color)
    {
        if (m_color == color)
            return;

        m_color = color;
        emit colorChanged(m_color);
    }

    void setText(QString text)
    {
        if (m_text == text)
            return;

        m_text = text;
        emit textChanged(m_text);
    }
    void sendClicked();

signals:
    void colorChanged(const QString &color);
    void resourceDestroyed();

    void textChanged(QString text);

protected:
    void example_local_object_destroy_resource(Resource *resource) override;
    void example_local_object_set_text(Resource *resource, const QString &text) override;

private:
    QString m_color;
    QString m_text;
};

Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(CustomExtension)

#endif // CUSTOMEXTENSION_H
