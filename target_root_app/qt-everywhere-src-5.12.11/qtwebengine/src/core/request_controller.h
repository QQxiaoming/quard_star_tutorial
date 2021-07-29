/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
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
#ifndef REQUEST_CONTROLLER_H
#define REQUEST_CONTROLLER_H

#include "qtwebenginecoreglobal.h"

#include <QUrl>

namespace QtWebEngineCore {

class RequestController {
public:
    RequestController(QUrl origin)
        : m_answered(false)
        , m_origin(std::move(origin))
    {}

    QUrl origin() const { return m_origin; }

    void accept()
    {
        if (!m_answered) {
            m_answered = true;
            accepted();
        }
    }

    void reject()
    {
        if (!m_answered) {
            m_answered = true;
            rejected();
        }
    }

    virtual ~RequestController() {}

protected:
    virtual void accepted() = 0;
    virtual void rejected() = 0;

private:
    bool m_answered;
    QUrl m_origin;
};

} // namespace QtWebEngineCore

#endif // !REQUEST_CONTROLLER_H
