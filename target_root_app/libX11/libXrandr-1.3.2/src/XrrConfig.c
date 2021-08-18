/*
 * Copyright © 2000 Compaq Computer Corporation, Inc.
 * Copyright © 2002 Hewlett Packard Company, Inc.
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
 *
 * Author:  Jim Gettys, HP Labs, HP.
 * Author:  Keith Packard, Intel Corporation
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

static XRRScreenConfiguration *_XRRGetScreenInfo (Display *dpy,
						  XExtDisplayInfo *info,
						  Window window);

Rotation XRRConfigRotations(XRRScreenConfiguration *config, Rotation *current_rotation)
{
  *current_rotation = config->current_rotation;
  return config->rotations;
}

XRRScreenSize *XRRConfigSizes(XRRScreenConfiguration *config, int *nsizes)
{
   *nsizes = config->nsizes;
  return config->sizes;
}

short *XRRConfigRates (XRRScreenConfiguration *config, int sizeID, int *nrates)
{
    short   *r = config->rates;
    int	    nents = config->nrates;

    /* Skip over the intervening rate lists */
    while (sizeID > 0 && nents > 0)
    {
	int i = (*r + 1);
	r += i;
	nents -= i;
	sizeID--;
    }
    if (!nents)
    {
	*nrates = 0;
	return NULL;
    }
    *nrates = (int) *r;
    return r + 1;
}

Time XRRConfigTimes (XRRScreenConfiguration *config, Time *config_timestamp)
{
    *config_timestamp = config->config_timestamp;
    return config->timestamp;
}


SizeID XRRConfigCurrentConfiguration (XRRScreenConfiguration *config, 
			      Rotation *rotation)
{
    *rotation = (Rotation) config->current_rotation;
    return (SizeID) config->current_size;
}

short XRRConfigCurrentRate (XRRScreenConfiguration *config)
{
    return config->current_rate;
}

/* 
 * Go get the screen configuration data and salt it away for future use; 
 * returns NULL if extension not supported
 */
static XRRScreenConfiguration *_XRRValidateCache (Display *dpy, 
						  XExtDisplayInfo *info,
						  int screen)
{
    XRRScreenConfiguration **configs;
    XRandRInfo *xrri;

    if ((screen >= 0) && (screen < ScreenCount(dpy)) && XextHasExtension(info)) {
	xrri = (XRandRInfo *) info->data;
	configs = xrri->config;

	if (configs[screen] == NULL)
	    configs[screen] = _XRRGetScreenInfo (dpy, info, RootWindow(dpy, screen));
	return configs[screen];
    } else {
	return NULL;
    }
}

/* given a screen, return the information from the (possibly) cached data */
Rotation XRRRotations(Display *dpy, int screen, Rotation *current_rotation)
{
  XRRScreenConfiguration *config;
  XExtDisplayInfo *info = XRRFindDisplay(dpy);
  Rotation cr;
  LockDisplay(dpy);
  if ((config = _XRRValidateCache(dpy, info, screen))) {
    *current_rotation = config->current_rotation;
    cr = config->rotations;
    UnlockDisplay(dpy);
    return cr;
  }
  else {
    UnlockDisplay(dpy);
    *current_rotation = RR_Rotate_0;
    return 0;	/* no rotations supported */
  }
}

/* given a screen, return the information from the (possibly) cached data */
XRRScreenSize *XRRSizes(Display *dpy, int screen, int *nsizes)
{
  XRRScreenConfiguration *config; 
  XExtDisplayInfo *info = XRRFindDisplay(dpy);
  XRRScreenSize *sizes;

  LockDisplay(dpy);
  if ((config = _XRRValidateCache(dpy, info, screen))) {
    *nsizes = config->nsizes;
    sizes = config->sizes;
    UnlockDisplay(dpy);
    return sizes;
    }
  else {
    UnlockDisplay(dpy);
    *nsizes = 0;
    return NULL;
  }  
}

short *XRRRates (Display *dpy, int screen, int sizeID, int *nrates)
{
  XRRScreenConfiguration *config; 
  XExtDisplayInfo *info = XRRFindDisplay(dpy);
  short *rates;

  LockDisplay(dpy);
  if ((config = _XRRValidateCache(dpy, info, screen))) {
    rates = XRRConfigRates (config, sizeID, nrates);
    UnlockDisplay(dpy);
    return rates;
    }
  else {
    UnlockDisplay(dpy);
    *nrates = 0;
    return NULL;
  }  
}

/* given a screen, return the information from the (possibly) cached data */
Time XRRTimes (Display *dpy, int screen, Time *config_timestamp)
{
  XRRScreenConfiguration *config; 
  XExtDisplayInfo *info = XRRFindDisplay(dpy);
  Time ts;

  LockDisplay(dpy);
  if ((config = _XRRValidateCache(dpy, info, screen))) {
      *config_timestamp = config->config_timestamp;
      ts = config->timestamp;
      UnlockDisplay(dpy);
      return ts;
    } else {
      UnlockDisplay(dpy);
	return CurrentTime;
    }
}

/* need a version that does not hold the display lock */
static XRRScreenConfiguration *_XRRGetScreenInfo (Display *dpy,
						  XExtDisplayInfo *info,
						  Window window)
{
    xRRGetScreenInfoReply   rep;
    xRRGetScreenInfoReq	    *req;
    _XAsyncHandler 	    async;
    _XRRVersionState	    async_state;
    int			    nbytes, nbytesRead, rbytes;
    int			    i;
    xScreenSizes	    size;
    struct _XRRScreenConfiguration  *scp;
    XRRScreenSize	    *ssp;
    short    		    *rates;
    xRRQueryVersionReq      *vreq;
    XRandRInfo		    *xrri;
    Bool		    getting_version = False;

    xrri = (XRandRInfo *) info->data;
    if (!xrri)
	return NULL;

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

    GetReq (RRGetScreenInfo, req);
    req->reqType = info->codes->major_opcode;
    req->randrReqType = X_RRGetScreenInfo;
    req->window = window;

    if (!_XReply (dpy, (xReply *) &rep, 0, xFalse))
    {
	if (getting_version)
	    DeqAsyncHandler (dpy, &async);
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

    /*
     * Make the reply compatible with v1.1
     */
    if (!xrri->has_rates)
    {
	rep.rate = 0;
	rep.nrateEnts = 0;
    }
    
    nbytes = (long) rep.length << 2;

    nbytesRead = (long) (rep.nSizes * SIZEOF (xScreenSizes) +
			 ((rep.nrateEnts + 1)& ~1) * 2 /* SIZEOF (CARD16) */);
    
    /* 
     * first we must compute how much space to allocate for 
     * randr library's use; we'll allocate the structures in a single
     * allocation, on cleanlyness grounds.
     */

    rbytes = sizeof (XRRScreenConfiguration) +
      (rep.nSizes * sizeof (XRRScreenSize) +
       rep.nrateEnts * sizeof (int));

    scp = (struct _XRRScreenConfiguration *) Xmalloc(rbytes);
    if (scp == NULL) {
	_XEatData (dpy, (unsigned long) nbytes);
	return NULL;
    }


    ssp = (XRRScreenSize *)(scp + 1);
    rates = (short *) (ssp + rep.nSizes);

    /* set up the screen configuration structure */
    scp->screen = 
      ScreenOfDisplay (dpy, XRRRootToScreen(dpy, rep.root));

    scp->sizes = ssp;
    scp->rates = rates;
    scp->rotations = rep.setOfRotations;
    scp->current_size = rep.sizeID;
    scp->current_rate = rep.rate;
    scp->current_rotation = rep.rotation;
    scp->timestamp = rep.timestamp;
    scp->config_timestamp = rep.configTimestamp;
    scp->nsizes = rep.nSizes;
    scp->nrates = rep.nrateEnts;

    /*
     * Time to unpack the data from the server.
     */

    /*
     * First the size information
     */
    for (i = 0; i < rep.nSizes; i++)  {
	_XReadPad (dpy, (char *) &size, SIZEOF (xScreenSizes));
	
        ssp[i].width = size.widthInPixels;
	ssp[i].height = size.heightInPixels;
	ssp[i].mwidth = size.widthInMillimeters;
	ssp[i].mheight = size.heightInMillimeters;
    }
    /*
     * And the rates
     */
    _XRead16Pad (dpy, rates, 2 /* SIZEOF (CARD16) */ * rep.nrateEnts);
    
    /*
     * Skip any extra data
     */
    if (nbytes > nbytesRead)
	_XEatData (dpy, (unsigned long) (nbytes - nbytesRead));
    
    return (XRRScreenConfiguration *)(scp);
}

XRRScreenConfiguration *XRRGetScreenInfo (Display *dpy, Window window)
{
    XRRScreenConfiguration *config;
    XExtDisplayInfo *info = XRRFindDisplay(dpy);
    XRRFindDisplay(dpy);
    LockDisplay (dpy);
    config = _XRRGetScreenInfo(dpy, info, window);
    UnlockDisplay (dpy);
    SyncHandle ();
    return config;
}

    
void XRRFreeScreenConfigInfo (XRRScreenConfiguration *config)
{
    Xfree (config);
}

Status XRRSetScreenConfigAndRate (Display *dpy,
				  XRRScreenConfiguration *config,
				  Drawable draw,
				  int size_index,
				  Rotation rotation, 
				  short rate,
				  Time timestamp)
{
    XExtDisplayInfo *info = XRRFindDisplay (dpy);
    xRRSetScreenConfigReply rep;
    XRandRInfo *xrri;
    int major, minor;

    RRCheckExtension (dpy, info, 0);

    /* Make sure has_rates is set */
    if (!XRRQueryVersion (dpy, &major, &minor))
	return 0;
    
    LockDisplay (dpy);
    xrri = (XRandRInfo *) info->data;
    if (xrri->has_rates)
    {
	xRRSetScreenConfigReq  *req;
	GetReq (RRSetScreenConfig, req);
	req->reqType = info->codes->major_opcode;
	req->randrReqType = X_RRSetScreenConfig;
	req->drawable = draw;
	req->sizeID = size_index;
	req->rotation = rotation;
	req->timestamp = timestamp;
	req->configTimestamp = config->config_timestamp;
	req->rate = rate;
    }
    else
    {
	xRR1_0SetScreenConfigReq  *req;
	GetReq (RR1_0SetScreenConfig, req);
	req->reqType = info->codes->major_opcode;
	req->randrReqType = X_RRSetScreenConfig;
	req->drawable = draw;
	req->sizeID = size_index;
	req->rotation = rotation;
	req->timestamp = timestamp;
	req->configTimestamp = config->config_timestamp;
    }
    
    (void) _XReply (dpy, (xReply *) &rep, 0, xTrue);

    /* actually .errorCode in struct xError */
    if (rep.status == RRSetConfigSuccess) {
      /* if we succeed, set our view of reality to what we set it to */
      config->config_timestamp = rep.newConfigTimestamp;
      config->timestamp = rep.newTimestamp;
      config->screen = ScreenOfDisplay (dpy, XRRRootToScreen(dpy, rep.root));
      config->current_size = size_index;
      config->current_rotation = rotation;
    }
    UnlockDisplay (dpy);
    SyncHandle ();
    return(rep.status);
}

Status XRRSetScreenConfig (Display *dpy,
			   XRRScreenConfiguration *config,
			   Drawable draw,
			   int size_index,
			   Rotation rotation, Time timestamp)
{
    return XRRSetScreenConfigAndRate (dpy, config, draw, size_index,
				      rotation, 0, timestamp);
}
