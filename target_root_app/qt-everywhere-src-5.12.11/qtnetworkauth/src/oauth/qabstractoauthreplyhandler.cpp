/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Network Auth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
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
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtNetwork/qtnetwork-config.h>

#ifndef QT_NO_HTTP

#include "qabstractoauthreplyhandler.h"
#include "qabstractoauthreplyhandler_p.h"

Q_LOGGING_CATEGORY(lcReplyHandler, "qt.networkauth.replyhandler")

QT_BEGIN_NAMESPACE

/*!
    \class QAbstractOAuthReplyHandler
    \inmodule QtNetworkAuth
    \ingroup oauth
    \brief Handles replies to OAuth authentication requests.
    \since 5.8

    The QAbstractOAuthReplyHandler class handles the answers
    to all OAuth authentication requests.
    This class is designed as a base whose subclasses implement
    custom behavior in the callback() and networkReplyFinished()
    methods.
*/

/*!
    \fn QString QAbstractOAuthReplyHandler::callback() const

    Returns an absolute URI that the server will redirect the
    resource owner back to when the Resource Owner Authorization step
    is completed.  If the client is unable to receive callbacks or a
    callback URI has been established via other means, the parameter
    value \b must be set to "oob" (all lower-case), to indicate an
    out-of-band configuration.

    Derived classes should implement this function to provide the
    expected callback type.
*/

/*!
    \fn void QAbstractOAuthReplyHandler::networkReplyFinished(QNetworkReply *reply)

    After the server determines whether the request is valid this
    function will be called. Reimplement it to get the data received
    from the server wrapped in \a reply.
*/

/*!
    \fn void QAbstractOAuthReplyHandler::callbackReceived(const QVariantMap &values)

    This signal is emitted when the reply from the server is
    received, with \a values containing the token credentials
    and any additional information the server may have returned.
    When this signal is emitted, the authorization process
    is complete.
*/

/*!
    \fn void QAbstractOAuthReplyHandler::tokensReceived(const QVariantMap &tokens)

    This signal is emitted when new \a tokens are received from the
    server.
*/

/*!
    \fn void QAbstractOAuthReplyHandler::replyDataReceived(const QByteArray &data)

    This signal is emitted when an HTTP request finishes and the
    data is available. \a data contains the response before parsing.
*/

/*!
    \fn void QAbstractOAuthReplyHandler::callbackDataReceived(const QByteArray &data)

    This signal is emitted when a callback request is received:
    \a data contains the information before parsing.
*/

/*!
    Constructs a reply handler as a child of \a parent.
*/
QAbstractOAuthReplyHandler::QAbstractOAuthReplyHandler(QObject *parent)
    : QObject(parent)
{}

/*!
    Destroys the reply handler.
*/
QAbstractOAuthReplyHandler::~QAbstractOAuthReplyHandler()
{}

/*! \internal */
QAbstractOAuthReplyHandler::QAbstractOAuthReplyHandler(QObjectPrivate &d, QObject *parent)
    : QObject(d, parent)
{}

QT_END_NAMESPACE

#endif // QT_NO_HTTP
