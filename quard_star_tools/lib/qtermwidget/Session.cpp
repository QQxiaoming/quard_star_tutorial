/*
    This file is part of Konsole

    Copyright (C) 2006-2007 by Robert Knight <robertknight@gmail.com>
    Copyright (C) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>

    Rewritten for QT4 by e_k <e_k at users.sourceforge.net>, Copyright (C)2008

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
#include "Session.h"

// Standard
#include <cstdlib>

// Qt
#include <QApplication>
#if QT_VERSION < 0x060000
#include <QRegExp>
#include <QByteRef>
#else
#include <QRegularExpression>
#endif
#include <QDir>
#include <QFile>
#include <QStringList>
#include <QFile>
#include <QtDebug>

#include "TerminalDisplay.h"
#include "ShellCommand.h"
#include "Vt102Emulation.h"

using namespace Konsole;

int Session::lastSessionId = 0;

Session::Session(QObject* parent) :
          QObject(parent)
        , _emulation(nullptr)
        , _monitorActivity(false)
        , _monitorSilence(false)
        , _notifiedActivity(false)
        , _autoClose(true)
        , _wantedClose(false)
        , _silenceSeconds(10)
        , _isTitleChanged(false)
        , _flowControl(true)
        , _fullScripting(false)
        , _sessionId(0)
        , _hasDarkBackground(false)
{
    _sessionId = ++lastSessionId;

    //create emulation backend
    _emulation = new Vt102Emulation();

    connect( _emulation, SIGNAL( titleChanged( int, const QString & ) ),
             this, SLOT( setUserTitle( int, const QString & ) ) );
    connect( _emulation, SIGNAL( stateSet(int) ),
             this, SLOT( activityStateSet(int) ) );
    connect( _emulation, SIGNAL( changeTabTextColorRequest( int ) ),
             this, SIGNAL( changeTabTextColorRequest( int ) ) );
    connect( _emulation, SIGNAL(profileChangeCommandReceived(const QString &)),
             this, SIGNAL( profileChangeCommandReceived(const QString &)) );

    connect(_emulation, SIGNAL(imageResizeRequest(QSize)),
            this, SLOT(onEmulationSizeChange(QSize)));
    connect(_emulation, SIGNAL(imageSizeChanged(int, int)),
            this, SLOT(onViewSizeChange(int, int)));
    connect(_emulation, &Vt102Emulation::cursorChanged,
            this, &Session::cursorChanged);


    //setup timer for monitoring session activity
    _monitorTimer = new QTimer(this);
    _monitorTimer->setSingleShot(true);
    connect(_monitorTimer, SIGNAL(timeout()), this, SLOT(monitorTimerDone()));
}

WId Session::windowId() const
{
    // On Qt5, requesting window IDs breaks QQuickWidget and the likes,
    // for example, see the following bug reports:
    // https://bugreports.qt.io/browse/QTBUG-40765
    // https://codereview.qt-project.org/#/c/94880/
    return 0;
}

void Session::setDarkBackground(bool darkBackground)
{
    _hasDarkBackground = darkBackground;
}

bool Session::hasDarkBackground() const
{
    return _hasDarkBackground;
}

void Session::setCodec(QTextCodec * codec) const
{
    emulation()->setCodec(codec);
}

void Session::setProgram(const QString & program)
{
    _program = ShellCommand::expand(program);
}
void Session::setInitialWorkingDirectory(const QString & dir)
{
    _initialWorkingDir = ShellCommand::expand(dir);
}
void Session::setArguments(const QStringList & arguments)
{
    _arguments = ShellCommand::expand(arguments);
}

QList<TerminalDisplay *> Session::views() const
{
    return _views;
}

void Session::addView(TerminalDisplay * widget)
{
    Q_ASSERT( !_views.contains(widget) );

    _views.append(widget);

    if ( _emulation != nullptr ) {
        // connect emulation - view signals and slots
        connect( widget , &TerminalDisplay::keyPressedSignal, _emulation ,
                 &Emulation::sendKeyEvent);
        connect( widget , SIGNAL(mouseSignal(int,int,int,int)) , _emulation ,
                 SLOT(sendMouseEvent(int,int,int,int)) );
        connect( widget , SIGNAL(sendStringToEmu(const char *)) , _emulation ,
                 SLOT(sendString(const char *)) );

        // allow emulation to notify view when the foreground process
        // indicates whether or not it is interested in mouse signals
        connect( _emulation , SIGNAL(programUsesMouseChanged(bool)) , widget ,
                 SLOT(setUsesMouse(bool)) );

        widget->setUsesMouse( _emulation->programUsesMouse() );

        connect( _emulation , SIGNAL(programBracketedPasteModeChanged(bool)) ,
                 widget , SLOT(setBracketedPasteMode(bool)) );

        widget->setBracketedPasteMode(_emulation->programBracketedPasteMode());

        widget->setScreenWindow(_emulation->createWindow());
    }

    //connect view signals and slots
    QObject::connect( widget ,SIGNAL(changedContentSizeSignal(int,int)),this,
                      SLOT(onViewSizeChange(int,int)));

    QObject::connect( widget ,SIGNAL(destroyed(QObject *)) , this ,
                      SLOT(viewDestroyed(QObject *)) );
//slot for close
    QObject::connect(this, SIGNAL(finished()), widget, SLOT(close()));

}

void Session::viewDestroyed(QObject * view)
{
    TerminalDisplay * display = (TerminalDisplay *)view;

    Q_ASSERT( _views.contains(display) );

    removeView(display);
}

void Session::removeView(TerminalDisplay * widget)
{
    _views.removeAll(widget);

    disconnect(widget,nullptr,this,nullptr);

    if ( _emulation != nullptr ) {
        // disconnect
        //  - key presses signals from widget
        //  - mouse activity signals from widget
        //  - string sending signals from widget
        //
        //  ... and any other signals connected in addView()
        disconnect( widget, nullptr, _emulation, nullptr);

        // disconnect state change signals emitted by emulation
        disconnect( _emulation , nullptr , widget , nullptr);
    }

    // close the session automatically when the last view is removed
    if ( _views.count() == 0 ) {
        close();
    }
}

void Session::run()
{
    emit started();
}

void Session::runEmptyPTY()
{
    emit started();
}

void Session::setUserTitle( int what, const QString & caption )
{
    //set to true if anything is actually changed (eg. old _nameTitle != new _nameTitle )
    bool modified = false;

    // (btw: what=0 changes _userTitle and icon, what=1 only icon, what=2 only _nameTitle
    if ((what == 0) || (what == 2)) {
        _isTitleChanged = true;
        if ( _userTitle != caption ) {
            _userTitle = caption;
            modified = true;
        }
    }

    if ((what == 0) || (what == 1)) {
        _isTitleChanged = true;
        if ( _iconText != caption ) {
            _iconText = caption;
            modified = true;
        }
    }

    if (what == 11) {
        QString colorString = caption.section(QLatin1Char(';'),0,0);
        //qDebug() << __FILE__ << __LINE__ << ": setting background colour to " << colorString;
        QColor backColor = QColor(colorString);
        if (backColor.isValid()) { // change color via \033]11;Color\007
            if (backColor != _modifiedBackground) {
                _modifiedBackground = backColor;

                // bail out here until the code to connect the terminal display
                // to the changeBackgroundColor() signal has been written
                // and tested - just so we don't forget to do this.
                Q_ASSERT( 0 );

                emit changeBackgroundColorRequest(backColor);
            }
        }
    }

    if (what == 30) {
        _isTitleChanged = true;
        if ( _nameTitle != caption ) {
            setTitle(Session::NameRole,caption);
            return;
        }
    }

    if (what == 31) {
        QString cwd=caption;
#if QT_VERSION >= 0x060000
        cwd=cwd.replace( QRegularExpression(QLatin1String("^~")), QDir::homePath() );
#else
        cwd=cwd.replace( QRegExp(QLatin1String("^~")), QDir::homePath() );
#endif
        emit openUrlRequest(cwd);
    }

    // change icon via \033]32;Icon\007
    if (what == 32) {
        _isTitleChanged = true;
        if ( _iconName != caption ) {
            _iconName = caption;

            modified = true;
        }
    }

    if (what == 50) {
        emit profileChangeCommandReceived(caption);
        return;
    }

    if ( modified ) {
        emit titleChanged();
    }
}

QString Session::userTitle() const
{
    return _userTitle;
}
void Session::setTabTitleFormat(TabTitleContext context , const QString & format)
{
    if ( context == LocalTabTitle ) {
        _localTabTitleFormat = format;
    } else if ( context == RemoteTabTitle ) {
        _remoteTabTitleFormat = format;
    }
}
QString Session::tabTitleFormat(TabTitleContext context) const
{
    if ( context == LocalTabTitle ) {
        return _localTabTitleFormat;
    } else if ( context == RemoteTabTitle ) {
        return _remoteTabTitleFormat;
    }

    return QString();
}

void Session::monitorTimerDone()
{
    //FIXME: The idea here is that the notification popup will appear to tell the user than output from
    //the terminal has stopped and the popup will disappear when the user activates the session.
    //
    //This breaks with the addition of multiple views of a session.  The popup should disappear
    //when any of the views of the session becomes active


    //FIXME: Make message text for this notification and the activity notification more descriptive.
    if (_monitorSilence) {
        emit silence();
        emit stateChanged(NOTIFYSILENCE);
    } else {
        emit stateChanged(NOTIFYNORMAL);
    }

    _notifiedActivity=false;
}

void Session::activityStateSet(int state)
{
    if (state==NOTIFYBELL) {
        emit bellRequest(tr("Bell in session '%1'").arg(_nameTitle));
    } else if (state==NOTIFYACTIVITY) {
        if (_monitorSilence) {
            _monitorTimer->start(_silenceSeconds*1000);
        }

        if ( _monitorActivity ) {
            //FIXME:  See comments in Session::monitorTimerDone()
            if (!_notifiedActivity) {
                _notifiedActivity=true;
                emit activity();
            }
        }
    }

    if ( state==NOTIFYACTIVITY && !_monitorActivity ) {
        state = NOTIFYNORMAL;
    }
    if ( state==NOTIFYSILENCE && !_monitorSilence ) {
        state = NOTIFYNORMAL;
    }

    emit stateChanged(state);
}

void Session::onViewSizeChange(int /*height*/, int /*width*/)
{
    updateTerminalSize();
}
void Session::onEmulationSizeChange(QSize size)
{
    setSize(size);
}

void Session::updateTerminalSize()
{
    QListIterator<TerminalDisplay *> viewIter(_views);

    int minLines = -1;
    int minColumns = -1;

    // minimum number of lines and columns that views require for
    // their size to be taken into consideration ( to avoid problems
    // with new view widgets which haven't yet been set to their correct size )
    const int VIEW_LINES_THRESHOLD = 2;
    const int VIEW_COLUMNS_THRESHOLD = 2;

    //select largest number of lines and columns that will fit in all visible views
    while ( viewIter.hasNext() ) {
        TerminalDisplay * view = viewIter.next();
        if ( view->isHidden() == false &&
                view->lines() >= VIEW_LINES_THRESHOLD &&
                view->columns() >= VIEW_COLUMNS_THRESHOLD ) {
            minLines = (minLines == -1) ? view->lines() : qMin( minLines , view->lines() );
            minColumns = (minColumns == -1) ? view->columns() : qMin( minColumns , view->columns() );
        }
    }

    // backend emulation must have a _terminal of at least 1 column x 1 line in size
    if ( minLines > 0 && minColumns > 0 ) {
        _emulation->setImageSize( minLines , minColumns );
    }
}

void Session::refresh()
{
    // attempt to get the shell process to redraw the display
    //
    // this requires the program running in the shell
    // to cooperate by sending an update in response to
    // a window size change
    //
    // the window size is changed twice, first made slightly larger and then
    // resized back to its normal size so that there is actually a change
    // in the window size (some shells do nothing if the
    // new and old sizes are the same)
    //
    // if there is a more 'correct' way to do this, please
    // send an email with method or patches to konsole-devel@kde.org
}

bool Session::sendHangUp(void)
{
    return true;
}

void Session::close()
{
    _autoClose = true;
    _wantedClose = true;
}

void Session::sendText(const QString & text) const
{
    _emulation->sendText(text);
}

void Session::sendKeyEvent(QKeyEvent* e) const
{
    _emulation->sendKeyEvent(e, false);
}

Session::~Session()
{
    delete _emulation;
}

void Session::setProfileKey(const QString & key)
{
    _profileKey = key;
    emit profileChanged(key);
}
QString Session::profileKey() const
{
    return _profileKey;
}

Emulation * Session::emulation() const
{
    return _emulation;
}

QString Session::keyBindings() const
{
    return _emulation->keyBindings();
}

QStringList Session::environment() const
{
    return _environment;
}

void Session::setEnvironment(const QStringList & environment)
{
    _environment = environment;
}

int Session::sessionId() const
{
    return _sessionId;
}

void Session::setKeyBindings(const QString & id)
{
    _emulation->setKeyBindings(id);
}

void Session::setTitle(TitleRole role , const QString & newTitle)
{
    if ( title(role) != newTitle ) {
        if ( role == NameRole ) {
            _nameTitle = newTitle;
        } else if ( role == DisplayedTitleRole ) {
            _displayTitle = newTitle;
        }

        emit titleChanged();
    }
}

QString Session::title(TitleRole role) const
{
    if ( role == NameRole ) {
        return _nameTitle;
    } else if ( role == DisplayedTitleRole ) {
        return _displayTitle;
    } else {
        return QString();
    }
}

void Session::setIconName(const QString & iconName)
{
    if ( iconName != _iconName ) {
        _iconName = iconName;
        emit titleChanged();
    }
}

void Session::setIconText(const QString & iconText)
{
    _iconText = iconText;
    //kDebug(1211)<<"Session setIconText " <<  _iconText;
}

QString Session::iconName() const
{
    return _iconName;
}

QString Session::iconText() const
{
    return _iconText;
}

bool Session::isTitleChanged() const
{
    return _isTitleChanged;
}

void Session::setHistoryType(const HistoryType & hType)
{
    _emulation->setHistory(hType);
}

const HistoryType & Session::historyType() const
{
    return _emulation->history();
}

void Session::clearHistory()
{
    _emulation->clearHistory();
}

QStringList Session::arguments() const
{
    return _arguments;
}

QString Session::program() const
{
    return _program;
}

// unused currently
bool Session::isMonitorActivity() const
{
    return _monitorActivity;
}
// unused currently
bool Session::isMonitorSilence()  const
{
    return _monitorSilence;
}

void Session::setMonitorActivity(bool _monitor)
{
    _monitorActivity=_monitor;
    _notifiedActivity=false;

    activityStateSet(NOTIFYNORMAL);
}

void Session::setMonitorSilence(bool _monitor)
{
    if (_monitorSilence==_monitor) {
        return;
    }

    _monitorSilence=_monitor;
    if (_monitorSilence) {
        _monitorTimer->start(_silenceSeconds*1000);
    } else {
        _monitorTimer->stop();
    }

    activityStateSet(NOTIFYNORMAL);
}

void Session::setMonitorSilenceSeconds(int seconds)
{
    _silenceSeconds=seconds;
    if (_monitorSilence) {
        _monitorTimer->start(_silenceSeconds*1000);
    }
}

void Session::setFlowControlEnabled(bool enabled)
{
    if (_flowControl == enabled) {
        return;
    }

    _flowControl = enabled;

    emit flowControlEnabledChanged(enabled);
}
bool Session::flowControlEnabled() const
{
    return _flowControl;
}

void Session::onReceiveBlock( const char * buf, int len )
{
    _emulation->receiveData( buf, len );
    emit receivedData( QString::fromLatin1( buf, len ) );
}

QSize Session::size()
{
    return _emulation->imageSize();
}

void Session::setSize(const QSize & size)
{
    if ((size.width() <= 1) || (size.height() <= 1)) {
        return;
    }

    emit resizeRequest(size);
}

int Session::recvData(const char *buff, int len)
{
    onReceiveBlock(buff,len);
    return len;
}

SessionGroup::SessionGroup()
        : _masterMode(0)
{
}
SessionGroup::~SessionGroup()
{
    // disconnect all
    connectAll(false);
}
int SessionGroup::masterMode() const
{
    return _masterMode;
}
QList<Session *> SessionGroup::sessions() const
{
    return _sessions.keys();
}
bool SessionGroup::masterStatus(Session * session) const
{
    return _sessions[session];
}

void SessionGroup::addSession(Session * session)
{
    _sessions.insert(session,false);

    QListIterator<Session *> masterIter(masters());

    while ( masterIter.hasNext() ) {
        connectPair(masterIter.next(),session);
    }
}
void SessionGroup::removeSession(Session * session)
{
    setMasterStatus(session,false);

    QListIterator<Session *> masterIter(masters());

    while ( masterIter.hasNext() ) {
        disconnectPair(masterIter.next(),session);
    }

    _sessions.remove(session);
}
void SessionGroup::setMasterMode(int mode)
{
    _masterMode = mode;

    connectAll(false);
    connectAll(true);
}
QList<Session *> SessionGroup::masters() const
{
    return _sessions.keys(true);
}
void SessionGroup::connectAll(bool connect)
{
    QListIterator<Session *> masterIter(masters());

    while ( masterIter.hasNext() ) {
        Session * master = masterIter.next();

        QListIterator<Session *> otherIter(_sessions.keys());
        while ( otherIter.hasNext() ) {
            Session * other = otherIter.next();

            if ( other != master ) {
                if ( connect ) {
                    connectPair(master,other);
                } else {
                    disconnectPair(master,other);
                }
            }
        }
    }
}
void SessionGroup::setMasterStatus(Session * session, bool master)
{
    bool wasMaster = _sessions[session];
    _sessions[session] = master;

    if (wasMaster == master) {
        return;
    }

    QListIterator<Session *> iter(_sessions.keys());
    while (iter.hasNext()) {
        Session * other = iter.next();

        if (other != session) {
            if (master) {
                connectPair(session, other);
            } else {
                disconnectPair(session, other);
            }
        }
    }
}

void SessionGroup::connectPair(Session * master , Session * other) const
{
    if ( _masterMode & CopyInputToAll ) {
        qDebug() << "Connection session " << master->nameTitle() << "to" << other->nameTitle();

        connect( master->emulation() , SIGNAL(sendData(const char *,int)) , other->emulation() ,
                 SLOT(sendString(const char *,int)) );
    }
}
void SessionGroup::disconnectPair(Session * master , Session * other) const
{
    if ( _masterMode & CopyInputToAll ) {
        qDebug() << "Disconnecting session " << master->nameTitle() << "from" << other->nameTitle();

        disconnect( master->emulation() , SIGNAL(sendData(const char *,int)) , other->emulation() ,
                    SLOT(sendString(const char *,int)) );
    }
}

