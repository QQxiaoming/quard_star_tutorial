/*
 * Copyright © 2000, Compaq Computer Corporation, 
 * Copyright © 2002, Hewlett Packard, Inc.
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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL HP
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Jim Gettys, HP Labs, Hewlett-Packard, Inc.
 */

#ifndef _XRANDRINT_H_
#define _XRANDRINT_H_

#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>
#include "Xrandr.h"
#include <X11/extensions/randr.h>
#include <X11/extensions/randrproto.h>

extern char XRRExtensionName[];

#define RRCheckExtension(dpy,i,val) \
  XextCheckExtension (dpy, i, XRRExtensionName, val)
#define RRSimpleCheckExtension(dpy,i) \
  XextSimpleCheckExtension (dpy, i, XRRExtensionName)

XExtDisplayInfo *XRRFindDisplay (Display *dpy);


/* deliberately opaque internal data structure; can be extended, 
   but not reordered */
struct _XRRScreenConfiguration {
  Screen *screen;	/* the root window in GetScreenInfo */
  XRRScreenSize *sizes;
  Rotation rotations;
  Rotation current_rotation;
  int nsizes;
  int current_size;
  short current_rate;
  Time timestamp;
  Time config_timestamp;
  int subpixel_order;	/* introduced in randr v0.1 */
  short *rates;		/* introduced in randr v1.1 */
  int nrates;
};

/*
 * if a configure notify on the root is recieved, or
 * an XRRScreenChangeNotify is recieved,
 * XRRUpdateConfiguration should be called to update the X library's
 * view of the screen configuration; it will also invalidate the cache
 * provided by XRRScreenConfig and XRRConfig, and force a round trip
 * when next used.  Returns invalid status if not an event type
 * the library routine understand.
 */
 
/* we cache one screen configuration/screen */

typedef struct _XRandRInfo {
  XRRScreenConfiguration **config;
  int major_version, minor_version;	/* major_version = -1 means we don't know */
  Bool has_rates;			/* Server supports refresh rates */
} XRandRInfo;

typedef struct _randrVersionState {
    unsigned long   version_seq;
    Bool	    error;
    int		    major_version;
    int		    minor_version;
} _XRRVersionState;

Bool
_XRRVersionHandler (Display	*dpy,
		    xReply	*rep,
		    char	*buf,
		    int		len,
		    XPointer    data);

_X_HIDDEN Bool
_XRRHasRates (int major, int minor);

#endif /* _XRANDRINT_H_ */
