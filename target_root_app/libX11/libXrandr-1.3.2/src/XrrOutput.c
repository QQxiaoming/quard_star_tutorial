/*
 * Copyright © 2006 Keith Packard
 * Copyright © 2008 Red Hat, Inc.
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

#define OutputInfoExtra	(SIZEOF(xRRGetOutputInfoReply) - 32)
				
XRROutputInfo *
XRRGetOutputInfo (Display *dpy, XRRScreenResources *resources, RROutput output)
{
    XExtDisplayInfo		*info = XRRFindDisplay(dpy);
    xRRGetOutputInfoReply	rep;
    xRRGetOutputInfoReq		*req;
    int				nbytes, nbytesRead, rbytes;
    XRROutputInfo		*xoi;

    RRCheckExtension (dpy, info, NULL);

    LockDisplay (dpy);
    GetReq (RRGetOutputInfo, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRGetOutputInfo;
    req->output = output;
    req->configTimestamp = resources->configTimestamp;

    if (!_XReply (dpy, (xReply *) &rep, OutputInfoExtra >> 2, xFalse))
    {
	UnlockDisplay (dpy);
	SyncHandle ();
	return NULL;
    }

    nbytes = ((long) (rep.length) << 2) - OutputInfoExtra;

    nbytesRead = (long) (rep.nCrtcs * 4 +
			 rep.nModes * 4 +
			 rep.nClones * 4 +
			 ((rep.nameLength + 3) & ~3));

    /* 
     * first we must compute how much space to allocate for 
     * randr library's use; we'll allocate the structures in a single
     * allocation, on cleanlyness grounds.
     */

    rbytes = (sizeof (XRROutputInfo) +
	      rep.nCrtcs * sizeof (RRCrtc) +
	      rep.nModes * sizeof (RRMode) +
	      rep.nClones * sizeof (RROutput) +
	      rep.nameLength + 1);	    /* '\0' terminate name */

    xoi = (XRROutputInfo *) Xmalloc(rbytes);
    if (xoi == NULL) {
	_XEatData (dpy, (unsigned long) nbytes);
	UnlockDisplay (dpy);
	SyncHandle ();
	return NULL;
    }

    xoi->timestamp = rep.timestamp;
    xoi->crtc = rep.crtc;
    xoi->mm_width = rep.mmWidth;
    xoi->mm_height = rep.mmHeight;
    xoi->connection = rep.connection;
    xoi->subpixel_order = rep.subpixelOrder;
    xoi->ncrtc = rep.nCrtcs;
    xoi->crtcs = (RRCrtc *) (xoi + 1);
    xoi->nmode = rep.nModes;
    xoi->npreferred = rep.nPreferred;
    xoi->modes = (RRMode *) (xoi->crtcs + rep.nCrtcs);
    xoi->nclone = rep.nClones;
    xoi->clones = (RROutput *) (xoi->modes + rep.nModes);
    xoi->name = (char *) (xoi->clones + rep.nClones);

    _XRead32 (dpy, xoi->crtcs, rep.nCrtcs << 2);
    _XRead32 (dpy, xoi->modes, rep.nModes << 2);
    _XRead32 (dpy, xoi->clones, rep.nClones << 2);
    
    /*
     * Read name and '\0' terminate
     */
    _XReadPad (dpy, xoi->name, rep.nameLength);
    xoi->name[rep.nameLength] = '\0';
    
    /*
     * Skip any extra data
     */
    if (nbytes > nbytesRead)
	_XEatData (dpy, (unsigned long) (nbytes - nbytesRead));
    
    UnlockDisplay (dpy);
    SyncHandle ();
    return (XRROutputInfo *) xoi;
}

void
XRRFreeOutputInfo (XRROutputInfo *outputInfo)
{
    Xfree (outputInfo);
}

static Bool
_XRRHasOutputPrimary (int major, int minor)
{
    return major > 1 || (major == 1 && minor >= 3);
}

void
XRRSetOutputPrimary(Display *dpy, Window window, RROutput output)
{
    XExtDisplayInfo	    *info = XRRFindDisplay(dpy);
    xRRSetOutputPrimaryReq  *req;
    int			    major_version, minor_version;

    RRSimpleCheckExtension (dpy, info);

    if (!XRRQueryVersion (dpy, &major_version, &minor_version) || 
	!_XRRHasOutputPrimary (major_version, minor_version))
	return;

    LockDisplay(dpy);
    GetReq (RRSetOutputPrimary, req);
    req->reqType       = info->codes->major_opcode;
    req->randrReqType  = X_RRSetOutputPrimary;
    req->window        = window;
    req->output	       = output;

    UnlockDisplay (dpy);
    SyncHandle ();
}

RROutput
XRRGetOutputPrimary(Display *dpy, Window window)
{
    XExtDisplayInfo	    *info = XRRFindDisplay(dpy);
    xRRGetOutputPrimaryReq  *req;
    xRRGetOutputPrimaryReply rep;
    int			    major_version, minor_version;

    RRCheckExtension (dpy, info, 0);

    if (!XRRQueryVersion (dpy, &major_version, &minor_version) || 
	!_XRRHasOutputPrimary (major_version, minor_version))
	return None;

    LockDisplay(dpy);
    GetReq (RRGetOutputPrimary, req);
    req->reqType	= info->codes->major_opcode;
    req->randrReqType	= X_RRGetOutputPrimary;
    req->window		= window;

    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse))
	rep.output = None;
	
    UnlockDisplay(dpy);
    SyncHandle();

    return rep.output;
}
