#ifndef _VERSION_H
#define _VERSION_H

#define VERSION(a, b) (((a) << 8) + (b))

#ifdef BUILDGUI
#include "ks_version.h"
#else
#include "tc_version.h"
#endif

#endif /* _VERSION_H */
