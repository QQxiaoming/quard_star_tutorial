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
#include <io.h>

#define CONPTY_MINIMAL_WINDOWS_VERSION 18309

///////////////////////
// private functions //
///////////////////////

//////////////////
// private data //
//////////////////

KPtyPrivate::KPtyPrivate(KPty* parent) :
        m_ptyHandler(INVALID_HANDLE_VALUE),
        m_hPipeIn(INVALID_HANDLE_VALUE), 
        m_hPipeOut(INVALID_HANDLE_VALUE), 
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

    qint32 buildNumber = QSysInfo::kernelVersion().split(".").last().toInt();
    if (buildNumber < CONPTY_MINIMAL_WINDOWS_VERSION) {
        qWarning() << "ConPty Error: Windows version is too old, please update to Windows 10 1809 or later.";
        abort();
    }

    d_ptr->m_winContext.init();
}

KPty::~KPty()
{
    close();
    delete d_ptr;
}

bool KPty::open()
{
    Q_D(KPty);

    if (d->m_ptyHandler != INVALID_HANDLE_VALUE)
        return true;

    d->ttyName = "Pseudo Console";

    // We try, as we know them, one by one.
    HRESULT hr{ E_UNEXPECTED };

    //  Create the Pseudo Console and pipes to it
    hr = createPseudoConsoleAndPipes(&d->m_ptyHandler, &d->m_hPipeIn, &d->m_hPipeOut, 80, 28);
    if (S_OK != hr)
    {
        qWarning() << "ConPty Error: CreatePseudoConsoleAndPipes fail";
        return false;
    }
    
    // Initialize the necessary startup info struct
    STARTUPINFOEX startupInfo{};
    if (S_OK != initializeStartupInfoAttachedToPseudoConsole(&startupInfo, d->m_ptyHandler))
    {
        qWarning() << "ConPty Error: InitializeStartupInfoAttachedToPseudoConsole fail";
        return false;
    }

    return true;
}

void KPty::close()
{
    Q_D(KPty);

    if (d->m_ptyHandler == INVALID_HANDLE_VALUE)
        return;
    
    d->m_winContext.closePseudoConsole(d->m_ptyHandler);

    // Clean-up the pipes
    if (INVALID_HANDLE_VALUE != d->m_hPipeOut) CloseHandle(d->m_hPipeOut);
    if (INVALID_HANDLE_VALUE != d->m_hPipeIn) CloseHandle(d->m_hPipeIn);
}

HRESULT KPty::createPseudoConsoleAndPipes(HPCON* phPC, HANDLE* phPipeIn, HANDLE* phPipeOut, qint16 cols, qint16 rows)
{
    Q_D(KPty);
    HRESULT hr{ E_UNEXPECTED };
    HANDLE hPipePTYIn{ INVALID_HANDLE_VALUE };
    HANDLE hPipePTYOut{ INVALID_HANDLE_VALUE };

    // Create the pipes to which the ConPTY will connect
    if (CreatePipe(&hPipePTYIn, phPipeOut, NULL, 0) &&
            CreatePipe(phPipeIn, &hPipePTYOut, NULL, 0))
    {
        // Create the Pseudo Console of the required size, attached to the PTY-end of the pipes
        hr = d->m_winContext.createPseudoConsole({cols, rows}, hPipePTYIn, hPipePTYOut, 0, phPC);

        // Note: We can close the handles to the PTY-end of the pipes here
        // because the handles are dup'ed into the ConHost and will be released
        // when the ConPTY is destroyed.
        if (INVALID_HANDLE_VALUE != hPipePTYOut) CloseHandle(hPipePTYOut);
        if (INVALID_HANDLE_VALUE != hPipePTYIn) CloseHandle(hPipePTYIn);
    }

    return hr;
}

// Initializes the specified startup info struct with the required properties and
// updates its thread attribute list with the specified ConPTY handle
HRESULT KPty::initializeStartupInfoAttachedToPseudoConsole(STARTUPINFOEX* pStartupInfo, HPCON hPC)
{
    Q_D(KPty);
    HRESULT hr{ E_UNEXPECTED };

    if (pStartupInfo)
    {
        SIZE_T attrListSize{};

        pStartupInfo->StartupInfo.cb = sizeof(STARTUPINFOEX);

        // Get the size of the thread attribute list.
        InitializeProcThreadAttributeList(NULL, 1, 0, &attrListSize);

        // Allocate a thread attribute list of the correct size
        pStartupInfo->lpAttributeList =
                reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(malloc(attrListSize));

        // Initialize thread attribute list
        if (pStartupInfo->lpAttributeList
                && InitializeProcThreadAttributeList(pStartupInfo->lpAttributeList, 1, 0, &attrListSize))
        {
            // Set Pseudo Console attribute
            hr = UpdateProcThreadAttribute(
                        pStartupInfo->lpAttributeList,
                        0,
                        PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
                        hPC,
                        sizeof(HPCON),
                        NULL,
                        NULL)
                    ? S_OK
                    : HRESULT_FROM_WIN32(GetLastError());
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    return hr;
}

bool KPty::setWinSize(int lines, int columns)
{
    qint16 cols = columns;
    qint16 rows = lines;
    
    Q_D(KPty);

    if (d->m_ptyHandler == nullptr)
    {
        return false;
    }

    bool res = SUCCEEDED(d->m_winContext.resizePseudoConsole(d->m_ptyHandler, {cols, rows}));

    return res;
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
    return 0x10;
}

const char * KPty::ttyName() const
{
    Q_D(const KPty);

    return d->ttyName.data();
}

int KPty::masterFd() const
{
    Q_D(const KPty);
    if(d->m_hPipeIn != INVALID_HANDLE_VALUE)
        return _open_osfhandle((intptr_t)d->m_hPipeIn, _O_RDONLY);
    else 
        return -1;
}

int KPty::slaveFd() const
{
    Q_D(const KPty);

    if(d->m_hPipeOut != INVALID_HANDLE_VALUE)
        return _open_osfhandle((intptr_t)d->m_hPipeOut, _O_WRONLY);
    else 
        return -1;
}

int KPty::foregroundProcessGroup() const
{
    return 0;
}
