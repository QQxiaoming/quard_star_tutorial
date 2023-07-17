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

#include <cerrno>
#include <fcntl.h>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>


///////////////////////
// private functions //
///////////////////////

//////////////////
// private data //
//////////////////

KPtyPrivate::KPtyPrivate(KPty* parent) :
        q_ptr(parent)
{
}

KPtyPrivate::~KPtyPrivate()
{
}

bool KPtyPrivate::chownpty(bool)
{
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

    //static int _index = 0;
    //server = new QLocalServer();
    //serverName = "myserver" + QString::number(_index++);
    //QLocalServer::removeServer(serverName);
    //if (!server->listen(serverName)) {
    //    qDebug() << "Failed to start server" << serverName;
    //}
    //QLocalSocket::connect(server, &QLocalServer::newConnection, [=]() {
    //    QLocalSocket* socket = server->nextPendingConnection();
    //    QLocalSocket::connect(socket, &QLocalSocket::readyRead, [=]() {
    //        QByteArray data = socket->readAll();
    //        socket->write(data);
    //        //socket->flush();
    //    });
    //});
    //socket = new QLocalSocket();
    //socket->setServerName(serverName);
    //socket->connectToServer();

    return true;
}

void KPty::close()
{
    //Q_D(KPty);
    //socket->close();
    //server->close();
    //QLocalServer::removeServer(serverName);
    //delete server;
    //delete socket;
}

bool KPty::setWinSize(int lines, int columns)
{
    Q_D(KPty);

    return true;
}

bool KPty::setEcho(bool echo)
{
    m_echo = echo;
    return true;
}

bool KPty::getEcho(void) const
{
    return m_echo;
}

bool KPty::setFlowControlEnabled(bool enable)
{
    m_fce = enable;
    return true;
}

bool KPty::getFlowControlEnabled(void) const
{
    return m_fce;
}

bool KPty::setUtf8Mode(bool enable)
{
    m_utf8mode = enable;
    return true;
}

bool KPty::getUtf8Mode(void) const
{
    return m_utf8mode;
}

bool KPty::setErase(char c)
{
    m_erase = c;
    return true;
}

char KPty::getErase(void) const
{
    return m_erase;
}

const char * KPty::ttyName() const
{
    Q_D(const KPty);

    return d->ttyName.data();
}

int KPty::masterFd() const
{
    Q_D(const KPty);
    return 1001;
}

int KPty::slaveFd() const
{
    Q_D(const KPty);
    return 1002;
}

int KPty::foregroundProcessGroup() const
{
    return 0;
}

int KPty::writeSlaveFd(const char *buff, int len) const
{
    QString str = QString::fromUtf8(buff,len);
    str.replace("\n", "\r\n"); 
    return socket->write(str.toUtf8().data(), str.toUtf8().length());
}
