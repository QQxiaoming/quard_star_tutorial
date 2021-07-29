/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "qwebengineurlschemehandler.h"

#include "qwebengineurlrequestjob.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineUrlSchemeHandler
    \brief The QWebEngineUrlSchemeHandler class is a base class for handling custom URL schemes.
    \since 5.6

    To implement a custom URL scheme for QtWebEngine, you first have to create an instance of
    QWebEngineUrlScheme and register it using QWebEngineUrlScheme::registerScheme().

    \note Make sure that you create and register the scheme object \e before the QGuiApplication
    or QApplication object is instantiated.

    Then you must create a class derived from QWebEngineUrlSchemeHandler,
    and reimplement the requestStarted() method.

    Finally, install the scheme handler object via QWebEngineProfile::installUrlSchemeHandler()
    or QQuickWebEngineProfile::installUrlSchemeHandler().

    \code

    class MySchemeHandler : public QWebEngineUrlSchemeHandler
    {
    public:
        MySchemeHandler(QObject *parent = nullptr);
        void requestStarted(QWebEngineUrlRequestJob *request)
        {
            // ....
        }
    };

    int main(int argc, char **argv)
    {
        QWebEngineUrlScheme scheme("myscheme");
        scheme.setSyntax(QWebEngineUrlScheme::Syntax::HostAndPort);
        scheme.setDefaultPort(2345);
        scheme.setFlags(QWebEngineUrlScheme::SecureScheme);
        QWebEngineUrlScheme::registerScheme(scheme);

        // ...
        QApplication app(argc, argv);
        // ...

        // installUrlSchemeHandler does not take ownership of the handler.
        MySchemeHandler *handler = new MySchemeHandler(parent);
        QWebEngineProfile::defaultProfile()->installUrlSchemeHandler("myscheme", handler);
    }
    \endcode

    \inmodule QtWebEngineCore

    \sa {QWebEngineUrlScheme}, {WebEngine Widgets WebUI Example}
*/

/*!
    Constructs a new URL scheme handler.

    The handler is created with the parent \a parent.

  */
QWebEngineUrlSchemeHandler::QWebEngineUrlSchemeHandler(QObject *parent)
    : QObject(parent)
{
}

/*!
    Deletes a custom URL scheme handler.
*/
QWebEngineUrlSchemeHandler::~QWebEngineUrlSchemeHandler()
{
    Q_EMIT _q_destroyedUrlSchemeHandler(this);
}

/*!
    \fn void QWebEngineUrlSchemeHandler::requestStarted(QWebEngineUrlRequestJob *request)

    This method is called whenever a request \a request for the registered scheme is started.

    This method must be reimplemented by all custom URL scheme handlers.
    The request is asynchronous and does not need to be handled right away.

    \sa QWebEngineUrlRequestJob
*/

QT_END_NAMESPACE
