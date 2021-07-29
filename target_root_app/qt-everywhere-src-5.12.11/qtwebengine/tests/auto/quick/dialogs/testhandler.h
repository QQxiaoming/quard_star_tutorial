/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef TESTHANDLER_H
#define TESTHANDLER_H

#include <QObject>

class TestHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* request READ request WRITE setRequest NOTIFY requestChanged)
    Q_PROPERTY(bool ready READ ready WRITE setReady NOTIFY readyChanged)
public:
    explicit TestHandler(QObject *parent = nullptr);
    QObject* request() const;

    bool ready() const;
    void setReady(bool ready);
    void setRequest(QObject *request);
    void runJavaScript(const QString &script);
    void load(const QUrl &page);

signals:
    void loadPage(const QUrl &page);
    void javaScript(const QString &script);
    void requestChanged(QObject *request);
    void readyChanged(bool ready);

private:
    QObject *m_request = nullptr;
    bool m_ready = false;
};

#endif // TESTHANDLER_H
