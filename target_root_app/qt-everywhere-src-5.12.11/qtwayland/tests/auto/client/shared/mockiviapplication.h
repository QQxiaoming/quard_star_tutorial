/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#ifndef MOCKIVIAPPLICATION_H
#define MOCKIVIAPPLICATION_H

#include <qwayland-server-ivi-application.h>

#include <QSharedPointer>
#include <QVector>

class MockIviSurface;

namespace Impl {

class Surface;
class IviApplication;

class IviSurface : public QtWaylandServer::ivi_surface
{
public:
    IviSurface(IviApplication *iviApplication, Surface *surface, uint iviId, wl_client *client, uint32_t id);
    ~IviSurface() override;
    IviApplication *iviApplication() const { return m_iviApplication; }
    Surface *surface() const { return m_surface; }
    uint iviId() const { return m_iviId; }

    QSharedPointer<MockIviSurface> mockIviSurface() const { return m_mockIviSurface; }

protected:
    void ivi_surface_destroy_resource(Resource *) override { delete this; }
    void ivi_surface_destroy(Resource *resource) override;

private:
    Surface *m_surface = nullptr;
    IviApplication *m_iviApplication = nullptr;
    const uint m_iviId = 0;
    QSharedPointer<MockIviSurface> m_mockIviSurface;
};

class IviApplication : public QtWaylandServer::ivi_application
{
public:
    explicit IviApplication(::wl_display *display) : ivi_application(display, 1) {}
    QVector<IviSurface *> iviSurfaces() const { return m_iviSurfaces; }

protected:
    void ivi_application_surface_create(Resource *resource, uint32_t ivi_id, ::wl_resource *surface, uint32_t id) override;

private:
    void addIviSurface(IviSurface *iviSurface) { m_iviSurfaces.append(iviSurface); }
    void removeIviSurface(IviSurface *iviSurface) { m_iviSurfaces.removeOne(iviSurface); }
    QVector<IviSurface *> m_iviSurfaces;

    friend class IviSurface;
};

} // namespace Impl

#endif // MOCKIVIAPPLICATION_H
