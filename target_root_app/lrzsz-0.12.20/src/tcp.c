/*
  tcp.c - tcp handling for lrzsz
  Copyright (C) 1997 Uwe Ohse

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

  originally written by Uwe Ohse
*/

#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <signal.h>

#include "zglobal.h"
#include <stdlib.h>
#include "error.h"

static RETSIGTYPE
tcp_alarm_handler(int dummy LRZSZ_ATTRIB_UNUSED)
{
    /* doesn't need to do anything */
}


/* server/lsz:
 * Get a TCP socket, bind it, listen, figure out the port,
 * and build the magic string for lrz in "buf".
 */
int 
tcp_server (char *buf)
{
	int sock;
	struct sockaddr_in s;
	struct sockaddr_in t;
	int on=1;
	size_t len;

	if ((sock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		error(1,errno,"socket");
	}
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof (on)) < 0) {
		error(1,errno,"setsockopt (reuse address)");
	}
	memset (&s, 0, sizeof (s));
	s.sin_family = AF_INET;
	s.sin_port=0; /* let system fill it in */
	s.sin_addr.s_addr=htonl(INADDR_ANY);
	if (bind(sock, (struct sockaddr *)&s, sizeof (s)) < 0) {
		error(1,errno,"bind");
	}
	len=sizeof(t);
	if (getsockname (sock, (struct sockaddr *) &t, &len)) {
		error(1,errno,"getsockname");
	}
	sprintf(buf,"[%s] <%d>\n",inet_ntoa(t.sin_addr),ntohs(t.sin_port));

	if (listen(sock, 1) < 0) {
		error(1,errno,"listen");
	}
	getsockname (sock, (struct sockaddr *) &t, &len);

	return (sock);
}

/* server/lsz: accept a connection */
int 
tcp_accept (int d)
{
	int so;
	struct  sockaddr_in s;
	size_t namelen;
	int num=0;

	namelen = sizeof(s);
	memset((char*)&s,0, namelen);

retry:
	signal(SIGALRM, tcp_alarm_handler);
	alarm(30);
	if ((so = accept(d, (struct sockaddr*)&s, &namelen)) < 0) {
		if (errno == EINTR) {
			if (++num<=5)
				goto retry;
		}
		error(1,errno,"accept");
	}
	alarm(0);
	return so;
}

/* client/lrz:
 * "Connect" to the TCP socket decribed in "buf" and
 * return the connected socket.
 */
int 
tcp_connect (char *buf)
{
	int sock;
	struct sockaddr_in s_in;
	char *p;
	char *q;

	memset(&s_in,0,sizeof(s_in));
	s_in.sin_family = AF_INET;

	/* i _really_ distrust scanf & co. Or maybe i distrust bad input */
	if (*buf!='[') {
		error(1,0,_("tcp_connect: illegal format1\n"));
	}
	p=strchr(buf+1,']');
	if (!p) {
		error(1,0,_("tcp_connect: illegal format2\n"));
	}
	*p++=0;
	s_in.sin_addr.s_addr=inet_addr(buf+1);
#ifndef INADDR_NONE
#define INADDR_NONE (-1)
#endif
	if (s_in.sin_addr.s_addr== (unsigned long) INADDR_NONE) {
		struct hostent *h=gethostbyname(buf+1);
		if (!h)
			error(1,0,_("tcp_connect: illegal format3\n"));
		memcpy(& s_in.sin_addr.s_addr,h->h_addr,h->h_length);
	}
	while (isspace((unsigned char)(*p)))
		p++;
	if (*p!='<') {
		error(1,0,_("tcp_connect: illegal format4\n"));
	}
	q=strchr(p+1,'>');
	if (!q)
		error(1,0,_("tcp_connect: illegal format5\n"));
	s_in.sin_port = htons(strtol(p+1,NULL,10));

	if ((sock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		error(1,errno,"socket");
	}

	signal(SIGALRM, tcp_alarm_handler);
	alarm(30);
	if (connect (sock, (struct sockaddr *) &s_in, sizeof (s_in)) < 0) {
		error(1,errno,"connect");
	}
	alarm(0);
	return (sock);
}
