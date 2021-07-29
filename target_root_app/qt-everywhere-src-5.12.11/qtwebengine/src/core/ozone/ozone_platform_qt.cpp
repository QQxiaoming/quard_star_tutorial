/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "ozone/ozone_platform_qt.h"

#if defined(USE_OZONE)
#include "ozone/surface_factory_qt.h"
#include "ozone/platform_window_qt.h"
#include "ui/display/types/native_display_delegate.h"
#include "ui/ozone/common/stub_client_native_pixmap_factory.h"
#include "ui/ozone/common/stub_overlay_manager.h"
#include "ui/ozone/public/cursor_factory_ozone.h"
#include "ui/ozone/public/gpu_platform_support_host.h"
#include "ui/ozone/public/input_controller.h"
#include "ui/ozone/public/ozone_platform.h"
#include "ui/platform_window/platform_window_delegate.h"
#include "ui/platform_window/platform_window_init_properties.h"
#include "ui/platform_window/platform_window.h"

namespace ui {

namespace {

class OzonePlatformQt : public OzonePlatform {
public:
    OzonePlatformQt();
    ~OzonePlatformQt() override;

    ui::SurfaceFactoryOzone* GetSurfaceFactoryOzone() override;
    ui::CursorFactoryOzone* GetCursorFactoryOzone() override;
    GpuPlatformSupportHost* GetGpuPlatformSupportHost() override;
    std::unique_ptr<PlatformWindow> CreatePlatformWindow(PlatformWindowDelegate* delegate, PlatformWindowInitProperties properties) override;
    std::unique_ptr<display::NativeDisplayDelegate> CreateNativeDisplayDelegate() override;
    ui::InputController* GetInputController() override;
    std::unique_ptr<ui::SystemInputInjector> CreateSystemInputInjector() override;
    ui::OverlayManagerOzone* GetOverlayManager() override;

private:
    void InitializeUI(const ui::OzonePlatform::InitParams &) override;
    void InitializeGPU(const ui::OzonePlatform::InitParams &) override;

    std::unique_ptr<QtWebEngineCore::SurfaceFactoryQt> surface_factory_ozone_;
    std::unique_ptr<CursorFactoryOzone> cursor_factory_ozone_;

    std::unique_ptr<GpuPlatformSupportHost> gpu_platform_support_host_;
    std::unique_ptr<InputController> input_controller_;
    std::unique_ptr<OverlayManagerOzone> overlay_manager_;

    DISALLOW_COPY_AND_ASSIGN(OzonePlatformQt);
};


OzonePlatformQt::OzonePlatformQt() {}

OzonePlatformQt::~OzonePlatformQt() {}

ui::SurfaceFactoryOzone* OzonePlatformQt::GetSurfaceFactoryOzone()
{
    return surface_factory_ozone_.get();
}

ui::CursorFactoryOzone* OzonePlatformQt::GetCursorFactoryOzone()
{
    return cursor_factory_ozone_.get();
}

GpuPlatformSupportHost* OzonePlatformQt::GetGpuPlatformSupportHost()
{
    return gpu_platform_support_host_.get();
}

std::unique_ptr<PlatformWindow> OzonePlatformQt::CreatePlatformWindow(PlatformWindowDelegate* delegate, PlatformWindowInitProperties properties)
{
    return base::WrapUnique(new PlatformWindowQt(delegate, properties.bounds));
}

ui::InputController* OzonePlatformQt::GetInputController()
{
    return input_controller_.get();
}

std::unique_ptr<ui::SystemInputInjector> OzonePlatformQt::CreateSystemInputInjector()
{
    return nullptr;  // no input injection support.
}

ui::OverlayManagerOzone* OzonePlatformQt::GetOverlayManager()
{
    return overlay_manager_.get();
}

std::unique_ptr<display::NativeDisplayDelegate> OzonePlatformQt::CreateNativeDisplayDelegate()
{
    NOTREACHED();
    return nullptr;
}

void OzonePlatformQt::InitializeUI(const ui::OzonePlatform::InitParams &)
{
    overlay_manager_.reset(new StubOverlayManager());
    cursor_factory_ozone_.reset(new CursorFactoryOzone());
    gpu_platform_support_host_.reset(ui::CreateStubGpuPlatformSupportHost());
    input_controller_ = CreateStubInputController();
}

void OzonePlatformQt::InitializeGPU(const ui::OzonePlatform::InitParams &)
{
    surface_factory_ozone_.reset(new QtWebEngineCore::SurfaceFactoryQt());
}

} // namespace

OzonePlatform* CreateOzonePlatformQt() { return new OzonePlatformQt; }

gfx::ClientNativePixmapFactory* CreateClientNativePixmapFactoryQt()
{
    return CreateStubClientNativePixmapFactory();
}

}  // namespace ui

#endif

