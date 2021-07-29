/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#ifndef MOCKOUTPUT_H
#define MOCKOUTPUT_H

#include <qglobal.h>

#include "qwayland-server-wayland.h"

#include "mockcompositor.h"

namespace Impl {

class Output : public QtWaylandServer::wl_output
{
public:
    Output(::wl_display *display, const QSize &resolution, const QPoint &position);

    QSharedPointer<MockOutput> mockOutput() const { return m_mockOutput; }
    void setCurrentMode(const QSize &size);
    void sendGeometryAndMode(const QRect &geometry);

protected:
    void output_bind_resource(Resource *resource) override;

private:
    void sendGeometry(Resource *resource);
    void sendCurrentMode(Resource *resource);
    QSize m_size;
    QPoint m_position;
    const QSize m_physicalSize;
    QSharedPointer<MockOutput> m_mockOutput;
};

}

#endif // MOCKOUTPUT_H
