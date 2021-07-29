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

#include "qwebenginequotarequest.h"

#include "quota_request_controller.h"

QT_BEGIN_NAMESPACE

/*!
    \class QWebEngineQuotaRequest
    \brief The QWebEngineQuotaRequest class enables accepting or rejecting
    requests for larger persistent storage than the application's current
    allocation in File System API.

    \since 5.11
    \inmodule QtWebEngineCore

    This class is used by the QWebEnginePage::quotaRequested() signal to \l
    accept() or \l reject() a request for an increase in the persistent storage
    allocated to the application. The default quota is 0 bytes.
*/

/*! \fn QWebEngineQuotaRequest::QWebEngineQuotaRequest()
    \internal
*/

/*! \internal */
QWebEngineQuotaRequest::QWebEngineQuotaRequest(QSharedPointer<QtWebEngineCore::QuotaRequestController> controller)
    : d_ptr(controller)
{
}

/*!
    Rejects a request for larger persistent storage.
*/
void QWebEngineQuotaRequest::reject()
{
    d_ptr->reject();
}

/*!
    Accepts a request for larger persistent storage.
*/
void QWebEngineQuotaRequest::accept()
{
    d_ptr->accept();
}

/*!
    \property QWebEngineQuotaRequest::origin
    \brief The URL of the web page that issued the quota request.
*/

QUrl QWebEngineQuotaRequest::origin() const
{
    return d_ptr->origin();
}

/*!
    \property QWebEngineQuotaRequest::requestedSize
    \brief Contains the size of the requested disk space in bytes.
*/

qint64 QWebEngineQuotaRequest::requestedSize() const
{
    return d_ptr->requestedSize();
}

/*! \fn bool QWebEngineQuotaRequest::operator==(const QWebEngineQuotaRequest &that) const
    Returns \c true if \a that points to the same object as this quota request.
*/

/*! \fn bool QWebEngineQuotaRequest::operator!=(const QWebEngineQuotaRequest &that) const
    Returns \c true if \a that points to a different object than this request.
*/

QT_END_NAMESPACE
