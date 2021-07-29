/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
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

#include <QtCore/private/qeventdispatcher_winrt_p.h>

#define Q_OS_WINRT
#include <QtCore/qfunctions_winrt.h>

#include <wrl.h>
#include <windows.devices.bluetooth.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Devices::Bluetooth;
using namespace ABI::Windows::Foundation;

QT_BEGIN_NAMESPACE

#pragma warning (push)
#pragma warning (disable: 4273)
HRESULT QEventDispatcherWinRT::runOnXamlThread(const std::function<HRESULT()> &delegate, bool waitForRun)
{
    Q_UNUSED(waitForRun)
    return delegate();
}
#pragma warning (pop)

extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH: {
        // Check if we are running on a recent enough Windows
        HRESULT  hr = OleInitialize(NULL);
        if (FAILED(hr)) {
            MessageBox(NULL, (LPCWSTR)L"OleInitialize failed.", (LPCWSTR)L"Error", MB_OK | MB_ICONERROR | MB_APPLMODAL);
            exit(-1);
        }
        ComPtr<IBluetoothDeviceStatics> deviceStatics;
        hr = GetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Devices_Bluetooth_BluetoothDevice).Get(), &deviceStatics);
        if (hr == REGDB_E_CLASSNOTREG) {
            QString error ("This Windows version (" + QSysInfo::kernelVersion() + ") does not "
                "support the required Bluetooth API. Consider updating to a more recent Windows "
                "(10.0.10586 or above).");
            MessageBox(NULL, (LPCWSTR)error.constData(), (LPCWSTR)L"Error", MB_OK | MB_ICONERROR | MB_APPLMODAL);
            CoUninitialize();
            exit(-1);
        }
        break;
    }
    case DLL_PROCESS_DETACH:
        CoUninitialize();
    }

    return TRUE;
}

QT_END_NAMESPACE
