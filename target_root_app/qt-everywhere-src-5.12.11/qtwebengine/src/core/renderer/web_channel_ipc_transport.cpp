/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

#include "renderer/web_channel_ipc_transport.h"

#include "common/qt_messages.h"

#include "content/public/renderer/render_frame.h"
#include "gin/arguments.h"
#include "gin/handle.h"
#include "gin/object_template_builder.h"
#include "gin/wrappable.h"
#include "third_party/blink/public/web/blink.h"
#include "third_party/blink/public/web/web_local_frame.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_provider.h"
#include "third_party/blink/public/common/associated_interfaces/associated_interface_registry.h"
#include "v8/include/v8.h"
#include "services/service_manager/public/cpp/interface_provider.h"
#include "qtwebengine/browser/qtwebchannel.mojom.h"

#include <QJsonDocument>

namespace QtWebEngineCore {

class WebChannelTransport : public gin::Wrappable<WebChannelTransport> {
public:
    static gin::WrapperInfo kWrapperInfo;
    static void Install(blink::WebLocalFrame *frame, uint worldId);
    static void Uninstall(blink::WebLocalFrame *frame, uint worldId);
private:
    WebChannelTransport() {}
    void NativeQtSendMessage(gin::Arguments *args);

    // gin::WrappableBase
    gin::ObjectTemplateBuilder GetObjectTemplateBuilder(v8::Isolate *isolate) override;

    DISALLOW_COPY_AND_ASSIGN(WebChannelTransport);
};

gin::WrapperInfo WebChannelTransport::kWrapperInfo = { gin::kEmbedderNativeGin };

void WebChannelTransport::Install(blink::WebLocalFrame *frame, uint worldId)
{
    v8::Isolate *isolate = blink::MainThreadIsolate();
    v8::HandleScope handleScope(isolate);
    v8::Local<v8::Context> context;
    if (worldId == 0)
        context = frame->MainWorldScriptContext();
    else
        context = frame->IsolatedWorldScriptContext(worldId);
    v8::Context::Scope contextScope(context);

    gin::Handle<WebChannelTransport> transport = gin::CreateHandle(isolate, new WebChannelTransport);

    v8::Local<v8::Object> global = context->Global();
    v8::Local<v8::Value> qtObjectValue = global->Get(gin::StringToV8(isolate, "qt"));
    v8::Local<v8::Object> qtObject;
    if (qtObjectValue.IsEmpty() || !qtObjectValue->IsObject()) {
        qtObject = v8::Object::New(isolate);
        global->Set(gin::StringToV8(isolate, "qt"), qtObject);
    } else {
        qtObject = v8::Local<v8::Object>::Cast(qtObjectValue);
    }
    qtObject->Set(gin::StringToV8(isolate, "webChannelTransport"), transport.ToV8());
}

void WebChannelTransport::Uninstall(blink::WebLocalFrame *frame, uint worldId)
{
    v8::Isolate *isolate = blink::MainThreadIsolate();
    v8::HandleScope handleScope(isolate);
    v8::Local<v8::Context> context;
    if (worldId == 0)
        context = frame->MainWorldScriptContext();
    else
        context = frame->IsolatedWorldScriptContext(worldId);
    v8::Context::Scope contextScope(context);

    v8::Local<v8::Object> global(context->Global());
    v8::Local<v8::Value> qtObjectValue = global->Get(gin::StringToV8(isolate, "qt"));
    if (qtObjectValue.IsEmpty() || !qtObjectValue->IsObject())
        return;
    v8::Local<v8::Object> qtObject = v8::Local<v8::Object>::Cast(qtObjectValue);
    qtObject->Delete(gin::StringToV8(isolate, "webChannelTransport"));
}

void WebChannelTransport::NativeQtSendMessage(gin::Arguments *args)
{
    blink::WebLocalFrame *frame = blink::WebLocalFrame::FrameForCurrentContext();
    if (!frame || !frame->View())
        return;

    content::RenderFrame *renderFrame = content::RenderFrame::FromWebFrame(frame);
    if (!renderFrame)
        return;

    v8::Local<v8::Value> jsonValue;
    if (!args->GetNext(&jsonValue)) {
        args->ThrowTypeError("Missing argument");
        return;
    }

    if (!jsonValue->IsString()) {
        args->ThrowTypeError("Expected string");
        return;
    }
    v8::Local<v8::String> jsonString = v8::Local<v8::String>::Cast(jsonValue);

    QByteArray json(jsonString->Utf8Length(), 0);
    jsonString->WriteUtf8(json.data(), json.size(),
                         nullptr,
                         v8::String::REPLACE_INVALID_UTF8);

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json, &error);
    if (error.error != QJsonParseError::NoError) {
        args->ThrowTypeError("Invalid JSON");
        return;
    }

    int size = 0;
    const char *rawData = doc.rawData(&size);
    qtwebchannel::mojom::WebChannelTransportHostAssociatedPtr webChannelTransport;
    renderFrame->GetRemoteAssociatedInterfaces()->GetInterface(&webChannelTransport);
    webChannelTransport->DispatchWebChannelMessage(std::vector<uint8_t>(rawData, rawData + size));
}

gin::ObjectTemplateBuilder WebChannelTransport::GetObjectTemplateBuilder(v8::Isolate *isolate)
{
    return gin::Wrappable<WebChannelTransport>::GetObjectTemplateBuilder(isolate)
        .SetMethod("send", &WebChannelTransport::NativeQtSendMessage);
}

WebChannelIPCTransport::WebChannelIPCTransport(content::RenderFrame *renderFrame)
    : content::RenderFrameObserver(renderFrame)
    , m_worldId(0)
    , m_worldInitialized(false)
{
    renderFrame->GetAssociatedInterfaceRegistry()->AddInterface(
                base::Bind(&WebChannelIPCTransport::BindRequest, base::Unretained(this)));
}

void WebChannelIPCTransport::BindRequest(
        qtwebchannel::mojom::WebChannelTransportRenderAssociatedRequest request) {

    m_binding.AddBinding(this, std::move(request));
}

void WebChannelIPCTransport::SetWorldId(uint32_t worldId)
{
    if (m_worldInitialized && m_worldId == worldId)
        return;

    if (m_worldInitialized && m_canUseContext)
        WebChannelTransport::Uninstall(render_frame()->GetWebFrame(), m_worldId);

    m_worldInitialized = true;
    m_worldId = worldId;

    if (m_canUseContext)
        WebChannelTransport::Install(render_frame()->GetWebFrame(), m_worldId);
}

void WebChannelIPCTransport::ResetWorldId()
{
   if (m_worldInitialized && m_canUseContext)
        WebChannelTransport::Uninstall(render_frame()->GetWebFrame(), m_worldId);

    m_worldInitialized = false;
    m_worldId = 0;
}

void WebChannelIPCTransport::DispatchWebChannelMessage(const std::vector<uint8_t> &binaryJson, uint32_t worldId)
{
    DCHECK(m_canUseContext);
    DCHECK(m_worldId == worldId);

    QJsonDocument doc = QJsonDocument::fromRawData(reinterpret_cast<const char *>(binaryJson.data()),
                                                   binaryJson.size(), QJsonDocument::BypassValidation);
    DCHECK(doc.isObject());
    QByteArray json = doc.toJson(QJsonDocument::Compact);

    blink::WebLocalFrame *frame = render_frame()->GetWebFrame();
    v8::Isolate *isolate = blink::MainThreadIsolate();
    v8::HandleScope handleScope(isolate);
    v8::Local<v8::Context> context;
    if (worldId == 0)
        context = frame->MainWorldScriptContext();
    else
        context = frame->IsolatedWorldScriptContext(worldId);
    v8::Context::Scope contextScope(context);

    v8::Local<v8::Object> global(context->Global());
    v8::Local<v8::Value> qtObjectValue(global->Get(gin::StringToV8(isolate, "qt")));
    if (qtObjectValue.IsEmpty() || !qtObjectValue->IsObject())
        return;
    v8::Local<v8::Object> qtObject = v8::Local<v8::Object>::Cast(qtObjectValue);
    v8::Local<v8::Value> webChannelObjectValue(qtObject->Get(gin::StringToV8(isolate, "webChannelTransport")));
    if (webChannelObjectValue.IsEmpty() || !webChannelObjectValue->IsObject())
        return;
    v8::Local<v8::Object> webChannelObject = v8::Local<v8::Object>::Cast(webChannelObjectValue);
    v8::Local<v8::Value> callbackValue(webChannelObject->Get(gin::StringToV8(isolate, "onmessage")));
    if (callbackValue.IsEmpty() || !callbackValue->IsFunction()) {
        LOG(WARNING) << "onmessage is not a callable property of qt.webChannelTransport. Some things might not work as expected.";
        return;
    }

    v8::Local<v8::Object> messageObject(v8::Object::New(isolate));
    v8::Maybe<bool> wasSet = messageObject->DefineOwnProperty(
                context,
                v8::String::NewFromUtf8(isolate, "data"),
                v8::String::NewFromUtf8(isolate, json.constData(), v8::String::kNormalString, json.size()),
                v8::PropertyAttribute(v8::ReadOnly | v8::DontDelete));
    DCHECK(!wasSet.IsNothing() && wasSet.FromJust());

    v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(callbackValue);
    v8::Local<v8::Value> argv[] = { messageObject };
    frame->CallFunctionEvenIfScriptDisabled(callback, webChannelObject, 1, argv);
}

void WebChannelIPCTransport::WillReleaseScriptContext(v8::Local<v8::Context> context, int worldId)
{
    if (static_cast<uint>(worldId) == m_worldId)
        m_canUseContext = false;
}

void WebChannelIPCTransport::DidClearWindowObject()
{
    if (!m_canUseContext) {
        m_canUseContext = true;
        if (m_worldInitialized)
            WebChannelTransport::Install(render_frame()->GetWebFrame(), m_worldId);
    }
}

void WebChannelIPCTransport::OnDestruct()
{
    delete this;
}

} // namespace QtWebEngineCore
