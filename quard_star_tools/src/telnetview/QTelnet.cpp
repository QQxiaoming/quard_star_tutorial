#include "QTelnet.h"
#include <QHostAddress>

const char QTelnet::IACWILL[2] = { IAC, WILL };
const char QTelnet::IACWONT[2] = { IAC, WONT };
const char QTelnet::IACDO[2]   = { IAC, DO };
const char QTelnet::IACDONT[2] = { IAC, DONT };
const char QTelnet::IACSB[2]   = { IAC, SB };
const char QTelnet::IACSE[2]   = { IAC, SE };

char QTelnet::_sendCodeArray[2]   = { IAC, 0 };
char QTelnet::_arrCRLF[2]         = { 13, 10 };
char QTelnet::_arrCR[2]           = { 13, 0 };

QTelnet::QTelnet(SocketType type, QObject *parent) :
    QObject(parent),  m_socketType(type), m_actualSB(0)
{
    connect(&m_tcpSocket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)),
            this, SLOT(socketError(QAbstractSocket::SocketError)) );
    connect(&m_tcpSocket, SIGNAL(readyRead()),	this, SLOT(onTcpReadyRead()) );
    connect(&m_tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onStateChanged(QAbstractSocket::SocketState)) );

    connect(&m_webSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(socketError(QAbstractSocket::SocketError)) );
    connect(&m_webSocket, SIGNAL(binaryMessageReceived(QByteArray)),
            this, SLOT(binaryMessageReceived(QByteArray)) );
    connect(&m_webSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onStateChanged(QAbstractSocket::SocketState)) );
}

void QTelnet::setType(SocketType type)
{
    m_socketType = type;
}

QString QTelnet::peerInfo() const
{
    if(m_socketType == TCP)
        return m_tcpSocket.peerName()+" ("+m_tcpSocket.peerAddress().toString()+" ):"+m_tcpSocket.peerPort();
    else if(m_socketType == WEBSOCKET)
        return m_webSocket.peerName()+" ("+m_webSocket.peerAddress().toString()+" ):"+QString::number(m_webSocket.peerPort());

    return QString();
}

QString QTelnet::peerName() const
{
    if(m_socketType == TCP)
        return m_tcpSocket.peerName();
    else if(m_socketType == WEBSOCKET)
        return m_webSocket.peerName();

    return QString();
}

bool QTelnet::isConnected() const
{
    if(m_socketType == TCP)
        return m_tcpSocket.state() == QAbstractSocket::ConnectedState;
    else if(m_socketType == WEBSOCKET)
        return m_webSocket.state() == QAbstractSocket::ConnectedState;

    return false;
}

void QTelnet::connectToHost(const QString &host, quint16 port)
{
    if(!isConnected()) {
        resetProtocol();
        if(m_socketType == TCP) {
            m_tcpSocket.abort();
            m_tcpSocket.connectToHost(host, port, QAbstractSocket::ReadWrite, QAbstractSocket::AnyIPProtocol);
        } else if(m_socketType == WEBSOCKET) {
            m_webSocket.abort();
            m_webSocket.open(QUrl("ws://"+host+":"+QString::number(port)));
        }
    }
}

void QTelnet::disconnectFromHost(void)
{
    if(m_socketType == TCP)
        m_tcpSocket.disconnectFromHost();
    else if(m_socketType == WEBSOCKET)
        m_webSocket.close();
}

void QTelnet::write(const char c)
{
    if(!isConnected())
        return;
    if(m_socketType == TCP)
        m_tcpSocket.write( (char*)&c, 1 );
    else if(m_socketType == WEBSOCKET)
        m_webSocket.sendBinaryMessage(QByteArray(&c, 1));
}

qint64 QTelnet::write(const char *data, qint64 len)
{
    if(!isConnected())
        return 0;
    if(m_socketType == TCP) {
        return m_tcpSocket.write( data, len );
    } else if(m_socketType == WEBSOCKET) {
        return m_webSocket.sendBinaryMessage(QByteArray(data, len));
    }
    return 0;
}

qint64 QTelnet::read(char *data, qint64 maxlen)
{
    if(!isConnected())
        return 0;
    if(m_socketType == TCP)
        return m_tcpSocket.read(data, maxlen);
    else if(m_socketType == WEBSOCKET) {
        return 0;
    }
    return 0;
}

bool QTelnet::testBinaryMode() const
{
    return m_receivedDX[(unsigned char)TELOPT_BINARY] == DO;
}

void QTelnet::sendData(const QByteArray &ba)
{
    if( isConnected() )
        transpose( ba.constData(), ba.length() );
}

void QTelnet::sendData(const char *data, int len)
{
    if( isConnected() )
        transpose( data, len );
}

void QTelnet::socketError(QAbstractSocket::SocketError err)
{
    disconnectFromHost();
    Q_UNUSED(err);
}

void QTelnet::setCustomCR(char cr, char cr2)
{
    _arrCR[0] = cr;
    _arrCR[1] = cr2;
}

void QTelnet::setCustomCRLF(char lf, char cr)
{
    _arrCR[0] = lf;
    _arrCR[1] = cr;
}

// Envia el codigo de control al servidor.
void QTelnet::sendTelnetControl(char codigo)
{
    _sendCodeArray[1] = codigo;
    write(_sendCodeArray, 2);
}

void QTelnet::writeCustomCRLF()
{
    write(_arrCRLF, 2);
}

void QTelnet::writeCustomCR()
{
    write(_arrCR, 2);
}

/// Resetea los datos del protocolo. Debe llamarse cada vez que se inicia una conexión nueva.
void QTelnet::resetProtocol()
{
    for( int i = 0; i < 256; i++ )
    {
        m_receivedDX[i] =
            m_receivedWX[i] =
            m_sentDX[i] =
            m_sentWX[i] = 0;
        m_negotiationState = STATE_DATA;
        m_buffSB.clear();
        m_actualSB = 0;
    }
    m_oldWinSize.setHeight(-1);
    m_oldWinSize.setWidth(-1);
}

void QTelnet::sendSB(char code, char *arr, int iLen)
{
    write(IAC);
    write(SB);
    write(code);

    write(arr, iLen);

    write(IAC);
    write(SE);
}
void QTelnet::sendWindowSize()
{
    if( isConnected() && (m_receivedDX[TELOPT_NAWS] == DO) && (m_oldWinSize != m_winSize) )
    {
        char size[4];

        m_oldWinSize = m_winSize;
        size[0] = (m_winSize.width()>>8) & 0xFF;
        size[1] = m_winSize.width() & 0xFF;
        size[2] = (m_winSize.height()>>8) & 0xFF;
        size[3] = m_winSize.height() & 0xFF;
        sendSB(TELOPT_NAWS, size, 4);
    }
}

// Handle an incoming IAC SB type chars IAC SE
void QTelnet::handleSB()
{
    switch( m_actualSB )
    {
    case TELOPT_TTYPE:
        if( (m_buffSB.length() > 0) &&
            ((unsigned char)m_buffSB[0] == (unsigned char)TELQUAL_SEND) )
        {
            write(IACSB, 2);
            write(TELOPT_TTYPE);
            write(TELQUAL_IS);
            /* FIXME: need more logic here if we use
            * more than one terminal type
            */
            write("SiraggaTerminal", 15);
            write(IACSE, 2);
        }
        break;
    }
}

// Analiza el texto saliente para que cumpla las normas del protocolo.
// Además ya lo escribe en el m_tcpSocket.
void QTelnet::transpose(const char *buf, int iLen)
{
    for( int i = 0; i < iLen; i++ )
    {
        switch( buf[i] )
        {
        case IAC:
            // Escape IAC twice in stream ... to be telnet protocol compliant
            // this is there in binary and non-binary mode.
            write(IAC);
            write(IAC);
            break;
        case 10:    // \n
            // We need to heed RFC 854. LF (\n) is 10, CR (\r) is 13
            // we assume that the Terminal sends \n for lf+cr and \r for just cr
            // linefeed+carriage return is CR LF

            // En modo binario no se traduce nada.
            if( testBinaryMode() || m_socketType == WEBSOCKET )
                write(buf[i]);
            else
                writeCustomCRLF();
            break;
        case 13:    // \r
            // carriage return is CR NUL */

            // En modo binario no se traduce nada.
            if( testBinaryMode() || m_socketType == WEBSOCKET )
                write(buf[i]);
            else
                writeCustomCRLF();
            break;
        default:
            // all other characters are just copied
            write(buf[i]);
            break;
        }
    }
}

void QTelnet::willsReply(char action, char reply)
{
    if( (reply != m_sentDX[(unsigned char)action]) ||
        (WILL != m_receivedWX[(unsigned char)action]) )
    {
        write(IAC);
        write(reply);
        write(action);

        m_sentDX[(unsigned char)action] = reply;
        m_receivedWX[(unsigned char)action] = WILL;
    }
}

void QTelnet::wontsReply(char action, char reply)
{
    if( (reply != m_sentDX[(unsigned char)action]) ||
        (WONT != m_receivedWX[(unsigned char)action]) )
    {
        write(IAC);
        write(reply);
        write(action);

        m_sentDX[(unsigned char)action] = reply;
        m_receivedWX[(unsigned char)action] = WONT;
    }
}

void QTelnet::doesReply(char action, char reply)
{
    if( (reply != m_sentWX[(unsigned char)action]) ||
        (DO != m_receivedDX[(unsigned char)action]) )
    {
        write(IAC);
        write(reply);
        write(action);

        m_sentWX[(unsigned char)action] = reply;
        m_receivedDX[(unsigned char)action] = DO;
    }
}

void QTelnet::dontsReply(char action, char reply)
{
    if( (reply != m_sentWX[(unsigned char)action]) ||
        (DONT != m_receivedDX[(unsigned char)action]) )
    {
        write(IAC);
        write(reply);
        write(action);

        m_sentWX[(unsigned char)action] = reply;
        m_receivedDX[(unsigned char)action] = DONT;
    }
}

// Analiza el buffer de entrada colocá	ndolo en el buffer de procesado usando el protocolo telnet.
qint64 QTelnet::doTelnetInProtocol(qint64 buffSize)
{
    qint64 iIn, iOut;

    for( iIn = 0, iOut = 0; iIn < buffSize; iIn++ )
    {
        char b = m_buffIncoming[iIn];

        switch( m_negotiationState )
        {
        // TODO: Currently we implement only bypass CRLF and CR, do not convert
        case STATE_DATA:
            switch( b )
            {
            case IAC:
                m_negotiationState = STATE_IAC;
                break;
            case '\r':
                m_buffProcessed[iOut++] = '\r';
                m_negotiationState = STATE_DATAR;
                break;
            case '\n':
                m_buffProcessed[iOut++] = '\n';
                m_negotiationState = STATE_DATAN;
                break;
            default:
                m_buffProcessed[iOut++] = b;
                break;
            }
            break;
        case STATE_DATAN:
        case STATE_DATAR:
            switch( b )
            {
            case IAC:
                m_negotiationState = STATE_IAC;
                break;
            case '\r':
                m_buffProcessed[iOut++] = '\r';
                m_negotiationState = STATE_DATA;
                break;
            case '\n':
                m_buffProcessed[iOut++] = '\n';
                m_negotiationState = STATE_DATA;
                break;
            default:
                m_buffProcessed[iOut++] = b;
                m_negotiationState = STATE_DATA;
                break;
            }
            break;
        case STATE_IAC:
            switch( b )
            {
            case IAC:   // Dos IAC seguidos, se intenta enviar un caracter con el valor IAC.
                m_negotiationState = STATE_DATA;
                m_buffProcessed[iOut++] = IAC;
                break;
            case WILL:
                m_negotiationState = STATE_IACWILL;
                break;
            case WONT:
                m_negotiationState = STATE_IACWONT;
                break;
            case DONT:
                m_negotiationState = STATE_IACDONT;
                break;
            case DO:
                m_negotiationState = STATE_IACDO;
                break;
            case EOR:
                emitEndOfRecord();
                m_negotiationState = STATE_DATA;
                break;
            case SB:
                m_negotiationState = STATE_IACSB;
                m_buffSB.clear();
                break;
            default:
                m_negotiationState = STATE_DATA;
                break;
            }
            break;
        case STATE_IACWILL:
            switch( b )
            {
            case TELOPT_ECHO:
                emitEchoLocal(false);
                willsReply(b, DO);
                break;
            case TELOPT_SGA:
                willsReply(b, DO);
                break;
            case TELOPT_EOR:
                willsReply(b, DO);
                break;
            case TELOPT_BINARY:
                willsReply(b, DO);
                break;
            default:
                willsReply(b, DONT);
                break;
            }
            m_negotiationState = STATE_DATA;
            break;
        case STATE_IACWONT:
            switch(b)
            {
            case TELOPT_ECHO:
                emitEchoLocal(true);
                wontsReply(b, DONT);
                break;
            case TELOPT_SGA:
                wontsReply(b, DONT);
                break;
            case TELOPT_EOR:
                wontsReply(b, DONT);
                break;
            case TELOPT_BINARY:
                wontsReply(b, DONT);
                break;
            default:
                wontsReply(b, DONT);
                break;
            }
            m_negotiationState = STATE_DATA;
            break;
        case STATE_IACDO:
            switch( b )
            {
            case TELOPT_ECHO:
                doesReply(b, WILL);
                emitEchoLocal(true);
                break;
            case TELOPT_SGA:
                doesReply(b, WILL);
                break;
            case TELOPT_TTYPE:
                doesReply(b, WILL);
                break;
            case TELOPT_BINARY:
                doesReply(b, WILL);
                break;
            case TELOPT_NAWS:
                m_receivedDX[(unsigned char)b] = (unsigned char)DO;
                m_sentWX[(unsigned char)b] = (unsigned char)WILL;
                write(IAC);
                write(WILL);
                write(b);

                // Enviamos el tamaño de la pantalla.
                sendWindowSize();
                break;
            default:
                doesReply(b, WONT);
                break;
            }
            m_negotiationState = STATE_DATA;
            break;
        case STATE_IACDONT:
            switch (b)
            {
            case TELOPT_ECHO:
                dontsReply(b, WONT);
                emitEchoLocal(false);
                break;
            case TELOPT_SGA:
                dontsReply(b, WONT);
                break;
            case TELOPT_NAWS:
                dontsReply(b, WONT);
                break;
            case TELOPT_BINARY:
                dontsReply(b, WONT);
                break;
            default:
                dontsReply(b, WONT);
                break;
            }
            m_negotiationState = STATE_DATA;
            break;
        case STATE_IACSB:
            switch( b )
            {
            case IAC:
                // Entramos en estado IAC en la sub-negociación.
                m_negotiationState = STATE_IACSBIAC;
                break;
            default:
                // Iniciamos la sub-negociación.
                m_buffSB.clear();
                m_actualSB = b;
                m_negotiationState = STATE_IACSBDATA;
                break;
            }
            break;
        case STATE_IACSBDATA:   // Estamos en datos de la subnegociación.
            switch( b )
            {
            case IAC:
                m_negotiationState = STATE_IACSBDATAIAC;
                break;
            default:
                m_buffSB.append(b);
                break;
            }
            break;
        case STATE_IACSBIAC:
            switch( b )
            {
            case IAC:
                // Reiniciamos la sub-negociación.
                m_buffSB.clear();
                m_actualSB = b;
                m_negotiationState = STATE_IACSBDATA;
                break;
            default:
                // Salimos de la sub-negociación.
                m_negotiationState = STATE_DATA;
                break;
            }
            break;
        case STATE_IACSBDATAIAC:
            switch( b )
            {
            case IAC:
                m_negotiationState = STATE_IACSBDATA;
                m_buffSB.append(IAC);
                break;
            case SE:
                handleSB();
                m_actualSB = 0;
                m_buffSB.clear();
                m_negotiationState = STATE_DATA;
                break;
            case SB:
                handleSB();
                m_buffSB.clear();
                m_negotiationState = STATE_IACSB;
                break;
            default:
                m_buffSB.clear();
                m_actualSB = 0;
                m_negotiationState = STATE_DATA;
                break;
            }
            break;
        default:
            m_negotiationState = STATE_DATA;
            break;
        }
    }
    return iOut;
}

void QTelnet::onTcpReadyRead()
{
    qint64 readed;
    qint64 processed;

    while( (readed = read(m_buffIncoming, IncommingBufferSize)) != 0 )
    {
        switch( readed )
        {
        case -1:
            disconnectFromHost();
            break;
        default:
            processed = doTelnetInProtocol(readed);
            if( processed > 0 )
                emit newData(m_buffProcessed, processed);

            break;
        }
    }
}

void QTelnet::binaryMessageReceived(const QByteArray &message)
{
    emit newData(message.constData(), message.length());
}

void QTelnet::onStateChanged(QAbstractSocket::SocketState s) 
{
    emit stateChanged(s);
}

