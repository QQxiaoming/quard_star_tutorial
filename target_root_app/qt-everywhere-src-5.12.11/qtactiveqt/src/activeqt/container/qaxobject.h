/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the ActiveQt framework of the Qt Toolkit.
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

#ifndef QAXOBJECT_H
#define QAXOBJECT_H

#include <ActiveQt/qaxbase.h>

QT_BEGIN_NAMESPACE

class QAxObject : public QObject, public QAxBase
{
    friend class QAxEventSink;
    Q_OBJECT_FAKE
public:
    QObject* qObject() const override { return static_cast<QObject *>(const_cast<QAxObject *>(this)); }
    const char *className() const override;

    explicit QAxObject(QObject *parent = nullptr);
    explicit QAxObject(const QString &c, QObject *parent = nullptr);
    explicit QAxObject(IUnknown *iface, QObject *parent = nullptr);
    ~QAxObject() override;

    bool doVerb(const QString &verb);

protected:
    void connectNotify(const QMetaMethod &signal) override;
    const QMetaObject *fallbackMetaObject() const override;

private:
    const QMetaObject *parentMetaObject() const override;
};

template <> inline QAxObject *qobject_cast<QAxObject*>(const QObject *o)
{
    void *result = o ? const_cast<QObject *>(o)->qt_metacast("QAxObject") : nullptr;
    return reinterpret_cast<QAxObject*>(result);
}

template <> inline QAxObject *qobject_cast<QAxObject*>(QObject *o)
{
    void *result = o ? o->qt_metacast("QAxObject") : nullptr;
    return reinterpret_cast<QAxObject*>(result);
}

QT_END_NAMESPACE
Q_DECLARE_METATYPE(QAxObject*)

#endif // QAXOBJECT_H
