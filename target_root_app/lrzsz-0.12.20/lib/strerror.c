/* strerror.c -- strerror() replacement
   Copyright (C) 1996 Uwe Ohse

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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

char *
strerror(num)
	int num;
{
	extern char *sys_errlist[];
	extern int sys_nerr;

	if (num < 0 || num > sys_nerr)
		return "Unknown system error / illegal error number";

	return sys_errlist[num];
}
