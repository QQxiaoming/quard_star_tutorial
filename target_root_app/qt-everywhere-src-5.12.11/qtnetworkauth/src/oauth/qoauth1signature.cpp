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

#include "qoauth1signature.h"
#include "qoauth1signature_p.h"

#include <QtCore/qurlquery.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qmessageauthenticationcode.h>

#include <QtNetwork/qnetworkaccessmanager.h>

#include <functional>
#include <type_traits>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(loggingCategory, "qt.networkauth.oauth1.signature")

/*!
    \class QOAuth1Signature
    \inmodule QtNetworkAuth
    \ingroup oauth
    \brief Implements OAuth 1 signature methods.
    \since 5.8

    OAuth-authenticated requests can have two sets of credentials:
    those passed via the "oauth_consumer_key" parameter and those in
    the "oauth_token" parameter.  In order for the server to verify
    the authenticity of the request and prevent unauthorized access,
    the client needs to prove that it is the rightful owner of the
    credentials.  This is accomplished using the shared-secret (or
    RSA key) part of each set of credentials.

    OAuth specifies three methods for the client to establish its
    rightful ownership of the credentials: "HMAC-SHA1", "RSA-SHA1",
    and "PLAINTEXT".  Each generates a "signature" with which the
    request is "signed"; the first two use a digest of the data
    signed in generating this, though the last does not.  The
    "RSA-SHA1" method is not supported here; it would use an RSA key
    rather than the shared-secret associated with the client
    credentials.
*/

/*!
    \enum QOAuth1Signature::HttpRequestMethod

    Indicates the HTTP request method.

    \value Head     HEAD method.
    \value Get      GET method.
    \value Put      PUT method.
    \value Post     POST method.
    \value Delete   DELETE method.
    \value Custom   Identifies a custom method.
    \value Unknown  Method not set.
*/

static_assert(static_cast<int>(QOAuth1Signature::HttpRequestMethod::Head) ==
              static_cast<int>(QNetworkAccessManager::HeadOperation) &&
              static_cast<int>(QOAuth1Signature::HttpRequestMethod::Get) ==
              static_cast<int>(QNetworkAccessManager::GetOperation) &&
              static_cast<int>(QOAuth1Signature::HttpRequestMethod::Put) ==
              static_cast<int>(QNetworkAccessManager::PutOperation) &&
              static_cast<int>(QOAuth1Signature::HttpRequestMethod::Post) ==
              static_cast<int>(QNetworkAccessManager::PostOperation) &&
              static_cast<int>(QOAuth1Signature::HttpRequestMethod::Delete) ==
              static_cast<int>(QNetworkAccessManager::DeleteOperation),
              "Invalid QOAuth1Signature::HttpRequestMethod enumeration values");

QOAuth1SignaturePrivate QOAuth1SignaturePrivate::shared_null;

QOAuth1SignaturePrivate::QOAuth1SignaturePrivate(const QUrl &url,
                                                 QOAuth1Signature::HttpRequestMethod method,
                                                 const QVariantMap &parameters,
                                                 const QString &clientSharedKey,
                                                 const QString &tokenSecret) :
    method(method), url(url), clientSharedKey(clientSharedKey), tokenSecret(tokenSecret),
    parameters(parameters)
{}

QByteArray QOAuth1SignaturePrivate::signatureBaseString() const
{
    // https://tools.ietf.org/html/rfc5849#section-3.4.1
    QByteArray base;

    switch (method) {
    case QOAuth1Signature::HttpRequestMethod::Head:
        base.append("HEAD");
        break;
    case QOAuth1Signature::HttpRequestMethod::Get:
        base.append("GET");
        break;
    case QOAuth1Signature::HttpRequestMethod::Put:
        base.append("PUT");
        break;
    case QOAuth1Signature::HttpRequestMethod::Post:
        base.append("POST");
        break;
    case QOAuth1Signature::HttpRequestMethod::Delete:
        base.append("DELETE");
        break;
    default:
        qCCritical(loggingCategory, "QOAuth1Signature: HttpRequestMethod not supported");
    }
    base.append('&');
    base.append(QUrl::toPercentEncoding(url.toString(QUrl::RemoveQuery)) + "&");

    QVariantMap p = parameters;
    {
        // replace '+' with spaces now before decoding so that '%2B' gets left as '+'
        const QString query = url.query().replace(QLatin1Char('+'), QLatin1Char(' '));
        const auto queryItems = QUrlQuery(query).queryItems(QUrl::FullyDecoded);
        for (auto it = queryItems.begin(), end = queryItems.end(); it != end; ++it)
            p.insert(it->first, it->second);
    }
    base.append(encodeHeaders(p));
    return base;
}

QByteArray QOAuth1SignaturePrivate::secret() const
{
    QByteArray secret;
    secret.append(QUrl::toPercentEncoding(clientSharedKey));
    secret.append('&');
    secret.append(QUrl::toPercentEncoding(tokenSecret));
    return secret;
}

QByteArray QOAuth1SignaturePrivate::parameterString(const QVariantMap &parameters)
{
    QByteArray ret;
    auto previous = parameters.end();
    for (auto it = parameters.begin(), end = parameters.end(); it != end; previous = it++) {
        if (previous != parameters.end()) {
            if (Q_UNLIKELY(previous.key() == it.key()))
                qCWarning(loggingCategory, "duplicated key %s", qPrintable(it.key()));
            ret.append("&");
        }
        ret.append(QUrl::toPercentEncoding(it.key()));
        ret.append("=");
        ret.append(QUrl::toPercentEncoding(it.value().toString()));
    }
    return ret;
}

QByteArray QOAuth1SignaturePrivate::encodeHeaders(const QVariantMap &headers)
{
    return QUrl::toPercentEncoding(QString::fromLatin1(parameterString(headers)));
}

/*!
    Creates a QOAuth1Signature using
    \list
        \li \a url as the target address
        \li \a method as the HTTP method used to send the request
        \li and the given user \a parameters to augment the request.
    \endlist
*/
QOAuth1Signature::QOAuth1Signature(const QUrl &url, QOAuth1Signature::HttpRequestMethod method,
                                   const QVariantMap &parameters) :
    d(new QOAuth1SignaturePrivate(url, method, parameters))
{}

/*!
    Creates a QOAuth1Signature using
    \list
        \li \a url as the target address
        \li \a clientSharedKey as the user token used to verify the
        signature
        \li \a tokenSecret as the negotiated token used to verify
        the signature
        \li \a method as the HTTP method used to send the request
        \li and the given user \a parameters to augment the request.
    \endlist
*/
QOAuth1Signature::QOAuth1Signature(const QUrl &url, const QString &clientSharedKey,
                                   const QString &tokenSecret, HttpRequestMethod method,
                                   const QVariantMap &parameters) :
    d(new QOAuth1SignaturePrivate(url, method, parameters, clientSharedKey, tokenSecret))
{}

/*!
    Creates a copy of \a other.
*/
QOAuth1Signature::QOAuth1Signature(const QOAuth1Signature &other) : d(other.d)
{}

/*!
    Move-constructs a QOAuth1Signature instance, taking over the
    private data \a other was using.
*/
QOAuth1Signature::QOAuth1Signature(QOAuth1Signature &&other) : d(other.d)
{
    other.d = &QOAuth1SignaturePrivate::shared_null;
}

/*!
    Destroys the QOAuth1Signature.
*/
QOAuth1Signature::~QOAuth1Signature()
{}

/*!
    Returns the request method.
*/
QOAuth1Signature::HttpRequestMethod QOAuth1Signature::httpRequestMethod() const
{
    return d->method;
}

/*!
    Sets the request \a method.
*/
void QOAuth1Signature::setHttpRequestMethod(QOAuth1Signature::HttpRequestMethod method)
{
    d->method = method;
}

/*!
    Returns the URL.
*/
QUrl QOAuth1Signature::url() const
{
    return d->url;
}

/*!
    Sets the URL to \a url.
*/
void QOAuth1Signature::setUrl(const QUrl &url)
{
    d->url = url;
}

/*!
    Returns the parameters.
*/
QVariantMap QOAuth1Signature::parameters() const
{
    return d->parameters;
}

/*!
    Sets the \a parameters.
*/
void QOAuth1Signature::setParameters(const QVariantMap &parameters)
{
    d->parameters = parameters;
}

/*!
    Adds the request \a body to the signature. When a POST request
    body contains arguments they should be included in the signed
    data.
*/
void QOAuth1Signature::addRequestBody(const QUrlQuery &body)
{
    const auto list = body.queryItems();
    for (auto it = list.begin(), end = list.end(); it != end; ++it)
        d->parameters.insert(it->first, it->second);
}

/*!
    Inserts a new pair \a key, \a value into the signature. When a
    POST request body contains arguments they should be included in
    the signed data.
*/
void QOAuth1Signature::insert(const QString &key, const QVariant &value)
{
    d->parameters.insert(key, value);
}

/*!
    Retrieves the list of keys of parameters included in the signed
    data.
*/
QList<QString> QOAuth1Signature::keys() const
{
    return d->parameters.uniqueKeys();
}

/*!
    Removes \a key and any associated value from the signed data.
*/
QVariant QOAuth1Signature::take(const QString &key)
{
    return d->parameters.take(key);
}

/*!
    Returns the value associated with \a key, if present in the
    signed data, otherwise \a defaultValue.
*/
QVariant QOAuth1Signature::value(const QString &key, const QVariant &defaultValue) const
{
    return d->parameters.value(key, defaultValue);
}

/*!
    Returns the user secret used to generate the signature.
*/
QString QOAuth1Signature::clientSharedKey() const
{
    return d->clientSharedKey;
}

/*!
    Sets \a secret as the user secret used to generate the signature.
*/
void QOAuth1Signature::setClientSharedKey(const QString &secret)
{
    d->clientSharedKey = secret;
}

/*!
    Returns the negotiated secret used to generate the signature.
*/
QString QOAuth1Signature::tokenSecret() const
{
    return d->tokenSecret;
}

/*!
    Sets \a secret as the negotiated secret used to generate the
    signature.
*/
void QOAuth1Signature::setTokenSecret(const QString &secret)
{
    d->tokenSecret = secret;
}

/*!
    Generates the HMAC-SHA1 signature using the client shared secret
    and, where available, token secret.
*/
QByteArray QOAuth1Signature::hmacSha1() const
{
    QMessageAuthenticationCode code(QCryptographicHash::Sha1);
    code.setKey(d->secret());
    code.addData(d->signatureBaseString());
    return code.result();
}

/*!
    Generates the RSA-SHA1 signature.

    \note Currently this method is not supported.
*/
QByteArray QOAuth1Signature::rsaSha1() const
{
    qCCritical(loggingCategory, "RSA-SHA1 signing method not supported");
    return QByteArray();
}

/*!
    Generates the PLAINTEXT signature.
*/
QByteArray QOAuth1Signature::plainText() const
{
    return plainText(d->clientSharedKey, d->tokenSecret);
}

/*!
    Generates a PLAINTEXT signature from the client secret
    \a clientSharedKey and the token secret \a tokenSecret.
*/
QByteArray QOAuth1Signature::plainText(const QString &clientSharedKey,
                                       const QString &tokenSecret)
{
    QByteArray ret;
    ret += clientSharedKey.toUtf8() + '&' + tokenSecret.toUtf8();
    return ret;
}

/*!
    Swaps signature \a other with this signature. This operation is
    very fast and never fails.
*/
void QOAuth1Signature::swap(QOAuth1Signature &other)
{
    qSwap(d, other.d);
}

QOAuth1Signature &QOAuth1Signature::operator=(const QOAuth1Signature &other)
{
    if (d != other.d) {
        QOAuth1Signature tmp(other);
        tmp.swap(*this);
    }
    return *this;
}

/*!
    Move-assignment operator.
*/
QOAuth1Signature &QOAuth1Signature::operator=(QOAuth1Signature &&other)
{
    QOAuth1Signature moved(std::move(other));
    swap(moved);
    return *this;
}

QT_END_NAMESPACE
