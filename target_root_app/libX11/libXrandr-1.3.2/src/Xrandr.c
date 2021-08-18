/*
 * Copyright © 2000 Compaq Computer Corporation, Inc.
 * Copyright © 2002 Hewlett Packard Company, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Compaq or HP not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  HP makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * HP DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL COMPAQ
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Jim Gettys, HP Labs, HP.
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

static XExtensionInfo XRRExtensionInfo;
_X_HIDDEN char XRRExtensionName[] = RANDR_NAME;

static Bool     XRRWireToEvent(Display *dpy, XEvent *event, xEvent *wire);
static Status   XRREventToWire(Display *dpy, XEvent *event, xEvent *wire);

static int
XRRCloseDisplay (Display *dpy, XExtCodes *codes);

static /* const */ XExtensionHooks rr_extension_hooks = {
    NULL,				/* create_gc */
    NULL,				/* copy_gc */
    NULL,				/* flush_gc */
    NULL,				/* free_gc */
    NULL,				/* create_font */
    NULL,				/* free_font */
    XRRCloseDisplay,			/* close_display */
    XRRWireToEvent,			/* wire_to_event */
    XRREventToWire,			/* event_to_wire */
    NULL,				/* error */
    NULL,				/* error_string */
};

static Bool XRRWireToEvent(Display *dpy, XEvent *event, xEvent *wire)
{
    XExtDisplayInfo *info = XRRFindDisplay(dpy);

    RRCheckExtension(dpy, info, False);

    switch ((wire->u.u.type & 0x7F) - info->codes->first_event)
    {
      case RRScreenChangeNotify: {
	XRRScreenChangeNotifyEvent *aevent= (XRRScreenChangeNotifyEvent *) event;
	xRRScreenChangeNotifyEvent *awire = (xRRScreenChangeNotifyEvent *) wire;
	aevent->type = awire->type & 0x7F;
	aevent->serial = _XSetLastRequestRead(dpy, (xGenericReply *) wire);
	aevent->send_event = (awire->type & 0x80) != 0;
	aevent->display = dpy;
	aevent->window = awire->window;
	aevent->root = awire->root;
	aevent->timestamp = awire->timestamp;
	aevent->config_timestamp = awire->configTimestamp;
	aevent->size_index = awire->sizeID;
	aevent->subpixel_order = awire->subpixelOrder;
	aevent->rotation = awire->rotation;
	aevent->width = awire->widthInPixels;
	aevent->height = awire->heightInPixels;
	aevent->mwidth = awire->widthInMillimeters;
	aevent->mheight = awire->heightInMillimeters;
	return True;
      }
      case RRNotify: {
	switch (wire->u.u.detail) {
	case RRNotify_OutputChange: {
	    XRROutputChangeNotifyEvent *aevent = (XRROutputChangeNotifyEvent *) event;
	    xRROutputChangeNotifyEvent *awire = (xRROutputChangeNotifyEvent *) wire;
	    aevent->type = awire->type & 0x7F;
	    aevent->serial = _XSetLastRequestRead(dpy, (xGenericReply *) wire);
	    aevent->send_event = (awire->type & 0x80) != 0;
	    aevent->display = dpy;
	    aevent->window = awire->window;
	    aevent->subtype = awire->subCode;
	    aevent->output = awire->output;
	    aevent->crtc = awire->crtc;
	    aevent->mode = awire->mode;
	    aevent->rotation = awire->rotation;
	    aevent->connection = awire->connection;
	    aevent->subpixel_order = awire->subpixelOrder;
	    return True;
	}
	case RRNotify_CrtcChange: {
	    XRRCrtcChangeNotifyEvent *aevent = (XRRCrtcChangeNotifyEvent *) event;
	    xRRCrtcChangeNotifyEvent *awire = (xRRCrtcChangeNotifyEvent *) wire;
	    aevent->type = awire->type & 0x7F;
	    aevent->serial = _XSetLastRequestRead(dpy, (xGenericReply *) wire);
	    aevent->send_event = (awire->type & 0x80) != 0;
	    aevent->display = dpy;
	    aevent->window = awire->window;
	    aevent->subtype = awire->subCode;
	    aevent->crtc = awire->crtc;
	    aevent->mode = awire->mode;
	    aevent->rotation = awire->rotation;
	    aevent->x = awire->x;
	    aevent->y = awire->y;
	    aevent->width = awire->width;
	    aevent->height = awire->height;
	    return True;
	}
	case RRNotify_OutputProperty: {
	    XRROutputPropertyNotifyEvent *aevent = (XRROutputPropertyNotifyEvent *) event;
	    xRROutputPropertyNotifyEvent *awire = (xRROutputPropertyNotifyEvent *) wire;
	    aevent->type = awire->type & 0x7F;
	    aevent->serial = _XSetLastRequestRead(dpy, (xGenericReply *) wire);
	    aevent->send_event = (awire->type & 0x80) != 0;
	    aevent->display = dpy;
	    aevent->window = awire->window;
	    aevent->subtype = awire->subCode;
	    aevent->output = awire->output;
	    aevent->property = awire->atom;
	    aevent->timestamp = awire->timestamp;
	    aevent->state = awire->state;
	    return True;
	}

	    break;
	}
      }
    }

    return False;
}

static Status XRREventToWire(Display *dpy, XEvent *event, xEvent *wire)
{
    XExtDisplayInfo *info = XRRFindDisplay(dpy);

    RRCheckExtension(dpy, info, False);

    switch ((event->type & 0x7F) - info->codes->first_event)
    {
      case RRScreenChangeNotify: {
	xRRScreenChangeNotifyEvent *awire = (xRRScreenChangeNotifyEvent *) wire;
	XRRScreenChangeNotifyEvent *aevent = (XRRScreenChangeNotifyEvent *) event;
	awire->type = aevent->type | (aevent->send_event ? 0x80 : 0);
	awire->rotation = (CARD8) aevent->rotation;
	awire->sequenceNumber = aevent->serial & 0xFFFF;
	awire->timestamp = aevent->timestamp;
	awire->configTimestamp = aevent->config_timestamp;
	awire->root = aevent->root;
	awire->window = aevent->window;
	awire->sizeID = aevent->size_index;
	awire->subpixelOrder = aevent->subpixel_order;
	awire->widthInPixels = aevent->width;
	awire->heightInPixels = aevent->height;
	awire->widthInMillimeters = aevent->mwidth;
	awire->heightInMillimeters = aevent->mheight;
	return True;
      }
      case RRNotify: {
	xRRCrtcChangeNotifyEvent *awire = (xRRCrtcChangeNotifyEvent *) wire;
	XRRNotifyEvent *aevent = (XRRNotifyEvent *) event;
	awire->type = aevent->type | (aevent->send_event ? 0x80 : 0);
	awire->sequenceNumber = aevent->serial & 0xFFFF;
	awire->subCode = aevent->subtype;
	switch (aevent->subtype) {
	case RRNotify_OutputChange: {
	    xRROutputChangeNotifyEvent *awire = (xRROutputChangeNotifyEvent *) wire;
	    XRROutputChangeNotifyEvent *aevent = (XRROutputChangeNotifyEvent *) event;
	    awire->window = aevent->window;
	    awire->output = aevent->output;
	    awire->crtc = aevent->crtc;
	    awire->mode = aevent->mode;
	    awire->rotation = aevent->rotation;
	    awire->connection = aevent->connection;
	    awire->subpixelOrder = aevent->subpixel_order;
	    return True;
	}
	case RRNotify_CrtcChange: {
	    xRRCrtcChangeNotifyEvent *awire = (xRRCrtcChangeNotifyEvent *) wire;
	    XRRCrtcChangeNotifyEvent *aevent = (XRRCrtcChangeNotifyEvent *) event;
	    awire->window = aevent->window;
	    awire->crtc = aevent->crtc;
	    awire->mode = aevent->mode;
	    awire->rotation = aevent->rotation;
	    awire->x = aevent->x;
	    awire->y = aevent->y;
	    awire->width = aevent->width;
	    awire->height = aevent->height;
	    return True;
	}
	case RRNotify_OutputProperty: {
	    xRROutputPropertyNotifyEvent *awire = (xRROutputPropertyNotifyEvent *) wire;
	    XRROutputPropertyNotifyEvent *aevent = (XRROutputPropertyNotifyEvent *) event;
	    awire->window = aevent->window;
	    awire->output = aevent->output;
	    awire->atom = aevent->property;
	    awire->timestamp = aevent->timestamp;
	    awire->state = aevent->state;
	    return True;
	}
	}
      }
    }
    return False;
}

_X_HIDDEN XExtDisplayInfo *
XRRFindDisplay (Display *dpy)
{
    XExtDisplayInfo *dpyinfo;
    XRandRInfo *xrri;
    int i, numscreens;

    dpyinfo = XextFindDisplay (&XRRExtensionInfo, dpy);
    if (!dpyinfo) {
	dpyinfo = XextAddDisplay (&XRRExtensionInfo, dpy, 
				  XRRExtensionName,
				  &rr_extension_hooks,
				  RRNumberEvents, NULL);
	numscreens = ScreenCount(dpy);
	xrri = Xmalloc (sizeof(XRandRInfo) + 
				 sizeof(char *) * numscreens);
	xrri->config = (XRRScreenConfiguration **)(xrri + 1);
	for(i = 0; i < numscreens; i++) 
	  xrri->config[i] = NULL;
	xrri->major_version = -1;
	dpyinfo->data = (char *) xrri;
    }
    return dpyinfo;
}

static int
XRRCloseDisplay (Display *dpy, XExtCodes *codes)
{
    int i;
    XRRScreenConfiguration **configs;
    XExtDisplayInfo *info = XRRFindDisplay (dpy);
    XRandRInfo *xrri;

    LockDisplay(dpy);
    /*
     * free cached data
     */
    if (XextHasExtension(info)) {
	xrri = (XRandRInfo *) info->data;
	if (xrri) {
	    configs = xrri->config;

	    for (i = 0; i < ScreenCount(dpy); i++) {
		if (configs[i] != NULL) XFree (configs[i]);
	    }
	    XFree (xrri);
	}
    }
    UnlockDisplay(dpy);
    return XextRemoveDisplay (&XRRExtensionInfo, dpy);
}

int XRRRootToScreen(Display *dpy, Window root)
{
  int snum;
  for (snum = 0; snum < ScreenCount(dpy); snum++) {
    if (RootWindow(dpy, snum) == root) return snum;
  }
  return -1;
}


Bool XRRQueryExtension (Display *dpy,
			int *event_base_return,
			int *error_base_return)
{
  XExtDisplayInfo *info = XRRFindDisplay (dpy);

    if (XextHasExtension(info)) {
	*event_base_return = info->codes->first_event;
	*error_base_return = info->codes->first_error;
	return True;
    } else {
	return False;
    }
}

_X_HIDDEN Bool
_XRRHasRates (int major, int minor)
{
    return major > 1 || (major == 1 && minor >= 1);
}

Status XRRQueryVersion (Display *dpy,
			    int	    *major_versionp,
			    int	    *minor_versionp)
{
    XExtDisplayInfo *info = XRRFindDisplay (dpy);
    xRRQueryVersionReply rep;
    xRRQueryVersionReq  *req;
    XRandRInfo *xrri;

    RRCheckExtension (dpy, info, 0);

    xrri = (XRandRInfo *) info->data;

    /* 
     * only get the version information from the server if we don't have it already
     */
    if (xrri->major_version == -1) {
      LockDisplay (dpy);
      GetReq (RRQueryVersion, req);
      req->reqType = info->codes->major_opcode;
      req->randrReqType = X_RRQueryVersion;
      req->majorVersion = RANDR_MAJOR;
      req->minorVersion = RANDR_MINOR;
      if (!_XReply (dpy, (xReply *) &rep, 0, xTrue)) {
	UnlockDisplay (dpy);
	SyncHandle ();
	return 0;
      }
      xrri->major_version = rep.majorVersion;
      xrri->minor_version = rep.minorVersion;
      xrri->has_rates = _XRRHasRates (xrri->major_version, xrri->minor_version);
      UnlockDisplay (dpy);
      SyncHandle ();
    }
    *major_versionp = xrri->major_version;
    *minor_versionp = xrri->minor_version;
    return 1;
}

_X_HIDDEN Bool
_XRRVersionHandler (Display	    *dpy,
			xReply	    *rep,
			char	    *buf,
			int	    len,
			XPointer    data)
{
    xRRQueryVersionReply	replbuf;
    xRRQueryVersionReply	*repl;
    _XRRVersionState	*state = (_XRRVersionState *) data;

    if (dpy->last_request_read != state->version_seq)
	return False;
    if (rep->generic.type == X_Error)
    {
	state->error = True;
	return False;
    }
    repl = (xRRQueryVersionReply *)
	_XGetAsyncReply(dpy, (char *)&replbuf, rep, buf, len,
		     (SIZEOF(xRRQueryVersionReply) - SIZEOF(xReply)) >> 2,
			True);
    state->major_version = repl->majorVersion;
    state->minor_version = repl->minorVersion;
    return True;
}

/* 
 * in protocol version 0.1, routine added to allow selecting for new events.
 */

void XRRSelectInput (Display *dpy, Window window, int mask)
{
    XExtDisplayInfo *info = XRRFindDisplay (dpy);
    xRRSelectInputReq  *req;

    RRSimpleCheckExtension (dpy, info);

    LockDisplay (dpy);
    GetReq (RRSelectInput, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRSelectInput;
    req->window = window;
    req->enable = 0;
    if (mask) req->enable = mask;
    UnlockDisplay (dpy);
    SyncHandle ();
    return;
}

int XRRUpdateConfiguration(XEvent *event)
{
    XRRScreenChangeNotifyEvent *scevent;
    XConfigureEvent *rcevent;
    Display *dpy = event->xany.display;
    XExtDisplayInfo *info;
    XRandRInfo *xrri;
    int snum;

    /* first, see if it is a vanilla configure notify event */
    if (event->type == ConfigureNotify) {
	rcevent = (XConfigureEvent *) event;
	snum = XRRRootToScreen(dpy, rcevent->window);
	if (snum != -1) {
	    dpy->screens[snum].width   = rcevent->width;
	    dpy->screens[snum].height  = rcevent->height;
	    return 1;
	}
    }

    info = XRRFindDisplay(dpy);
    RRCheckExtension (dpy, info, 0);

    switch (event->type - info->codes->first_event) {
    case RRScreenChangeNotify:
	scevent = (XRRScreenChangeNotifyEvent *) event;
	snum = XRRRootToScreen(dpy, 
			       ((XRRScreenChangeNotifyEvent *) event)->root);
	if (scevent->rotation & (RR_Rotate_90 | RR_Rotate_270)) {
		dpy->screens[snum].width   = scevent->height;
		dpy->screens[snum].height  = scevent->width;
		dpy->screens[snum].mwidth  = scevent->mheight;
		dpy->screens[snum].mheight = scevent->mwidth;
	} else {
		dpy->screens[snum].width   = scevent->width;
		dpy->screens[snum].height  = scevent->height;
		dpy->screens[snum].mwidth  = scevent->mwidth;
		dpy->screens[snum].mheight = scevent->mheight;
	}
	XRenderSetSubpixelOrder (dpy, snum, scevent->subpixel_order);
	break;
    default:
	return 0;
    }
    xrri = (XRandRInfo *) info->data;
    /* 
     * so the next time someone wants some data, it will be fetched; 
     * it might be better to force the round trip immediately, but 
     * I dislike pounding the server simultaneously when not necessary
     */
    if (xrri->config[snum] != NULL) {
	XFree (xrri->config[snum]);
	xrri->config[snum] = NULL;
    }
    return 1;
}
