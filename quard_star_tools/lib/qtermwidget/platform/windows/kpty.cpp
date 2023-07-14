/*

   This file is part of the KDE libraries
   Copyright (C) 2002 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2002-2003,2007 Oswald Buddenhagen <ossi@kde.org>

    Rewritten for QT4 by e_k <e_k at users.sourceforge.net>, Copyright (C)2008

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kpty_p.h"

#include <QtDebug>

#include <windows.h>

#include <cerrno>
#include <fcntl.h>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>

KPtyPrivate::KPtyPrivate(KPty* parent) :
        masterFd(-1), slaveFd(-1), ownMaster(true), q_ptr(parent)
{
}

KPtyPrivate::~KPtyPrivate()
{
}

bool KPtyPrivate::chownpty(bool)
{
    return true;
}

KPty::KPty() :
        d_ptr(new KPtyPrivate(this))
{
}

KPty::KPty(KPtyPrivate *d) :
        d_ptr(d)
{
}

KPty::~KPty()
{
}

bool KPty::open()
{
    return true;
}

bool KPty::open(int fd)
{
    return true;
}

void KPty::closeSlave()
{

}

bool KPty::openSlave()
{
    return true;
}

void KPty::close()
{

}

void KPty::setCTty()
{

}

void KPty::login(const char * user, const char * remotehost)
{

}

void KPty::logout()
{

}

bool KPty::tcGetAttr(struct ::termios * ttmode) const
{
    return false;
}

bool KPty::tcSetAttr(struct ::termios * ttmode)
{
    return false;
}

bool KPty::setWinSize(int lines, int columns)
{
    return false;
}

bool KPty::setEcho(bool echo)
{
    return false;
}

bool KPty::getEcho(void) const
{
    return false;
}

bool KPty::setFlowControlEnabled(bool enable)
{
    return false;
}

bool KPty::getFlowControlEnabled(void) const
{
    return false;
}

bool KPty::setUtf8Mode(bool enable)
{
    return false;
}

bool KPty::getUtf8Mode(void) const
{
    return false;
}

bool KPty::setErase(char c)
{
    return false;
}

char KPty::getErase(void) const
{
    return '\0';
}

const char * KPty::ttyName() const
{
    return nullptr;
}

int KPty::masterFd() const
{
    return 0;
}

int KPty::slaveFd() const
{
    return 0;
}

int KPty::foregroundProcessGroup() const
{
    return 0;
}
