/*
 * This code is taken from the RPM package manager.
 *
 * RPM is Copyright (c) 1998 by Red Hat Software, Inc.,
 * and may be distributed under the terms of the GPL and LGPL.
 * See http://rpm.org/gitweb?p=rpm.git;a=blob_plain;f=COPYING;hb=HEAD
 *
 * The code should follow upstream as closely as possible.
 * See http://rpm.org/gitweb?p=rpm.git;a=blob_plain;f=lib/rpmlib.h;hb=HEAD
 */

/*
 * Segmented string compare for version or release strings.
 *
 * @param a		1st string
 * @param b		2nd string
 * @return		+1 if a is "newer", 0 if equal, -1 if b is "newer"
 */
int rpmvercmp(const char * a, const char * b);
