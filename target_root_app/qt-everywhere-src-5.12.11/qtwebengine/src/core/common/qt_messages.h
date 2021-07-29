// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.Chromium file.

// Multiply-included file, no traditional include guard.

#include "base/optional.h"
#include "content/public/common/common_param_traits.h"
#include "content/public/common/webplugininfo.h"
#include "ipc/ipc_message_macros.h"
#include "ppapi/buildflags/buildflags.h"
#include "user_script_data.h"

IPC_STRUCT_TRAITS_BEGIN(UserScriptData)
    IPC_STRUCT_TRAITS_MEMBER(source)
    IPC_STRUCT_TRAITS_MEMBER(url)
    IPC_STRUCT_TRAITS_MEMBER(injectionPoint)
    IPC_STRUCT_TRAITS_MEMBER(injectForSubframes)
    IPC_STRUCT_TRAITS_MEMBER(worldId)
    IPC_STRUCT_TRAITS_MEMBER(scriptId)
    IPC_STRUCT_TRAITS_MEMBER(globs)
    IPC_STRUCT_TRAITS_MEMBER(excludeGlobs)
    IPC_STRUCT_TRAITS_MEMBER(urlPatterns)
IPC_STRUCT_TRAITS_END()


#define IPC_MESSAGE_START QtMsgStart

//-----------------------------------------------------------------------------
// RenderView messages
// These are messages sent from the browser to the renderer process.

IPC_MESSAGE_ROUTED1(RenderViewObserverQt_FetchDocumentMarkup,
                    uint64_t /* requestId */)

IPC_MESSAGE_ROUTED1(RenderViewObserverQt_FetchDocumentInnerText,
                    uint64_t /* requestId */)

// User scripts messages
IPC_MESSAGE_ROUTED1(RenderFrameObserverHelper_AddScript,
                    UserScriptData /* script */)
IPC_MESSAGE_ROUTED1(RenderFrameObserverHelper_RemoveScript,
                    UserScriptData /* script */)
IPC_MESSAGE_ROUTED0(RenderFrameObserverHelper_ClearScripts)

IPC_MESSAGE_CONTROL1(UserResourceController_AddScript, UserScriptData /* scriptContents */)
IPC_MESSAGE_CONTROL1(UserResourceController_RemoveScript, UserScriptData /* scriptContents */)
IPC_MESSAGE_CONTROL0(UserResourceController_ClearScripts)

// Tells the renderer whether or not a file system access has been allowed.
IPC_MESSAGE_ROUTED2(QtWebEngineMsg_RequestFileSystemAccessAsyncResponse,
                    int  /* request_id */,
                    bool /* allowed */)


//-----------------------------------------------------------------------------
// WebContents messages
// These are messages sent from the renderer back to the browser process.

IPC_MESSAGE_ROUTED2(RenderViewObserverHostQt_DidFetchDocumentMarkup,
                    uint64_t /* requestId */,
                    base::string16 /* markup */)

IPC_MESSAGE_ROUTED2(RenderViewObserverHostQt_DidFetchDocumentInnerText,
                    uint64_t /* requestId */,
                    base::string16 /* innerText */)

IPC_MESSAGE_ROUTED1(RenderViewObserverQt_SetBackgroundColor,
                    uint32_t /* color */)

IPC_MESSAGE_ROUTED0(RenderViewObserverHostQt_DidFirstVisuallyNonEmptyLayout)

//-----------------------------------------------------------------------------
// Misc messages
// These are messages sent from the renderer to the browser process.

// Sent by the renderer process to check whether access to web databases is
// granted by content settings.
IPC_SYNC_MESSAGE_CONTROL5_1(QtWebEngineHostMsg_AllowDatabase,
                            int /* render_frame_id */,
                            GURL /* origin_url */,
                            GURL /* top origin url */,
                            base::string16 /* database name */,
                            base::string16 /* database display name */,
                            bool /* allowed */)

// Sent by the renderer process to check whether access to DOM Storage is
// granted by content settings.
IPC_SYNC_MESSAGE_CONTROL4_1(QtWebEngineHostMsg_AllowDOMStorage,
                            int /* render_frame_id */,
                            GURL /* origin_url */,
                            GURL /* top origin url */,
                            bool /* if true local storage, otherwise session */,
                            bool /* allowed */)

// Sent by the renderer process to check whether access to FileSystem is
// granted by content settings.
IPC_SYNC_MESSAGE_CONTROL3_1(QtWebEngineHostMsg_RequestFileSystemAccessSync,
                            int /* render_frame_id */,
                            GURL /* origin_url */,
                            GURL /* top origin url */,
                            bool /* allowed */)

// Sent by the renderer process to check whether access to FileSystem is
// granted by content settings.
IPC_MESSAGE_CONTROL4(QtWebEngineHostMsg_RequestFileSystemAccessAsync,
                     int /* render_frame_id */,
                     int /* request_id */,
                     GURL /* origin_url */,
                     GURL /* top origin url */)

// Sent by the renderer process to check whether access to Indexed DB is
// granted by content settings.
IPC_SYNC_MESSAGE_CONTROL4_1(QtWebEngineHostMsg_AllowIndexedDB,
                            int /* render_frame_id */,
                            GURL /* origin_url */,
                            GURL /* top origin url */,
                            base::string16 /* database name */,
                            bool /* allowed */)
