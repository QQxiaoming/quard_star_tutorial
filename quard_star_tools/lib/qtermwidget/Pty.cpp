/*
 * This file is a part of QTerminal - http://gitorious.org/qterminal
 *
 * This file was un-linked from KDE and modified
 * by Maxim Bourmistrov <maxim@unixconn.com>
 *
 */

/*
	This file is part of Konsole, an X terminal.
	Copyright 1997,1998 by Lars Doelle <lars.doelle@on-line.de>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
	02110-1301  USA.
*/

// Own
#include "Pty.h"

// System
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <csignal>

// Qt
#include <QStringList>
#include <QtDebug>


using namespace Konsole;

Pty::Pty(QObject* parent)
		: KProcess(parent)
{
	_windowColumns = 0;
	_windowLines = 0;
	_eraseChar = 0;
	_xonXoff = true;
	_utf8 =true;
}

Pty::~Pty()
{
}

void Pty::setWindowSize(int lines, int cols)
{
	_windowColumns = cols;
	_windowLines = lines;
}

QSize Pty::windowSize() const
{
	return {_windowColumns,_windowLines};
}

void Pty::setFlowControlEnabled(bool enable)
{
	_xonXoff = enable;
}
bool Pty::flowControlEnabled() const
{
	return true;
}

void Pty::setUtf8Mode(bool enable)
{
	_utf8 = enable;
}

void Pty::setErase(char erase)
{
	_eraseChar = erase;
}

char Pty::erase() const
{
	return _eraseChar;
}

void Pty::addEnvironmentVariables(const QStringList& environment)
{
	QListIterator<QString> iter(environment);
	while (iter.hasNext())
	{
		QString pair = iter.next();

		// split on the first '=' character
		int pos = pair.indexOf(QLatin1Char('='));

		if ( pos >= 0 )
		{
			QString variable = pair.left(pos);
			QString value = pair.mid(pos+1);

			setEnv(variable,value);
		}
	}
}

int Pty::start(const QString& program,
							 const QStringList& programArguments,
							 const QStringList& environment,
                             ulong winid
							 )
{
	clearProgram();

	// For historical reasons, the first argument in programArguments is the
	// name of the program to execute, so create a list consisting of all
	// but the first argument to pass to setProgram()
	Q_ASSERT(programArguments.count() >= 1);
	setProgram(program, programArguments.mid(1));

	addEnvironmentVariables(environment);

	setEnv(QLatin1String("WINDOWID"), QString::number(winid));
	setEnv(QLatin1String("COLORTERM"), QLatin1String("truecolor"));

	// unless the LANGUAGE environment variable has been set explicitly
	// set it to a null string
	// this fixes the problem where KCatalog sets the LANGUAGE environment
	// variable during the application's startup to something which
	// differs from LANG,LC_* etc. and causes programs run from
	// the terminal to display messages in the wrong language
	//
	// this can happen if LANG contains a language which KDE
	// does not have a translation for
	//
	// BR:149300
	setEnv(QLatin1String("LANGUAGE"),QString(),false /* do not overwrite existing value if any */);

	KProcess::start();

	if (!waitForStarted())
		return -1;

	return 0;
}

int Pty::dataReceived(const char* data, int length)
{
	emit receivedData(data,length);
	return length;
}
