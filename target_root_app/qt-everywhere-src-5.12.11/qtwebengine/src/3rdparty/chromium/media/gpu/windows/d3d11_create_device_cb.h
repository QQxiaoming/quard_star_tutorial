// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_GPU_WINDOWS_D3D11_CREATE_DEVICE_CB_H_
#define MEDIA_GPU_WINDOWS_D3D11_CREATE_DEVICE_CB_H_

#include <d3d11_1.h>
#include <wrl/client.h>

#include "base/callback.h"

namespace media {

// Handy callback to inject mocks via D3D11CreateDevice.
//
// The signature matches D3D11CreateDevice(). decltype(D3D11CreateDevice) does
// not work because __attribute__((stdcall)) gets appended, and the template
// instantiation fails.
using D3D11CreateDeviceCB =
    base::RepeatingCallback<HRESULT(IDXGIAdapter*,
                                    D3D_DRIVER_TYPE,
                                    HMODULE,
                                    UINT,
                                    const D3D_FEATURE_LEVEL*,
                                    UINT,
                                    UINT,
                                    ID3D11Device**,
                                    D3D_FEATURE_LEVEL*,
                                    ID3D11DeviceContext**)>;
}  // namespace media

#endif  // MEDIA_GPU_WINDOWS_D3D11_CREATE_DEVICE_CB_H_
