/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>
#include <QtX11Extras/QX11Info>

class IntensivePainter : public QWindow
{
public:
    IntensivePainter() : m_backingStore(new QBackingStore(this))
    { setGeometry(100, 100, 900, 600); }

protected:
    bool interrupted() const { return m_stopPainting; }
    void stopPainting() { m_stopPainting = true; }
    void beginPaining() { m_stopPainting = false; }

    void exposeEvent(QExposeEvent *)
    {
        bool useGreenColor = false;
        QRect rect(0, 0, width(), height());
        m_backingStore->resize(QSize(width(), height()));

        qint32 peekerId = QX11Info::generatePeekerId();
        if (peekerId == -1) {
            qWarning() << "Internal error: QX11Info::generatePeekerId() returned -1";
            exit(EXIT_FAILURE);
        }

        while (!interrupted()) { // Begin a long operation
            m_backingStore->beginPaint(rect);
            QPaintDevice *device = m_backingStore->paintDevice();
            QPainter painter(device);
            painter.fillRect(rect, useGreenColor ? Qt::green : Qt::red);
            useGreenColor = !useGreenColor;
            painter.drawText(rect, Qt::AlignCenter, QStringLiteral("Press any key to exit.\n"
            "(make sure the window is in focus when testing)\n\n If key press does not "
            "immediately exit the application,\n then that should be considered a bug/regression."
            "\n\n A demo might appear frozen to some window managers\n and therefore "
            "might get grayed out (e.g Unity), this is the expected behavior.\n On KWin, for "
            "example, this demo does not get grayed out.\n Resizing the window on any system will "
            "cause rendering artefacts, that is not a bug,\n but simply the way the test is "
            "implemented."));
            m_backingStore->endPaint();
            m_backingStore->flush(rect);
            QThread::msleep(500); // Reduce the speed of re-painting (blinking)

            QX11Info::peekEventQueue([](xcb_generic_event_t *event, void *data) {
                bool isKeyPress = (event->response_type & ~0x80) == XCB_KEY_PRESS;
                if (isKeyPress) {
                    IntensivePainter *painter = static_cast<IntensivePainter *>(data);
                    painter->stopPainting();
                }
                return isKeyPress;
            }, this, QX11Info::PeekOption::PeekFromCachedIndex, peekerId);
        }

        QX11Info::removePeekerId(peekerId);
        exit(EXIT_SUCCESS);
    }

private:
    QBackingStore *m_backingStore;
    bool m_stopPainting = false;
};

int main(int argc, char** argv)
{
    QGuiApplication app(argc, argv);

    IntensivePainter painterWindow;
    painterWindow.show();

    return app.exec();
}
