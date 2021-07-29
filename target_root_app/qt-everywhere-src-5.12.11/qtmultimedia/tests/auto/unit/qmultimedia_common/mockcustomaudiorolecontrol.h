/****************************************************************************
**
** Copyright (C) 2017 QNX Software Systems. All rights reserved.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
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
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef MOCKCUSTOMAUDIOROLECONTROL_H
#define MOCKCUSTOMAUDIOROLECONTROL_H

#include <qcustomaudiorolecontrol.h>

class MockCustomAudioRoleControl : public QCustomAudioRoleControl
{
    friend class MockMediaPlayerService;

public:
    MockCustomAudioRoleControl()
        : QCustomAudioRoleControl()
        , m_customAudioRole(QAudio::UnknownRole)
    {
    }

    QString customAudioRole() const
    {
        return m_customAudioRole;
    }

    void setCustomAudioRole(const QString &role)
    {
        if (role != m_customAudioRole)
            emit customAudioRoleChanged(m_customAudioRole = role);
    }

    QStringList supportedCustomAudioRoles() const
    {
        return QStringList() << QStringLiteral("customRole")
                             << QStringLiteral("customRole2");
    }

    QString m_customAudioRole;
};

#endif // MOCKCUSTOMAUDIOROLECONTROL_H
