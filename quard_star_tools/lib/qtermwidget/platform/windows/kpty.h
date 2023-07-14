/* This file is part of the KDE libraries

    Copyright (C) 2003,2007 Oswald Buddenhagen <ossi@kde.org>

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

#ifndef kpty_h
#define kpty_h

#include <QObject>

#include <windows.h>
#include <process.h>


//Taken from the RS5 Windows SDK, but redefined here in case we're targeting <= 17733
//Just for compile, ConPty doesn't work with Windows SDK < 17733
#ifndef PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE
#define PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE \
  ProcThreadAttributeValue(22, FALSE, TRUE, FALSE)

typedef VOID* HPCON;

#define TOO_OLD_WINSDK
#endif

//ConPTY available only on Windows 10 releazed after 1903 (19H1) Windows release
class WindowsContext
{
public:
    typedef HRESULT (*CreatePseudoConsolePtr)(
            COORD size,         // ConPty Dimensions
            HANDLE hInput,      // ConPty Input
            HANDLE hOutput,	    // ConPty Output
            DWORD dwFlags,      // ConPty Flags
            HPCON* phPC);       // ConPty Reference

    typedef HRESULT (*ResizePseudoConsolePtr)(HPCON hPC, COORD size);

    typedef VOID (*ClosePseudoConsolePtr)(HPCON hPC);

    WindowsContext()
        : createPseudoConsole(nullptr)
        , resizePseudoConsole(nullptr)
        , closePseudoConsole(nullptr)
    {

    }

    bool init()
    {
        //already initialized
        if (createPseudoConsole)
            return true;

        //try to load symbols from library
        //if it fails -> we can't use ConPty API
        HANDLE kernel32Handle = LoadLibraryExW(L"kernel32.dll", 0, 0);

        if (kernel32Handle != nullptr)
        {
            createPseudoConsole = (CreatePseudoConsolePtr)GetProcAddress((HMODULE)kernel32Handle, "CreatePseudoConsole");
            resizePseudoConsole = (ResizePseudoConsolePtr)GetProcAddress((HMODULE)kernel32Handle, "ResizePseudoConsole");
            closePseudoConsole = (ClosePseudoConsolePtr)GetProcAddress((HMODULE)kernel32Handle, "ClosePseudoConsole");
            if (createPseudoConsole == NULL || resizePseudoConsole == NULL || closePseudoConsole == NULL)
            {
                m_lastError = QString("WindowsContext/ConPty error: %1").arg("Invalid on load API functions");
                return false;
            }
        }
        else
        {
            m_lastError = QString("WindowsContext/ConPty error: %1").arg("Unable to load kernel32");
            return false;
        }

        return true;
    }

    QString lastError()
    {
        return m_lastError;
    }

public:
    //vars
    CreatePseudoConsolePtr createPseudoConsole;
    ResizePseudoConsolePtr resizePseudoConsole;
    ClosePseudoConsolePtr closePseudoConsole;

private:
    QString m_lastError;
};

class KPtyPrivate;

/**
 * Provides primitives for opening & closing a pseudo TTY pair, assigning the
 * controlling TTY, utmp registration and setting various terminal attributes.
 */
class KPty {
    Q_DECLARE_PRIVATE(KPty)

public:

    /**
     * Constructor
     */
    KPty();

    /**
     * Destructor:
     *
     *  If the pty is still open, it will be closed. Note, however, that
     *  an utmp registration is @em not undone.
    */
    ~KPty();

    KPty(const KPty &) = delete;
    KPty &operator=(const KPty &) = delete;

    /**
     * Create a pty master/slave pair.
     *
     * @return true if a pty pair was successfully opened
     */
    bool open();

    bool open(int fd);

    /**
     * Close the pty master/slave pair.
     */
    void close();

    /**
     * Change the logical (screen) size of the pty.
     * The default is 24 lines by 80 columns.
     *
     * This function can be used only while the PTY is open.
     *
     * @param lines the number of rows
     * @param columns the number of columns
     * @return @c true on success, false otherwise
     */
    bool setWinSize(int lines, int columns);

    /**
     * Set whether the pty should echo input.
     *
     * Echo is on by default.
     * If the output of automatically fed (non-interactive) PTY clients
     * needs to be parsed, disabling echo often makes it much simpler.
     *
     * This function can be used only while the PTY is open.
     *
     * @param echo true if input should be echoed.
     * @return @c true on success, false otherwise
     */
    bool setEcho(bool echo);
    bool getEcho(void) const;

    bool setFlowControlEnabled(bool enable);
    bool getFlowControlEnabled(void) const;

    bool setUtf8Mode(bool enable);
    bool getUtf8Mode(void) const;

    bool setErase(char c);
    char getErase(void) const;

    /**
     * @return the name of the slave pty device.
     *
     * This function should be called only while the pty is open.
     */
    const char * ttyName() const;

    /**
     * @return the file descriptor of the master pty
     *
     * This function should be called only while the pty is open.
     */
    int masterFd() const;

    /**
     * @return the file descriptor of the slave pty
     *
     * This function should be called only while the pty slave is open.
     */
    int slaveFd() const;

    int foregroundProcessGroup() const;

private:
    HRESULT createPseudoConsoleAndPipes(HPCON* phPC, HANDLE* phPipeIn, HANDLE* phPipeOut, qint16 cols, qint16 rows);
    HRESULT initializeStartupInfoAttachedToPseudoConsole(STARTUPINFOEX* pStartupInfo, HPCON hPC);

protected:
    /**
     * @internal
     */
    KPty(KPtyPrivate * d);

    /**
     * @internal
     */
    KPtyPrivate * const d_ptr;
};

#endif

