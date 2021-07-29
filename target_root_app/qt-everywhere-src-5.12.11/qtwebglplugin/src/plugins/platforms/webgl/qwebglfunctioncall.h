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

#ifndef QWEBGLFUNCTIONCALL_H
#define QWEBGLFUNCTIONCALL_H

#include <QtCore/qcoreevent.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qvariant.h>

#include <tuple>

QT_BEGIN_NAMESPACE

class QByteArray;
class QPlatformSurface;
class QString;
class QThread;
class QWebGLFunctionCallPrivate;

class QWebGLFunctionCall : public QEvent
{
public:
    QWebGLFunctionCall(const QString &functionName, QPlatformSurface *surface, bool wait = false);
    ~QWebGLFunctionCall() override;

    static Type type();

    int id() const;
    QThread *thread() const;
    bool isBlocking() const;
    QPlatformSurface *surface() const;

    QString functionName() const;

    void addString(const QString &value);
    void addInt(int value);
    void addUInt(uint value);
    void addFloat(float value);
    void addData(const QByteArray &data);
    void addList(const QVariantList &list);
    void addNull();

    void add(const QString &value) { addString(value); }
    void add(const char *value) { addString(QString::fromLatin1(value)); }
    void add(int value) { addInt(value); }
    void add(uint value) { addUInt(value); }
    void add(float value) { addFloat(value); }
    void add(const QByteArray &data) { addData(data); }
    void add(const QVariantList &list) { addList(list); }
    void add(std::nullptr_t) { addNull(); }

    template<class...Ts>
    void addParameters(Ts&&... arguments)
    {
        addImpl(arguments...);
    }

    QVariantList parameters() const;

protected:
    template<typename T>
    void addImpl(T first)
    {
        add(first);
    }

    template<typename T, typename... Ts>
    void addImpl(T first, Ts... rest)
    {
        add(first);
        addImpl(rest...);
    }

private:
    Q_DISABLE_COPY(QWebGLFunctionCall)
    Q_DECLARE_PRIVATE(QWebGLFunctionCall)
    QScopedPointer<QWebGLFunctionCallPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QWEBGLFUNCTIONCALL_H
