/****************************************************************************
**
** Copyright (C) 2017 BogDan Vatra <bogdan@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QANDROIDBINDER_H
#define QANDROIDBINDER_H

#include <QtAndroidExtras/qandroidextrasglobal.h>
#include <QSharedPointer>

#include <jni.h>

QT_BEGIN_NAMESPACE
class QAndroidBinderPrivate;
class QAndroidParcel;
class QAndroidJniObject;

class Q_ANDROIDEXTRAS_EXPORT QAndroidBinder
{
public:
    enum class CallType {
        Normal = 0,
        OneWay = 1
    };

public:
    explicit QAndroidBinder();
    QAndroidBinder(const QAndroidJniObject &binder);

    virtual ~QAndroidBinder();

    virtual bool onTransact(int code, const QAndroidParcel &data, const QAndroidParcel &reply, CallType flags);
    bool transact(int code, const QAndroidParcel &data, QAndroidParcel *reply = nullptr, CallType flags = CallType::Normal) const;

    QAndroidJniObject handle() const;

private:
    friend class QAndroidBinderPrivate;
    friend class QAndroidParcelPrivate;
    friend class QAndroidServicePrivate;
    QSharedPointer<QAndroidBinderPrivate> d;
};
QT_END_NAMESPACE

#endif // QANDROIDBINDER_H
