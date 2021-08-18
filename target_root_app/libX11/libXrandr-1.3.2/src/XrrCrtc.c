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

XRRCrtcInfo *
XRRGetCrtcInfo (Display *dpy, XRRScreenResources *resources, RRCrtc crtc)
{
    XExtDisplayInfo	    *info = XRRFindDisplay(dpy);
    xRRGetCrtcInfoReply	    rep;
    xRRGetCrtcInfoReq	    *req;
    int			    nbytes, nbytesRead, rbytes;
    XRRCrtcInfo		    *xci;

    RRCheckExtension (dpy, info, NULL);

    LockDisplay (dpy);
    GetReq (RRGetCrtcInfo, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRGetCrtcInfo;
    req->crtc = crtc;
    req->configTimestamp = resources->configTimestamp;

    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse))
    {
	UnlockDisplay (dpy);
	SyncHandle ();
	return NULL;
    }

    nbytes = (long) rep.length << 2;

    nbytesRead = (long) (rep.nOutput * 4 +
			 rep.nPossibleOutput * 4);

    /* 
     * first we must compute how much space to allocate for 
     * randr library's use; we'll allocate the structures in a single
     * allocation, on cleanlyness grounds.
     */

    rbytes = (sizeof (XRRCrtcInfo) +
	      rep.nOutput * sizeof (RROutput) +
	      rep.nPossibleOutput * sizeof (RROutput));

    xci = (XRRCrtcInfo *) Xmalloc(rbytes);
    if (xci == NULL) {
	_XEatData (dpy, (unsigned long) nbytes);
	UnlockDisplay (dpy);
	SyncHandle ();
	return NULL;
    }

    xci->timestamp = rep.timestamp;
    xci->x = rep.x;
    xci->y = rep.y;
    xci->width = rep.width;
    xci->height = rep.height;
    xci->mode = rep.mode;
    xci->rotation = rep.rotation;
    xci->noutput = rep.nOutput;
    xci->outputs = (RROutput *) (xci + 1);
    xci->rotations = rep.rotations;
    xci->npossible = rep.nPossibleOutput;
    xci->possible = (RROutput *) (xci->outputs + rep.nOutput);

    _XRead32 (dpy, xci->outputs, rep.nOutput << 2);
    _XRead32 (dpy, xci->possible, rep.nPossibleOutput << 2);
    
    /*
     * Skip any extra data
     */
    if (nbytes > nbytesRead)
	_XEatData (dpy, (unsigned long) (nbytes - nbytesRead));
    
    UnlockDisplay (dpy);
    SyncHandle ();
    return (XRRCrtcInfo *) xci;
}

void
XRRFreeCrtcInfo (XRRCrtcInfo *crtcInfo)
{
    Xfree (crtcInfo);
}

Status
XRRSetCrtcConfig (Display *dpy,
		  XRRScreenResources *resources,
		  RRCrtc crtc,
		  Time timestamp,
		  int x, int y,
		  RRMode mode,
		  Rotation rotation,
		  RROutput *outputs,
		  int noutputs)
{
    XExtDisplayInfo	    *info = XRRFindDisplay(dpy);
    xRRSetCrtcConfigReply   rep;
    xRRSetCrtcConfigReq	    *req;

    RRCheckExtension (dpy, info, 0);

    LockDisplay(dpy);
    GetReq (RRSetCrtcConfig, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRSetCrtcConfig;
    req->length += noutputs;
    req->crtc = crtc;
    req->timestamp = timestamp;
    req->configTimestamp = resources->configTimestamp;
    req->x = x;
    req->y = y;
    req->mode = mode;
    req->rotation = rotation;
    Data32 (dpy, outputs, noutputs << 2);

    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse))
	rep.status = RRSetConfigFailed;
    UnlockDisplay (dpy);
    SyncHandle ();
    return rep.status;
}

int
XRRGetCrtcGammaSize (Display *dpy, RRCrtc crtc)
{
    XExtDisplayInfo		*info = XRRFindDisplay(dpy);
    xRRGetCrtcGammaSizeReply	rep;
    xRRGetCrtcGammaSizeReq	*req;

    RRCheckExtension (dpy, info, 0);

    LockDisplay(dpy);
    GetReq (RRGetCrtcGammaSize, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRGetCrtcGammaSize;
    req->crtc = crtc;

    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse))
	rep.size = 0;
    UnlockDisplay (dpy);
    SyncHandle ();
    return rep.size;
}

XRRCrtcGamma *
XRRGetCrtcGamma (Display *dpy, RRCrtc crtc)
{
    XExtDisplayInfo	    *info = XRRFindDisplay(dpy);
    xRRGetCrtcGammaReply    rep;
    xRRGetCrtcGammaReq	    *req;
    XRRCrtcGamma	    *crtc_gamma = NULL;
    long    		    nbytes;
    long    		    nbytesRead;

    RRCheckExtension (dpy, info, NULL);

    LockDisplay(dpy);
    GetReq (RRGetCrtcGamma, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRGetCrtcGamma;
    req->crtc = crtc;

    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse))
	goto out;

    nbytes = (long) rep.length << 2;
    
    /* three channels of CARD16 data */
    nbytesRead = (rep.size * 2 * 3);

    crtc_gamma = XRRAllocGamma (rep.size);
    
    if (!crtc_gamma)
    {
	_XEatData (dpy, (unsigned long) nbytes);
	goto out;
    }
    _XRead16 (dpy, crtc_gamma->red, rep.size * 2);
    _XRead16 (dpy, crtc_gamma->green, rep.size * 2);
    _XRead16 (dpy, crtc_gamma->blue, rep.size * 2);
    
    if (nbytes > nbytesRead)
	_XEatData (dpy, (unsigned long) (nbytes - nbytesRead));

out:
    UnlockDisplay (dpy);
    SyncHandle ();
    return crtc_gamma;
}

XRRCrtcGamma *
XRRAllocGamma (int size)
{
    XRRCrtcGamma    *crtc_gamma;

    crtc_gamma = Xmalloc (sizeof (XRRCrtcGamma) +
			  sizeof (crtc_gamma->red[0]) * size * 3);
    if (!crtc_gamma)
	return NULL;
    crtc_gamma->size = size;
    crtc_gamma->red = (unsigned short *) (crtc_gamma + 1);
    crtc_gamma->green = crtc_gamma->red + size;
    crtc_gamma->blue = crtc_gamma->green + size;
    return crtc_gamma;
}

void
XRRSetCrtcGamma (Display *dpy, RRCrtc crtc, XRRCrtcGamma *crtc_gamma)
{
    XExtDisplayInfo	    *info = XRRFindDisplay(dpy);
    xRRSetCrtcGammaReq	    *req;

    RRSimpleCheckExtension (dpy, info);

    LockDisplay(dpy);
    GetReq (RRSetCrtcGamma, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRSetCrtcGamma;
    req->crtc = crtc;
    req->size = crtc_gamma->size;
    req->length += (crtc_gamma->size * 2 * 3 + 3) >> 2;
    /*
     * Note this assumes the structure was allocated with XRRAllocGamma,
     * otherwise the channels might not be contiguous
     */
    Data16 (dpy, crtc_gamma->red, crtc_gamma->size * 2 * 3);
    
    UnlockDisplay (dpy);
    SyncHandle ();
}

void
XRRFreeGamma (XRRCrtcGamma *crtc_gamma)
{
    Xfree (crtc_gamma);
}

/* Version 1.3 additions */

static void
XTransform_from_xRenderTransform (XTransform *x,
				  xRenderTransform *render)
{
    x->matrix[0][0] = render->matrix11;
    x->matrix[0][1] = render->matrix12;
    x->matrix[0][2] = render->matrix13;

    x->matrix[1][0] = render->matrix21;
    x->matrix[1][1] = render->matrix22;
    x->matrix[1][2] = render->matrix23;

    x->matrix[2][0] = render->matrix31;
    x->matrix[2][1] = render->matrix32;
    x->matrix[2][2] = render->matrix33;
}

static void
xRenderTransform_from_XTransform (xRenderTransform *render,
				  XTransform *x)
{
    render->matrix11 = x->matrix[0][0];
    render->matrix12 = x->matrix[0][1];
    render->matrix13 = x->matrix[0][2];

    render->matrix21 = x->matrix[1][0];
    render->matrix22 = x->matrix[1][1];
    render->matrix23 = x->matrix[1][2];

    render->matrix31 = x->matrix[2][0];
    render->matrix32 = x->matrix[2][1];
    render->matrix33 = x->matrix[2][2];
}

void
XRRSetCrtcTransform (Display	*dpy,
		     RRCrtc	crtc, 
		     XTransform	*transform,
		     char	*filter,
		     XFixed	*params,
		     int	nparams)
{
    XExtDisplayInfo	    *info = XRRFindDisplay(dpy);
    xRRSetCrtcTransformReq  *req;
    int			    nbytes = strlen (filter);

    RRSimpleCheckExtension (dpy, info);

    LockDisplay(dpy);
    GetReq (RRSetCrtcTransform, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRSetCrtcTransform;
    req->crtc = crtc;

    xRenderTransform_from_XTransform (&req->transform, transform);

    req->nbytesFilter = nbytes;
    req->length += ((nbytes + 3) >> 2) + nparams;
    Data (dpy, filter, nbytes);
    Data32 (dpy, params, nparams << 2);

    UnlockDisplay (dpy);
    SyncHandle ();
}

#define CrtcTransformExtra	(SIZEOF(xRRGetCrtcTransformReply) - 32)
				
static const xRenderTransform identity = {
    0x10000, 0, 0,
    0, 0x10000, 0,
    0, 0, 0x10000,
};

static Bool
_XRRHasTransform (int major, int minor)
{
    return major > 1 || (major == 1 && minor >= 3);
}

Status
XRRGetCrtcTransform (Display	*dpy,
		     RRCrtc	crtc,
		     XRRCrtcTransformAttributes **attributes)
{
    XExtDisplayInfo		*info = XRRFindDisplay(dpy);
    xRRGetCrtcTransformReply	rep;
    xRRGetCrtcTransformReq	*req;
    int				major_version, minor_version;
    XRRCrtcTransformAttributes	*attr;
    char			*extra = NULL, *e;
    int				p;

    *attributes = NULL;

    RRCheckExtension (dpy, info, False);

    if (!XRRQueryVersion (dpy, &major_version, &minor_version) || 
	!_XRRHasTransform (major_version, minor_version))
    {
	/* For pre-1.3 servers, just report identity matrices everywhere */
	rep.pendingTransform = identity;
	rep.pendingNbytesFilter = 0;
	rep.pendingNparamsFilter = 0;
	rep.currentTransform = identity;
	rep.currentNbytesFilter = 0;
	rep.currentNparamsFilter = 0;
    }
    else
    {
	LockDisplay (dpy);
	GetReq (RRGetCrtcTransform, req);
	req->reqType = info->codes->major_opcode;
	req->randrReqType = X_RRGetCrtcTransform;
	req->crtc = crtc;
    
	if (!_XReply (dpy, (xReply *) &rep, CrtcTransformExtra >> 2, xFalse))
	{
	    rep.pendingTransform = identity;
	    rep.pendingNbytesFilter = 0;
	    rep.pendingNparamsFilter = 0;
	    rep.currentTransform = identity;
	    rep.currentNbytesFilter = 0;
	    rep.currentNparamsFilter = 0;
	}
	else
	{
	    int extraBytes = rep.length * 4 - CrtcTransformExtra;
	    extra = Xmalloc (extraBytes);
	    if (!extra) {
		_XEatData (dpy, extraBytes);
		UnlockDisplay (dpy);
		SyncHandle ();
		return False;
	    }
	    _XRead (dpy, extra, extraBytes);
	}

	UnlockDisplay (dpy);
	SyncHandle ();
    }

    attr = Xmalloc (sizeof (XRRCrtcTransformAttributes) +
		    rep.pendingNparamsFilter * sizeof (XFixed) +
		    rep.currentNparamsFilter * sizeof (XFixed) +
		    rep.pendingNbytesFilter + 1 +
		    rep.currentNbytesFilter + 1);

    if (!attr) {
	XFree (extra);
	return False;
    }
    XTransform_from_xRenderTransform (&attr->pendingTransform, &rep.pendingTransform);
    XTransform_from_xRenderTransform (&attr->currentTransform, &rep.currentTransform);

    attr->pendingParams = (XFixed *) (attr + 1);
    attr->currentParams = attr->pendingParams + rep.pendingNparamsFilter;
    attr->pendingFilter = (char *) (attr->currentParams + rep.currentNparamsFilter);
    attr->currentFilter = attr->pendingFilter + rep.pendingNbytesFilter + 1;

    e = extra;

    memcpy (attr->pendingFilter, e, rep.pendingNbytesFilter);
    attr->pendingFilter[rep.pendingNbytesFilter] = '\0';
    e += (rep.pendingNbytesFilter + 3) & ~3;
    for (p = 0; p < rep.pendingNparamsFilter; p++) {
	INT32	f;
	memcpy (&f, e, 4);
	e += 4;
	attr->pendingParams[p] = (XFixed) f;
    }
    attr->pendingNparams = rep.pendingNparamsFilter;

    memcpy (attr->currentFilter, e, rep.currentNbytesFilter);
    attr->currentFilter[rep.currentNbytesFilter] = '\0';
    e += (rep.currentNbytesFilter + 3) & ~3;
    for (p = 0; p < rep.currentNparamsFilter; p++) {
	INT32	f;
	memcpy (&f, e, 4);
	e += 4;
	attr->currentParams[p] = (XFixed) f;
    }
    attr->currentNparams = rep.currentNparamsFilter;

    if (extra)
	XFree (extra);
    *attributes = attr;

    return True;
}

XRRPanning *
XRRGetPanning (Display *dpy, XRRScreenResources *resources, RRCrtc crtc)
{
    XExtDisplayInfo	    *info = XRRFindDisplay(dpy);
    xRRGetPanningReply	    rep;
    xRRGetPanningReq	    *req;
    XRRPanning		    *xp;

    RRCheckExtension (dpy, info, NULL);

    LockDisplay (dpy);
    GetReq (RRGetPanning, req);
    req->reqType         = info->codes->major_opcode;
    req->randrReqType    = X_RRGetPanning;
    req->crtc            = crtc;

    if (!_XReply (dpy, (xReply *) &rep, 1, xFalse))
    {
	UnlockDisplay (dpy);
	SyncHandle ();
	return NULL;
    }

    if (! (xp = (XRRPanning *) Xmalloc(sizeof(XRRPanning))) ) {
	_XEatData (dpy, sizeof(XRRPanning));
	UnlockDisplay (dpy);
	SyncHandle ();
	return NULL;
    }

    xp->timestamp     = rep.timestamp;
    xp->left          = rep.left;
    xp->top           = rep.top;
    xp->width         = rep.width;
    xp->height        = rep.height;
    xp->track_left    = rep.track_left;
    xp->track_top     = rep.track_top;
    xp->track_width   = rep.track_width;
    xp->track_height  = rep.track_height;
    xp->border_left   = rep.border_left;
    xp->border_top    = rep.border_top;
    xp->border_right  = rep.border_right;
    xp->border_bottom = rep.border_bottom;

    UnlockDisplay (dpy);
    SyncHandle ();
    return (XRRPanning *) xp;
}

void
XRRFreePanning (XRRPanning *panning)
{
    Xfree (panning);
}

Status
XRRSetPanning (Display *dpy,
               XRRScreenResources *resources,
               RRCrtc crtc,
               XRRPanning *panning)
{
    XExtDisplayInfo	    *info = XRRFindDisplay(dpy);
    xRRSetPanningReply      rep;
    xRRSetPanningReq	    *req;

    RRCheckExtension (dpy, info, 0);

    LockDisplay(dpy);
    GetReq (RRSetPanning, req);
    req->reqType       = info->codes->major_opcode;
    req->randrReqType  = X_RRSetPanning;
    req->crtc          = crtc;
    req->timestamp     = panning->timestamp;
    req->left          = panning->left;
    req->top           = panning->top;
    req->width         = panning->width;
    req->height        = panning->height;
    req->track_left    = panning->track_left;
    req->track_top     = panning->track_top;
    req->track_width   = panning->track_width;
    req->track_height  = panning->track_height;
    req->border_left   = panning->border_left;
    req->border_top    = panning->border_top;
    req->border_right  = panning->border_right;
    req->border_bottom = panning->border_bottom;

    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse))
	rep.status = RRSetConfigFailed;
    UnlockDisplay (dpy);
    SyncHandle ();
    return rep.status;
}

