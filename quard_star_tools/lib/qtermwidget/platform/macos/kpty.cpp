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

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/param.h>

#include <cerrno>
#include <fcntl.h>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <grp.h>

#include <util.h>
#include <utmp.h>
#include <utmpx.h>

extern "C" {
#include <termios.h>
}


#define _tcgetattr(fd, ttmode) ioctl(fd, TIOCGETA, (char *)ttmode)
#define _tcsetattr(fd, ttmode) ioctl(fd, TIOCSETA, (char *)ttmode)

#define TTY_GROUP "tty"

///////////////////////
// private functions //
///////////////////////

//////////////////
// private data //
//////////////////

KPtyPrivate::KPtyPrivate(KPty* parent) :
        masterFd(-1), slaveFd(-1), ownMaster(true), q_ptr(parent)
{
}

KPtyPrivate::~KPtyPrivate()
{
}

bool KPtyPrivate::chownpty(bool)
{
//    return !QProcess::execute(KStandardDirs::findExe("kgrantpty"),
//        QStringList() << (grant?"--grant":"--revoke") << QString::number(masterFd));
    return true;
}

/////////////////////////////
// public member functions //
/////////////////////////////

KPty::KPty() :
        d_ptr(new KPtyPrivate(this))
{
}

KPty::KPty(KPtyPrivate *d) :
        d_ptr(d)
{
    d_ptr->q_ptr = this;
}

KPty::~KPty()
{
    close();
    delete d_ptr;
}

bool KPty::open()
{
    Q_D(KPty);

    if (d->masterFd >= 0)
        return true;

    d->ownMaster = true;

    // Find a master pty that we can open ////////////////////////////////

    // Because not all the pty animals are created equal, they want to
    // be opened by several different methods.

    // We try, as we know them, one by one.

    char ptsn[PATH_MAX];
    if (::openpty( &d->masterFd, &d->slaveFd, ptsn, 0, 0)) {
        d->masterFd = -1;
        d->slaveFd = -1;
        qWarning() << "Can't open a pseudo teletype";
        return false;
    }
    d->ttyName = ptsn;


    fcntl(d->masterFd, F_SETFD, FD_CLOEXEC);
    fcntl(d->slaveFd, F_SETFD, FD_CLOEXEC);

    return true;
}

bool KPty::open(int fd)
{
     qWarning() << "Unsupported attempt to open pty with fd" << fd;
     return false;
}

void KPty::closeSlave()
{
    Q_D(KPty);

    if (d->slaveFd < 0) {
        return;
    }
    ::close(d->slaveFd);
    d->slaveFd = -1;
}

bool KPty::openSlave()
{
    Q_D(KPty);

    if (d->slaveFd >= 0)
	return true;
    if (d->masterFd < 0) {
	qDebug() << "Attempting to open pty slave while master is closed";
	return false;
    }
    //d->slaveFd = KDE_open(d->ttyName.data(), O_RDWR | O_NOCTTY);
    d->slaveFd = ::open(d->ttyName.data(), O_RDWR | O_NOCTTY);
    if (d->slaveFd < 0) {
	qDebug() << "Can't open slave pseudo teletype";
	return false;
    }
    fcntl(d->slaveFd, F_SETFD, FD_CLOEXEC);
    return true;
}

void KPty::close()
{
    Q_D(KPty);

    if (d->masterFd < 0) {
        return;
    }
    closeSlave();
    // don't bother resetting unix98 pty, it will go away after closing master anyway.
    if (memcmp(d->ttyName.data(), "/dev/pts/", 9)) {
        if (!geteuid()) {
            struct stat st;
            if (!stat(d->ttyName.data(), &st)) {
                int f = chown(d->ttyName.data(), 0, st.st_gid == getgid() ? 0 : -1);
                Q_UNUSED(f);
                chmod(d->ttyName.data(), S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
            }
        } else {
            fcntl(d->masterFd, F_SETFD, 0);
            d->chownpty(false);
        }
    }
    ::close(d->masterFd);
    d->masterFd = -1;
}

void KPty::setCTty()
{
    Q_D(KPty);

    // Setup job control //////////////////////////////////

    // Become session leader, process group leader,
    // and get rid of the old controlling terminal.
    setsid();

    // make our slave pty the new controlling terminal.
    ioctl(d->slaveFd, TIOCSCTTY, 0);

    // make our new process group the foreground group on the pty
    int pgrp = getpid();
    tcsetpgrp(d->slaveFd, pgrp);
}

void KPty::login(const char * user, const char * remotehost)
{
    struct utmpx l_struct;

    memset(&l_struct, 0, sizeof(l_struct));
    // note: strncpy without terminators _is_ correct here. man 4 utmp

    if (user) {
        strncpy(l_struct.ut_user, user, sizeof(l_struct.ut_user));
    }

    if (remotehost) {
        strncpy(l_struct.ut_host, remotehost, sizeof(l_struct.ut_host));
    }

    Q_D(KPty);
    const char * str_ptr = d->ttyName.data();
    if (!memcmp(str_ptr, "/dev/", 5)) {
        str_ptr += 5;
    }
    strncpy(l_struct.ut_line, str_ptr, sizeof(l_struct.ut_line));

    gettimeofday(&l_struct.ut_tv, 0);

    utmpxname(_PATH_UTMPX);
    setutxent();
    pututxline(&l_struct);
    endutxent();
}

void KPty::logout()
{
    Q_D(KPty);

    const char *str_ptr = d->ttyName.data();
    if (!memcmp(str_ptr, "/dev/", 5)) {
        str_ptr += 5;
    }

    struct utmpx l_struct, *ut;

    memset(&l_struct, 0, sizeof(l_struct));

    strncpy(l_struct.ut_line, str_ptr, sizeof(l_struct.ut_line));

    utmpxname(_PATH_UTMPX);
    setutxent();
    if ((ut = getutxline(&l_struct))) {

        memset(ut->ut_user, 0, sizeof(*ut->ut_user));
        memset(ut->ut_host, 0, sizeof(*ut->ut_host));

        gettimeofday(&ut->ut_tv, 0);
        pututxline(ut);
    }
    endutxent();
}

// XXX Supposedly, tc[gs]etattr do not work with the master on Solaris.
// Please verify.

bool KPty::tcGetAttr(struct ::termios * ttmode) const
{
    Q_D(const KPty);

    return _tcgetattr(d->masterFd, ttmode) == 0;
}

bool KPty::tcSetAttr(struct ::termios * ttmode)
{
    Q_D(KPty);

    return _tcsetattr(d->masterFd, ttmode) == 0;
}

bool KPty::setWinSize(int lines, int columns)
{
    Q_D(KPty);

    struct winsize winSize;
    memset(&winSize, 0, sizeof(winSize));
    winSize.ws_row = (unsigned short)lines;
    winSize.ws_col = (unsigned short)columns;
    return ioctl(d->masterFd, TIOCSWINSZ, (char *)&winSize) != -1;
}

bool KPty::setEcho(bool echo)
{
    struct ::termios ttmode;
    if (!tcGetAttr(&ttmode)) {
        return false;
    }
    if (!echo) {
        ttmode.c_lflag &= ~ECHO;
    } else {
        ttmode.c_lflag |= ECHO;
    }
    if (!tcSetAttr(&ttmode)) {
        return false;
    } else {
        return true;
    }
}

bool KPty::getEcho(void) const
{
    struct ::termios ttmode;
    if (!tcGetAttr(&ttmode)) {
        return false;
    }
    return (ttmode.c_iflag & ECHO) == ECHO;
}

bool KPty::setFlowControlEnabled(bool enable)
{
    struct ::termios ttmode;
    if (!tcGetAttr(&ttmode)) {
        return false;
    }
    if (!enable) {
        ttmode.c_iflag &= ~(IXOFF | IXON);
    } else {
        ttmode.c_iflag |= (IXOFF | IXON);
    }
    if (!tcSetAttr(&ttmode)) {
        return false;
    } else {
        return true;
    }
}

bool KPty::getFlowControlEnabled(void) const
{
    struct ::termios ttmode;
    if (!tcGetAttr(&ttmode)) {
        return false;
    }
    return (ttmode.c_iflag & (IXOFF | IXON)) == (IXOFF | IXON);
}

bool KPty::setUtf8Mode(bool enable)
{
#ifdef IUTF8
    struct ::termios ttmode;
    if (!tcGetAttr(&ttmode)) {
        return false;
    }
    if (!enable) {
        ttmode.c_iflag &= ~IUTF8;
    } else {
        ttmode.c_iflag |= IUTF8;
    }
    if (!tcSetAttr(&ttmode)) {
        return false;
    } else {
        return true;
    }
#else
    Q_UNUSED(enable);
    return false;
#endif
}

bool KPty::getUtf8Mode(void) const
{
#ifdef IUTF8
    struct ::termios ttmode;
    if (!tcGetAttr(&ttmode)) {
        return false;
    }
    return (ttmode.c_iflag & IUTF8) == IUTF8;
#else
    return false;
#endif
}

bool KPty::setErase(char c)
{
    struct ::termios ttmode;
    if (!tcGetAttr(&ttmode)) {
        return false;
    }
    ttmode.c_cc[VERASE] = c;
    if (!tcSetAttr(&ttmode)) {
        return false;
    } else {
        return true;
    }
}

char KPty::getErase(void) const
{
    struct ::termios ttmode;
    if (!tcGetAttr(&ttmode)) {
        return '\0';
    }
    return ttmode.c_cc[VERASE];
}

const char * KPty::ttyName() const
{
    Q_D(const KPty);

    return d->ttyName.data();
}

int KPty::masterFd() const
{
    Q_D(const KPty);

    return d->masterFd;
}

int KPty::slaveFd() const
{
    Q_D(const KPty);

    return d->slaveFd;
}

int KPty::foregroundProcessGroup() const
{
    int pid = tcgetpgrp(masterFd());

    if ( pid != -1 )
    {
        return pid;
    }

    return 0;
}
