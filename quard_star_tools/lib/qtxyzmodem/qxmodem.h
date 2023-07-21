#ifndef QXMODEM_H
#define QXMODEM_H

#include <QString>
#include <QFile>
#include <QMutex>
#include <QThread>
#include <QDebug>

class QXmodem: public QThread {
    Q_OBJECT
public:
    explicit QXmodem():QThread()
    {
    };
    ~QXmodem(){};

    enum {
        SEND,
        RECV
    };
    
    /* xmodem control characters */
    enum {
        SOH	  = 0x01,
        STX	  = 0x02,
        EOT	  = 0x04,
        ACK	  = 0x06,
        NAK	  = 0x15,
        CAN	  = 0x18,
        CTRLZ = 0x1A,
    };

    /* xmodem timeout/retry parameters */
    const int XMODEM_TIMEOUT_DELAY=	1000;
    const int XMODEM_RETRY_LIMIT = 16;
    const int XMODEM_BLOCK = 1;

    /* error return codes */
    enum {
        XMODEM_ERROR_REMOTECANCEL = -1,
        XMODEM_ERROR_OUTOFSYNC	  = -2,
        XMODEM_ERROR_RETRYEXCEED  = -3,
    };

    void startSend(void) {
        dir=SEND;
        start();
    }

    void startRecv(void) {
        dir=RECV;
        start();
    }

protected:
    void run() {
        _start();
        switch(dir) {
            case SEND:
                xmodemTransmit();
                break;
            case RECV:
                xmodemReceive();
                break;
        }
        _end();
    }

private:
    virtual void _start(void) = 0;
    virtual void _end(void) = 0;

    virtual int writefile(const char* buffer, int size) = 0;
    virtual int readfile(char* buffer, int size) = 0;

    virtual int sendStream(const char* buffer, int size) = 0;
    virtual int receiveStream(const char* buffer, int size) = 0;

    void xmodemOut(unsigned char c) {
        sendStream((const char*)&c,1);
    }

    int xmodemIn(unsigned char *c) {
        return receiveStream((const char*)c,1);
    }    
    
    void timerPause(int t) {
        Q_UNUSED(t);
    }

    uint16_t crc_xmodem_update(uint16_t crc, uint8_t data);
    long xmodemReceive(void);
    long xmodemTransmit(void);
    int xmodemCrcCheck(int crcflag, const unsigned char *buffer, int size);
    int xmodemInTime(unsigned char *c, unsigned short timeout);
    void xmodemInFlush(void);
public:
    int dir=SEND;
};

class QXmodemFile: public QXmodem {
    Q_OBJECT
public:
    QXmodemFile(QString filename) {
        m_file = new QFile(filename);
    }
    QXmodemFile(const char *filename) {
        m_file = new QFile(QString(filename));
    }
    ~QXmodemFile(){
        delete m_file;
    }
    
signals:
    void send(QByteArray ba);

public slots:
    void receive(QByteArray ba) {
        m_mutex.lock();
        cache.append(ba);
        m_mutex.unlock();
    }

private:
    void _start(void) {
        m_file->open(QIODevice::ReadWrite);
    }

    void _end(void) {
        m_file->close();
    }

    int writefile(const char* buffer, int size) {
        return m_file->write(buffer,size);
    }
    int readfile(char* buffer, int size) {
        int r = m_file->read(buffer,size);
        return r;
    }
    int sendStream(const char* buffer, int size) {
        emit send(QByteArray(buffer,size));
        return size;
    }
    int receiveStream(const char* buffer, int size) {
        m_mutex.lock();
        int ret = qMin(size,cache.size());
        memcpy((void*)buffer,cache.data(),ret);
        cache.remove(0,ret);
        m_mutex.unlock();
        return ret;
    }

private:
    QFile *m_file = nullptr;
    QMutex m_mutex;
    QByteArray cache;
};

#endif /* QXMODEM_H */ 
