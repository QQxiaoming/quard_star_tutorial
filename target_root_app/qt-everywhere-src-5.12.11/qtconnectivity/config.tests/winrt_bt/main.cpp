/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtConnectivity module of the Qt Toolkit.
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

#include <wrl.h>
#include <windows.devices.enumeration.h>
#include <windows.devices.bluetooth.h>

#if defined(_WIN32) && defined(__INTEL_COMPILER)
#error "Windows ICC fails to build the WinRT backend (QTBUG-68026)."
#endif

int main()
{
    Microsoft::WRL::ComPtr<ABI::Windows::Devices::Enumeration::IDeviceInformationStatics> deviceInformationStatics;
    ABI::Windows::Foundation::GetActivationFactory(Microsoft::WRL::Wrappers::HString::MakeReference
        (RuntimeClass_Windows_Devices_Enumeration_DeviceInformation).Get(), &deviceInformationStatics);

    (void)Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::IBluetoothDevice>().Get();
    (void)Microsoft::WRL::ComPtr<ABI::Windows::Devices::Bluetooth::IBluetoothLEDevice>().Get();
    return 0;
}
