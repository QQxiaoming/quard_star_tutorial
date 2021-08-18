/*
 * Copyright © 2006 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <X11/Xlib.h>
/* we need to be able to manipulate the Display structure on events */
#include <X11/Xlibint.h>
#include <X11/extensions/render.h>
#include <X11/extensions/Xrender.h>
#include "Xrandrint.h"

XRRModeInfo *
XRRAllocModeInfo (char *name, int nameLength)
{
    XRRModeInfo	*mode_info;

    mode_info = Xmalloc (sizeof (XRRModeInfo) + nameLength + 1);
    if (!mode_info)
	return NULL;
    memset (mode_info, '\0', sizeof (XRRModeInfo));
    mode_info->nameLength = nameLength;
    mode_info->name = (char *) (mode_info + 1);
    memcpy (mode_info->name, name, nameLength);
    mode_info->name[nameLength] = '\0';
    return mode_info;
}

RRMode
XRRCreateMode (Display *dpy, Window window, XRRModeInfo *mode_info)
{
    XExtDisplayInfo	    *info = XRRFindDisplay(dpy);
    xRRCreateModeReq	    *req;
    xRRCreateModeReply	    rep;

    RRCheckExtension (dpy, info, None);

    LockDisplay(dpy);
    GetReq (RRCreateMode, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRCreateMode;
    req->length += (mode_info->nameLength + 3) >> 2;
    
    req->window = window;
    
    req->modeInfo.id = 0;
    req->modeInfo.width = mode_info->width;
    req->modeInfo.height = mode_info->height;
    req->modeInfo.dotClock = mode_info->dotClock;
    req->modeInfo.hSyncStart = mode_info->hSyncStart;
    req->modeInfo.hSyncEnd = mode_info->hSyncEnd;
    req->modeInfo.hTotal = mode_info->hTotal;
    req->modeInfo.hSkew = mode_info->hSkew;
    req->modeInfo.vSyncStart = mode_info->vSyncStart;
    req->modeInfo.vSyncEnd = mode_info->vSyncEnd;
    req->modeInfo.vTotal = mode_info->vTotal;
    req->modeInfo.nameLength = mode_info->nameLength;
    req->modeInfo.modeFlags = mode_info->modeFlags;
    
    Data (dpy, mode_info->name, mode_info->nameLength);
    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse))
    {
	UnlockDisplay (dpy);
	SyncHandle ();
	return None;
    }
    
    UnlockDisplay (dpy);
    SyncHandle ();
    return rep.mode;
}

void
XRRDestroyMode (Display *dpy, RRMode mode)
{
    XExtDisplayInfo	    *info = XRRFindDisplay(dpy);
    xRRDestroyModeReq	    *req;
    RRSimpleCheckExtension (dpy, info);

    LockDisplay(dpy);
    GetReq (RRDestroyMode, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRDestroyMode;
    req->mode = mode;
    UnlockDisplay (dpy);
    SyncHandle ();
}

void
XRRAddOutputMode (Display *dpy, RROutput output, RRMode mode)
{
    XExtDisplayInfo	    *info = XRRFindDisplay(dpy);
    xRRAddOutputModeReq	    *req;
    RRSimpleCheckExtension (dpy, info);

    LockDisplay(dpy);
    GetReq (RRAddOutputMode, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRAddOutputMode;
    req->output = output;
    req->mode = mode;
    UnlockDisplay (dpy);
    SyncHandle ();
}

void
XRRDeleteOutputMode (Display *dpy, RROutput output, RRMode mode)
{
    XExtDisplayInfo	    *info = XRRFindDisplay(dpy);
    xRRDeleteOutputModeReq  *req;
    RRSimpleCheckExtension (dpy, info);

    LockDisplay(dpy);
    GetReq (RRDeleteOutputMode, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRDeleteOutputMode;
    req->output = output;
    req->mode = mode;
    UnlockDisplay (dpy);
    SyncHandle ();
}

void
XRRFreeModeInfo (XRRModeInfo *modeInfo)
{
    Xfree (modeInfo);
}
