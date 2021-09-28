/*
  protname.c - return the name of the protocol used
  Copyright (C) 1996, 1997 Uwe Ohse

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
  02111-1307, USA.

*/
#include "zglobal.h"

/* this code was duplicate in lrz.c and lsz.c */

const char *
protname(void)
{
	const char *prot_name;
	switch(protocol) {
	case ZM_XMODEM:
		prot_name="XMODEM"; 
		break;
	case ZM_YMODEM:
		prot_name="YMODEM"; 
		break;
	default: 
		prot_name="ZMODEM";
		break;
	}
	return prot_name;
}
