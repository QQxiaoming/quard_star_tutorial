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

/*
 * this is cheating on the knowledge that the two requests are identical
 * but for the request number.
 */
static XRRScreenResources *
doGetScreenResources (Display *dpy, Window window, int poll)
{
    XExtDisplayInfo		*info = XRRFindDisplay(dpy);
    xRRGetScreenResourcesReply  rep;
    xRRGetScreenResourcesReq	*req;
    _XAsyncHandler		async;
    _XRRVersionState		async_state;
    int				nbytes, nbytesRead, rbytes;
    int				i;
    xRRQueryVersionReq		*vreq;
    XRRScreenResources		*xrsr;
    char			*names;
    char			*wire_names, *wire_name;
    Bool			getting_version = False;
    XRandRInfo			*xrri;

    RRCheckExtension (dpy, info, NULL);

    LockDisplay (dpy);
    xrri = (XRandRInfo *) info->data;

    if (xrri->major_version == -1)
    {
	/* hide a version query in the request */
	GetReq (RRQueryVersion, vreq);
	vreq->reqType = info->codes->major_opcode;
	vreq->randrReqType = X_RRQueryVersion;
	vreq->majorVersion = RANDR_MAJOR;
	vreq->minorVersion = RANDR_MINOR;
    
	async_state.version_seq = dpy->request;
	async_state.error = False;
	async.next = dpy->async_handlers;
	async.handler = _XRRVersionHandler;
	async.data = (XPointer) &async_state;
	dpy->async_handlers = &async;

	getting_version = True;
    }

    GetReq (RRGetScreenResources, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = poll ? X_RRGetScreenResources
			     : X_RRGetScreenResourcesCurrent;
    req->window = window;

    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse))
    {
	if (getting_version)
	    DeqAsyncHandler (dpy, &async);
	UnlockDisplay (dpy);
	SyncHandle ();
	return NULL;
    }
    if (getting_version)
    {
	DeqAsyncHandler (dpy, &async);
	if (async_state.error)
	{
	    UnlockDisplay (dpy);
	    SyncHandle();
	    LockDisplay (dpy);
	}
	xrri->major_version = async_state.major_version;
	xrri->minor_version = async_state.minor_version;
	xrri->has_rates = _XRRHasRates (xrri->minor_version, xrri->major_version);
    }

    nbytes = (long) rep.length << 2;

    nbytesRead = (long) (rep.nCrtcs * 4 +
			 rep.nOutputs * 4 +
			 rep.nModes * SIZEOF (xRRModeInfo) +
			 ((rep.nbytesNames + 3) & ~3));

    /* 
     * first we must compute how much space to allocate for 
     * randr library's use; we'll allocate the structures in a single
     * allocation, on cleanlyness grounds.
     */

    rbytes = (sizeof (XRRScreenResources) +
	      rep.nCrtcs * sizeof (RRCrtc) +
	      rep.nOutputs * sizeof (RROutput) +
	      rep.nModes * sizeof (XRRModeInfo) +
	      rep.nbytesNames + rep.nModes);	/* '\0' terminate names */

    xrsr = (XRRScreenResources *) Xmalloc(rbytes);
    wire_names = (char *) Xmalloc (rep.nbytesNames);
    if (xrsr == NULL || wire_names == NULL) {
	if (xrsr) Xfree (xrsr);
	if (wire_names) Xfree (wire_names);
	_XEatData (dpy, (unsigned long) nbytes);
	UnlockDisplay (dpy);
	SyncHandle ();
	return NULL;
    }

    xrsr->timestamp = rep.timestamp;
    xrsr->configTimestamp = rep.configTimestamp;
    xrsr->ncrtc = rep.nCrtcs;
    xrsr->crtcs = (RRCrtc *) (xrsr + 1);
    xrsr->noutput = rep.nOutputs;
    xrsr->outputs = (RROutput *) (xrsr->crtcs + rep.nCrtcs);
    xrsr->nmode = rep.nModes;
    xrsr->modes = (XRRModeInfo *) (xrsr->outputs + rep.nOutputs);
    names = (char *) (xrsr->modes + rep.nModes);

    _XRead32 (dpy, xrsr->crtcs, rep.nCrtcs << 2);
    _XRead32 (dpy, xrsr->outputs, rep.nOutputs << 2);
    
    for (i = 0; i < rep.nModes; i++)  {
	xRRModeInfo modeInfo;
	
	_XReadPad (dpy, (char *) &modeInfo, SIZEOF (xRRModeInfo));
	xrsr->modes[i].id = modeInfo.id;
	xrsr->modes[i].width = modeInfo.width;
	xrsr->modes[i].height = modeInfo.height;
	xrsr->modes[i].dotClock = modeInfo.dotClock;
	xrsr->modes[i].hSyncStart = modeInfo.hSyncStart;
	xrsr->modes[i].hSyncEnd = modeInfo.hSyncEnd;
	xrsr->modes[i].hTotal = modeInfo.hTotal;
	xrsr->modes[i].hSkew = modeInfo.hSkew;
	xrsr->modes[i].vSyncStart = modeInfo.vSyncStart;
	xrsr->modes[i].vSyncEnd = modeInfo.vSyncEnd;
	xrsr->modes[i].vTotal = modeInfo.vTotal;
	xrsr->modes[i].nameLength = modeInfo.nameLength;
	xrsr->modes[i].modeFlags = modeInfo.modeFlags;
    }

    /*
     * Read names and '\0' pad each one
     */
    _XReadPad (dpy, wire_names, rep.nbytesNames);
    wire_name = wire_names;
    for (i = 0; i < rep.nModes; i++)  {
	xrsr->modes[i].name = names;
	memcpy (names, wire_name, xrsr->modes[i].nameLength);
	names[xrsr->modes[i].nameLength] = '\0';
	names += xrsr->modes[i].nameLength + 1;
	wire_name += xrsr->modes[i].nameLength;
    }
    Xfree (wire_names);
    
    /*
     * Skip any extra data
     */
    if (nbytes > nbytesRead)
	_XEatData (dpy, (unsigned long) (nbytes - nbytesRead));
    
    UnlockDisplay (dpy);
    SyncHandle();
    return (XRRScreenResources *) xrsr;
}

XRRScreenResources *
XRRGetScreenResources(Display *dpy, Window window)
{
    return doGetScreenResources(dpy, window, 1);
}

XRRScreenResources *
XRRGetScreenResourcesCurrent(Display *dpy, Window window)
{
    return doGetScreenResources(dpy, window, 0);
}

void
XRRFreeScreenResources (XRRScreenResources *resources)
{
    Xfree (resources);
}

Status
XRRGetScreenSizeRange (Display *dpy, Window window,
		       int *minWidth, int *minHeight,
		       int *maxWidth, int *maxHeight)
{
    XExtDisplayInfo		*info = XRRFindDisplay(dpy);
    xRRGetScreenSizeRangeReq	*req;
    xRRGetScreenSizeRangeReply	rep;

    RRCheckExtension (dpy, info, 0);
    LockDisplay (dpy);
    GetReq (RRGetScreenSizeRange, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRGetScreenSizeRange;
    req->window = window;
    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse))
    {
	UnlockDisplay (dpy);
	SyncHandle ();
	return False;
    }
    UnlockDisplay (dpy);
    SyncHandle ();
    *minWidth = rep.minWidth;
    *minHeight = rep.minHeight;
    *maxWidth = rep.maxWidth;
    *maxHeight = rep.maxHeight;
   return True;
}

void
XRRSetScreenSize (Display *dpy, Window window,
		  int width, int height,
		  int mmWidth, int mmHeight)
{
    XExtDisplayInfo		*info = XRRFindDisplay(dpy);
    xRRSetScreenSizeReq		*req;

    RRSimpleCheckExtension (dpy, info);
    LockDisplay (dpy);
    GetReq (RRSetScreenSize, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRSetScreenSize;
    req->window = window;
    req->width = width;
    req->height = height;
    req->widthInMillimeters = mmWidth;
    req->heightInMillimeters = mmHeight;
    UnlockDisplay (dpy);
    SyncHandle ();
}
