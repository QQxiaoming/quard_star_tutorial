/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
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


#include <QtTest/QtTest>

#include <qwebenginepage.h>
#include <qwebengineview.h>
#include <QDebug>

class tst_Shutdown : public QObject
{
    Q_OBJECT

public:
    tst_Shutdown();
    virtual ~tst_Shutdown();

public Q_SLOTS:
    void init();
    void cleanup();

private Q_SLOTS:
    void dummyTest();

private:


private:
    QWebEngineView* m_view;
    QWebEnginePage* m_page;
};

tst_Shutdown::tst_Shutdown()
{
}

tst_Shutdown::~tst_Shutdown()
{
}

void tst_Shutdown::init()
{
    m_view = new QWebEngineView();
    m_page = m_view->page();
}

void tst_Shutdown::cleanup()
{
    delete m_view;
}

void tst_Shutdown::dummyTest()
{
    QVERIFY(m_view);
}

QTEST_MAIN(tst_Shutdown)
#include "tst_shutdown.moc"
