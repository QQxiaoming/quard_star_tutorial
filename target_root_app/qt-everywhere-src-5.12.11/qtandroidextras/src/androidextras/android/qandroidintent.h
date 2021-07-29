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

#ifndef QANDROIDINTENT_H
#define QANDROIDINTENT_H

#include <QtAndroidExtras/qandroidextrasglobal.h>
#include <QtAndroidExtras/qandroidjniobject.h>

QT_BEGIN_NAMESPACE
class QAndroidJniObject;
class Q_ANDROIDEXTRAS_EXPORT QAndroidIntent
{
public:
    QAndroidIntent();
    virtual ~QAndroidIntent();
    explicit QAndroidIntent(const QAndroidJniObject &intent);
    explicit QAndroidIntent(const QString &action);
    explicit QAndroidIntent(const QAndroidJniObject &packageContext, const char *className);

    void putExtra(const QString &key, const QByteArray &data);
    QByteArray extraBytes(const QString &key);

    void putExtra(const QString &key, const QVariant &value);
    QVariant extraVariant(const QString &key);

    QAndroidJniObject handle() const;

private:
    QAndroidJniObject m_handle;
};
QT_END_NAMESPACE

#endif // QANDROIDINTENT_H
