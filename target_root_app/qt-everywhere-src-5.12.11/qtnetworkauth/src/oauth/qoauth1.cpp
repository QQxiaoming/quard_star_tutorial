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

#include "qoauth1.h"
#include "qoauth1_p.h"
#include "qoauth1signature.h"
#include "qoauthoobreplyhandler.h"
#include "qoauthhttpserverreplyhandler.h"

#include <QtCore/qmap.h>
#include <QtCore/qlist.h>
#include <QtCore/qvariant.h>
#include <QtCore/qurlquery.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qmessageauthenticationcode.h>

#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qnetworkaccessmanager.h>

QT_BEGIN_NAMESPACE

/*!
    \class QOAuth1
    \inmodule QtNetworkAuth
    \ingroup oauth
    \brief The QOAuth1 class provides an implementation of the
    \l {https://tools.ietf.org/html/rfc5849}{OAuth 1 Protocol}.
    \since 5.8

    QOAuth1 provides a method for clients to access server resources
    on behalf of a resource owner (such as a different client or an
    end-user). It also provides a process for end-users to authorize
    third-party access to their server resources without sharing
    their credentials (typically, a username and password pair),
    using user-agent redirections.

    QOAuth1 uses tokens to represent the authorization granted to the
    client by the resource owner.  Typically, token credentials are
    issued by the server at the resource owner's request, after
    authenticating the resource owner's identity (usually using a
    username and password).

    When making the temporary credentials request, the client
    authenticates using only the client credentials. When making the
    token request, the client authenticates using the client
    credentials as well as the temporary credentials. Once the
    client receives and stores the token credentials, it can
    proceed to access protected resources on behalf of the resource
    owner by making authenticated requests using the client
    credentials together with the token credentials received.
*/

/*!
    \enum QOAuth1::SignatureMethod

    Indicates the signature method to be used to sign requests.

    \value Hmac_Sha1
    \l {https://tools.ietf.org/html/rfc5849#section-3.4.2}
    {HMAC-SHA1} signature method.

    \value Rsa_Sha1
    \l {https://tools.ietf.org/html/rfc5849#section-3.4.3}
    {RSA-SHA1} signature method (not supported).

    \value PlainText
    \l {https://tools.ietf.org/html/rfc5849#section-3.4.4}
    {PLAINTEXT} signature method.
*/

using Key = QOAuth1Private::OAuth1KeyString;
const QString Key::oauthCallback =           QStringLiteral("oauth_callback");
const QString Key::oauthCallbackConfirmed =  QStringLiteral("oauth_callback_confirmed");
const QString Key::oauthConsumerKey =        QStringLiteral("oauth_consumer_key");
const QString Key::oauthNonce =              QStringLiteral("oauth_nonce");
const QString Key::oauthSignature =          QStringLiteral("oauth_signature");
const QString Key::oauthSignatureMethod =    QStringLiteral("oauth_signature_method");
const QString Key::oauthTimestamp =          QStringLiteral("oauth_timestamp");
const QString Key::oauthToken =              QStringLiteral("oauth_token");
const QString Key::oauthTokenSecret =        QStringLiteral("oauth_token_secret");
const QString Key::oauthVerifier =           QStringLiteral("oauth_verifier");
const QString Key::oauthVersion =            QStringLiteral("oauth_version");

QOAuth1Private::QOAuth1Private(const QPair<QString, QString> &clientCredentials,
                               QNetworkAccessManager *networkAccessManager) :
    QAbstractOAuthPrivate("qt.networkauth.oauth1",
                          QUrl(),
                          clientCredentials.first,
                          networkAccessManager),
    clientIdentifierSharedKey(clientCredentials.second)
{
    qRegisterMetaType<QNetworkReply::NetworkError>("QNetworkReply::NetworkError");
    qRegisterMetaType<QOAuth1::SignatureMethod>("QOAuth1::SignatureMethod");
}

void QOAuth1Private::appendCommonHeaders(QVariantMap *headers)
{
    const auto currentDateTime = QDateTime::currentDateTimeUtc();

    headers->insert(Key::oauthNonce, QOAuth1::nonce());
    headers->insert(Key::oauthConsumerKey, clientIdentifier);
    headers->insert(Key::oauthTimestamp, QString::number(currentDateTime.toTime_t()));
    headers->insert(Key::oauthVersion, oauthVersion);
    headers->insert(Key::oauthSignatureMethod, signatureMethodString().toUtf8());
}

void QOAuth1Private::appendSignature(QAbstractOAuth::Stage stage,
                                     QVariantMap *headers,
                                     const QUrl &url,
                                     QNetworkAccessManager::Operation operation,
                                     const QVariantMap parameters)
{
    QByteArray signature;
    {
        QVariantMap allParameters = QVariantMap(*headers).unite(parameters);
        if (modifyParametersFunction)
            modifyParametersFunction(stage, &allParameters);
        signature = generateSignature(allParameters, url, operation);
    }
    headers->insert(Key::oauthSignature, signature);
}

QNetworkReply *QOAuth1Private::requestToken(QNetworkAccessManager::Operation operation,
                                            const QUrl &url,
                                            const QPair<QString, QString> &token,
                                            const QVariantMap &parameters)
{
    if (Q_UNLIKELY(!networkAccessManager())) {
        qCWarning(loggingCategory, "QNetworkAccessManager not available");
        return nullptr;
    }
    if (Q_UNLIKELY(url.isEmpty())) {
        qCWarning(loggingCategory, "Request Url not set");
        return nullptr;
    }
    if (Q_UNLIKELY(operation != QNetworkAccessManager::GetOperation &&
                   operation != QNetworkAccessManager::PostOperation)) {
        qCWarning(loggingCategory, "Operation not supported");
        return nullptr;
    }

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    QAbstractOAuth::Stage stage = QAbstractOAuth::Stage::RequestingTemporaryCredentials;
    QVariantMap headers;
    QVariantMap remainingParameters;
    appendCommonHeaders(&headers);
    for (auto it = parameters.begin(), end = parameters.end(); it != end; ++it) {
        const auto key = it.key();
        const auto value = it.value();
        if (key.startsWith(QStringLiteral("oauth_")))
            headers.insert(key, value);
        else
            remainingParameters.insert(key, value);
    }
    if (!token.first.isEmpty()) {
        headers.insert(Key::oauthToken, token.first);
        stage = QAbstractOAuth::Stage::RequestingAccessToken;
    }
    appendSignature(stage, &headers, url, operation, remainingParameters);

    request.setRawHeader("Authorization", QOAuth1::generateAuthorizationHeader(headers));

    QNetworkReply *reply = nullptr;
    if (operation == QNetworkAccessManager::GetOperation) {
        if (parameters.size() > 0) {
            QUrl url = request.url();
            url.setQuery(QOAuth1Private::createQuery(remainingParameters));
            request.setUrl(url);
        }
        reply = networkAccessManager()->get(request);
    }
    else if (operation == QNetworkAccessManager::PostOperation) {
        QUrlQuery query = QOAuth1Private::createQuery(remainingParameters);
        const QByteArray data = query.toString(QUrl::FullyEncoded).toUtf8();
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("application/x-www-form-urlencoded"));
        reply = networkAccessManager()->post(request, data);
    }

    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &QOAuth1Private::_q_onTokenRequestError);

    QAbstractOAuthReplyHandler *handler = replyHandler ? replyHandler.data()
                                                       : defaultReplyHandler.data();
    QObject::connect(reply, &QNetworkReply::finished,
                     [handler, reply]() { handler->networkReplyFinished(reply); });
    connect(handler, &QAbstractOAuthReplyHandler::tokensReceived, this,
            &QOAuth1Private::_q_tokensReceived);

    return reply;
}

QString QOAuth1Private::signatureMethodString() const
{
    switch (signatureMethod) { // No default: intended
    case QOAuth1::SignatureMethod::PlainText:
        return QStringLiteral("PLAINTEXT");
    case QOAuth1::SignatureMethod::Hmac_Sha1:
        return QStringLiteral("HMAC-SHA1");
    case QOAuth1::SignatureMethod::Rsa_Sha1:
        qFatal("RSA-SHA1 signature method not supported");
        return QStringLiteral("RSA-SHA1");
    }
    qFatal("Invalid signature method");
    return QString();
}

QByteArray QOAuth1Private::generateSignature(const QVariantMap &parameters,
                                             const QUrl &url,
                                             QNetworkAccessManager::Operation operation) const
{
    const QOAuth1Signature signature(url,
                                     clientIdentifierSharedKey,
                                     tokenSecret,
                                     static_cast<QOAuth1Signature::HttpRequestMethod>(operation),
                                     parameters);

    switch (signatureMethod) {
    case QOAuth1::SignatureMethod::Hmac_Sha1:
        return signature.hmacSha1().toBase64();
    case QOAuth1::SignatureMethod::PlainText:
        return signature.plainText();
    default:
        qFatal("QOAuth1Private::generateSignature: Signature method not supported");
        return QByteArray();
    }
}

void QOAuth1Private::_q_onTokenRequestError(QNetworkReply::NetworkError error)
{
    Q_Q(QOAuth1);
    Q_UNUSED(error);
    Q_EMIT q->requestFailed(QAbstractOAuth::Error::NetworkError);
}

void QOAuth1Private::_q_tokensReceived(const QVariantMap &tokens)
{
    Q_Q(QOAuth1);

    if (!tokenRequested && status == QAbstractOAuth::Status::TemporaryCredentialsReceived) {
        // We didn't request a token yet, but in the "TemporaryCredentialsReceived" state _any_
        // new tokens received will count as a successful authentication and we move to the
        // 'Granted' state. To avoid this, 'status' will be temporarily set to 'NotAuthenticated'.
        status = QAbstractOAuth::Status::NotAuthenticated;
    }
    if (tokenRequested) // 'Reset' tokenRequested now that we've gotten new tokens
        tokenRequested = false;

    QPair<QString, QString> credential(tokens.value(Key::oauthToken).toString(),
                                       tokens.value(Key::oauthTokenSecret).toString());
    switch (status) {
    case QAbstractOAuth::Status::NotAuthenticated:
        if (tokens.value(Key::oauthCallbackConfirmed, true).toBool()) {
            q->setTokenCredentials(credential);
            setStatus(QAbstractOAuth::Status::TemporaryCredentialsReceived);
        } else {
            Q_EMIT q->requestFailed(QAbstractOAuth::Error::OAuthCallbackNotVerified);
        }
        break;
    case QAbstractOAuth::Status::TemporaryCredentialsReceived:
        q->setTokenCredentials(credential);
        setStatus(QAbstractOAuth::Status::Granted);
        break;
    case QAbstractOAuth::Status::Granted:
    case QAbstractOAuth::Status::RefreshingToken:
        break;
    }
}

/*!
    Constructs a QOAuth1 object with parent object \a parent.
*/
QOAuth1::QOAuth1(QObject *parent) :
    QOAuth1(nullptr,
            parent)
{}

/*!
    Constructs a QOAuth1 object with parent object \a parent, using
    \a manager to access the network.
*/
QOAuth1::QOAuth1(QNetworkAccessManager *manager, QObject *parent) :
    QOAuth1(QString(),
            QString(),
            manager,
            parent)
{}

/*!
    Constructs a QOAuth1 object with parent object \a parent, using
    \a manager to access the network.
    Also sets \a clientIdentifier and \a clientSharedSecret to sign
    the calls to the web server and identify the application.
*/
QOAuth1::QOAuth1(const QString &clientIdentifier,
                 const QString &clientSharedSecret,
                 QNetworkAccessManager *manager,
                 QObject *parent)
    : QAbstractOAuth(*new QOAuth1Private(qMakePair(clientIdentifier, clientSharedSecret),
                                         manager),
                     parent)
{}

/*!
    Returns the current shared secret used to sign requests to
    the web server.

    \sa setClientSharedSecret(), clientCredentials()
*/
QString QOAuth1::clientSharedSecret() const
{
    Q_D(const QOAuth1);
    return d->clientIdentifierSharedKey;
}

/*!
    Sets \a clientSharedSecret as the string used to sign the
    requests to the web server.

    \sa clientSharedSecret(), setClientCredentials()
*/
void QOAuth1::setClientSharedSecret(const QString &clientSharedSecret)
{
    Q_D(QOAuth1);
    if (d->clientIdentifierSharedKey != clientSharedSecret) {
        d->clientIdentifierSharedKey = clientSharedSecret;
        Q_EMIT clientSharedSecretChanged(clientSharedSecret);
    }
}

/*!
    Returns the pair of QString used to identify the application and
    sign requests to the web server.

    \sa setClientCredentials()
*/
QPair<QString, QString> QOAuth1::clientCredentials() const
{
    Q_D(const QOAuth1);
    return qMakePair(d->clientIdentifier, d->clientIdentifierSharedKey);
}

/*!
    Sets \a clientCredentials as the pair of QString used to identify
    the application and sign requests to the web server.

    \sa clientCredentials()
*/
void QOAuth1::setClientCredentials(const QPair<QString, QString> &clientCredentials)
{
    setClientCredentials(clientCredentials.first, clientCredentials.second);
}

/*!
    Sets \a clientIdentifier and \a clientSharedSecret as the pair of
    QString used to identify the application and sign requests to the
    web server. \a clientIdentifier identifies the application and
    \a clientSharedSecret is used to sign requests.

    \sa clientCredentials()
*/
void QOAuth1::setClientCredentials(const QString &clientIdentifier,
                                   const QString &clientSharedSecret)
{
    setClientIdentifier(clientIdentifier);
    setClientSharedSecret(clientSharedSecret);
}

/*!
    Returns the current token secret used to sign authenticated
    requests to the web server.

    \sa setTokenSecret(), tokenCredentials()
*/
QString QOAuth1::tokenSecret() const
{
    Q_D(const QOAuth1);
    return d->tokenSecret;
}
/*!
    Sets \a tokenSecret as the current token secret used to sign
    authenticated calls to the web server.

    \sa tokenSecret(), setTokenCredentials()
*/
void QOAuth1::setTokenSecret(const QString &tokenSecret)
{
    Q_D(QOAuth1);
    if (d->tokenSecret != tokenSecret) {
        d->tokenSecret = tokenSecret;
        Q_EMIT tokenSecretChanged(tokenSecret);
    }
}

/*!
    Returns the pair of QString used to identify and sign
    authenticated requests to the web server.

    \sa setTokenCredentials()
*/
QPair<QString, QString> QOAuth1::tokenCredentials() const
{
    Q_D(const QOAuth1);
    return qMakePair(d->token, d->tokenSecret);
}

/*!
    Sets \a tokenCredentials as the pair of QString used to identify
    and sign authenticated requests to the web server.

    \sa tokenCredentials()
*/
void QOAuth1::setTokenCredentials(const QPair<QString, QString> &tokenCredentials)
{
    setTokenCredentials(tokenCredentials.first, tokenCredentials.second);
}

/*!
    Sets \a token and \a tokenSecret as the pair of QString used to
    identify and sign authenticated requests to the web server.
    Once the client receives and stores the token credentials, it can
    proceed to access protected resources on behalf of the resource
    owner by making authenticated requests using the client
    credentials together with the token credentials received.

    \sa tokenCredentials()
*/
void QOAuth1::setTokenCredentials(const QString &token, const QString &tokenSecret)
{
    setToken(token);
    setTokenSecret(tokenSecret);
}

/*!
    Returns the url used to request temporary credentials to
    start the authentication process.

    \sa setTemporaryCredentialsUrl()
*/
QUrl QOAuth1::temporaryCredentialsUrl() const
{
    Q_D(const QOAuth1);
    return d->temporaryCredentialsUrl;
}

/*!
    Sets \a url as the URL to request temporary credentials to
    start the authentication process.

    \sa temporaryCredentialsUrl()
*/
void QOAuth1::setTemporaryCredentialsUrl(const QUrl &url)
{
    Q_D(QOAuth1);
    if (d->temporaryCredentialsUrl != url) {
        d->temporaryCredentialsUrl = url;
        Q_EMIT temporaryCredentialsUrlChanged(url);
    }
}

/*!
    Returns the url used to request token credentials to continue
    the authentication process.

    \sa setTokenCredentialsUrl()
*/
QUrl QOAuth1::tokenCredentialsUrl() const
{
    Q_D(const QOAuth1);
    return d->tokenCredentialsUrl;
}

/*!
    Sets \a url as the URL to request the token credentials to
    continue the authentication process.

    \sa tokenCredentialsUrl()
*/
void QOAuth1::setTokenCredentialsUrl(const QUrl &url)
{
    Q_D(QOAuth1);
    if (d->tokenCredentialsUrl != url) {
        d->tokenCredentialsUrl = url;
        Q_EMIT tokenCredentialsUrlChanged(url);
    }
}

/*!
    Returns the method used to sign the request to the web server.

    \sa setSignatureMethod()
*/
QOAuth1::SignatureMethod QOAuth1::signatureMethod() const
{
    Q_D(const QOAuth1);
    return d->signatureMethod;
}

/*!
    Sets \a value as the method used to sign requests to the web
    server.

    \sa signatureMethod()
*/
void QOAuth1::setSignatureMethod(QOAuth1::SignatureMethod value)
{
    Q_D(QOAuth1);
    if (d->signatureMethod != value) {
        d->signatureMethod = value;
        Q_EMIT signatureMethodChanged(value);
    }
}

/*!
    Sends an authenticated HEAD request and returns a new
    QNetworkReply. The \a url and \a parameters are used to create
    the request.

    \b {See also}: \l {https://tools.ietf.org/html/rfc2616#section-9.4}
    {Hypertext Transfer Protocol -- HTTP/1.1: HEAD}
*/
QNetworkReply *QOAuth1::head(const QUrl &url, const QVariantMap &parameters)
{
    Q_D(QOAuth1);
    if (!d->networkAccessManager()) {
        qCWarning(d->loggingCategory, "QNetworkAccessManager not available");
        return nullptr;
    }
    QNetworkRequest request(url);
    setup(&request, parameters, QNetworkAccessManager::HeadOperation);
    return d->networkAccessManager()->head(request);
}

/*!
    Sends an authenticated GET request and returns a new
    QNetworkReply. The \a url and \a parameters are used to create
    the request.

    \b {See also}: \l {https://tools.ietf.org/html/rfc2616#section-9.3}
    {Hypertext Transfer Protocol -- HTTP/1.1: GET}
*/
QNetworkReply *QOAuth1::get(const QUrl &url, const QVariantMap &parameters)
{
    Q_D(QOAuth1);
    if (!d->networkAccessManager()) {
        qCWarning(d->loggingCategory, "QNetworkAccessManager not available");
        return nullptr;
    }
    QNetworkRequest request(url);
    setup(&request, parameters, QNetworkAccessManager::GetOperation);
    QNetworkReply *reply = d->networkAccessManager()->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply]() { emit finished(reply); });
    return reply;
}

/*!
    Sends an authenticated POST request and returns a new
    QNetworkReply. The \a url and \a parameters are used to create
    the request.

    \b {See also}: \l {https://tools.ietf.org/html/rfc2616#section-9.5}
    {Hypertext Transfer Protocol -- HTTP/1.1: POST}
*/
QNetworkReply *QOAuth1::post(const QUrl &url, const QVariantMap &parameters)
{
    Q_D(QOAuth1);
    if (!d->networkAccessManager()) {
        qCWarning(d->loggingCategory, "QNetworkAccessManager not available");
        return nullptr;
    }
    QNetworkRequest request(url);
    setup(&request, parameters, QNetworkAccessManager::PostOperation);
    d->addContentTypeHeaders(&request);

    const QByteArray data = d->convertParameters(parameters);
    QNetworkReply *reply = d->networkAccessManager()->post(request, data);
    connect(reply, &QNetworkReply::finished, [this, reply]() { emit finished(reply); });
    return reply;
}

/*!
    Sends an authenticated PUT request and returns a new
    QNetworkReply. The \a url and \a parameters are used to create
    the request.

    \b {See also}: \l {https://tools.ietf.org/html/rfc2616#section-9.6}
    {Hypertext Transfer Protocol -- HTTP/1.1: PUT}
*/
QNetworkReply *QOAuth1::put(const QUrl &url, const QVariantMap &parameters)
{
    Q_D(QOAuth1);
    if (!d->networkAccessManager()) {
        qCWarning(d->loggingCategory, "QNetworkAccessManager not available");
        return nullptr;
    }
    QNetworkRequest request(url);
    setup(&request, parameters, QNetworkAccessManager::PutOperation);
    d->addContentTypeHeaders(&request);

    const QByteArray data = d->convertParameters(parameters);
    QNetworkReply *reply = d->networkAccessManager()->put(request, data);
    connect(reply, &QNetworkReply::finished, std::bind(&QAbstractOAuth::finished, this, reply));
    return reply;
}

/*!
    Sends an authenticated DELETE request and returns a new
    QNetworkReply. The \a url and \a parameters are used to create
    the request.

    \b {See also}: \l {https://tools.ietf.org/html/rfc2616#section-9.7}
    {Hypertext Transfer Protocol -- HTTP/1.1: DELETE}
*/
QNetworkReply *QOAuth1::deleteResource(const QUrl &url, const QVariantMap &parameters)
{
    Q_D(QOAuth1);
    if (!d->networkAccessManager()) {
        qCWarning(d->loggingCategory, "QNetworkAccessManager not available");
        return nullptr;
    }
    QNetworkRequest request(url);
    setup(&request, parameters, QNetworkAccessManager::DeleteOperation);
    QNetworkReply *reply = d->networkAccessManager()->deleteResource(request);
    connect(reply, &QNetworkReply::finished, [this, reply]() { emit finished(reply); });
    return reply;
}

/*!
    Starts the a request for temporary credentials using the request
    method \a operation. The request URL is \a url and the
    \a parameters shall encoded and sent during the request.

    \b {See also}: \l {https://tools.ietf.org/html/rfc5849#section-2.1}
    {The OAuth 1.0 Protocol: Temporary Credentials}
*/
QNetworkReply *QOAuth1::requestTemporaryCredentials(QNetworkAccessManager::Operation operation,
                                                    const QUrl &url,
                                                    const QVariantMap &parameters)
{
    Q_D(QOAuth1);
    d->token.clear();
    d->tokenSecret.clear();
    QVariantMap allParameters(parameters);
    allParameters.insert(Key::oauthCallback, callback());
    return d->requestToken(operation, url, qMakePair(d->token, d->tokenSecret), allParameters);
}

/*!
    Starts a request for token credentials using the request
    method \a operation. The request URL is \a url and the
    \a parameters shall be encoded and sent during the
    request. The \a temporaryToken pair of string is used to identify
    and sign the request.

    \b {See also}: \l {https://tools.ietf.org/html/rfc5849#section-2.3}
    {The OAuth 1.0 Protocol: Token Credentials}
*/
QNetworkReply *QOAuth1::requestTokenCredentials(QNetworkAccessManager::Operation operation,
                                                const QUrl &url,
                                                const QPair<QString, QString> &temporaryToken,
                                                const QVariantMap &parameters)
{
    Q_D(QOAuth1);
    d->tokenRequested = true;
    return d->requestToken(operation, url, temporaryToken, parameters);
}

/*!
    Signs the \a request using \a signingParameters and \a operation.
*/
void QOAuth1::setup(QNetworkRequest *request,
                    const QVariantMap &signingParameters,
                    QNetworkAccessManager::Operation operation)
{
    Q_D(const QOAuth1);

    QVariantMap oauthParams;

    const auto currentDateTime = QDateTime::currentDateTimeUtc();

    oauthParams.insert(Key::oauthConsumerKey, d->clientIdentifier);
    oauthParams.insert(Key::oauthVersion, QStringLiteral("1.0"));
    oauthParams.insert(Key::oauthToken, d->token);
    oauthParams.insert(Key::oauthSignatureMethod, d->signatureMethodString());
    oauthParams.insert(Key::oauthNonce, QOAuth1::nonce());
    oauthParams.insert(Key::oauthTimestamp, QString::number(currentDateTime.toTime_t()));

    // Add signature parameter
    {
        const auto parameters = QVariantMap(oauthParams).unite(signingParameters);
        const auto signature = d->generateSignature(parameters, request->url(), operation);
        oauthParams.insert(Key::oauthSignature, signature);
    }

    if (operation == QNetworkAccessManager::GetOperation) {
        if (signingParameters.size()) {
            QUrl url = request->url();
            QUrlQuery query = QUrlQuery(url.query());
            for (auto it = signingParameters.begin(), end = signingParameters.end(); it != end;
                 ++it)
                query.addQueryItem(it.key(), it.value().toString());
            url.setQuery(query);
            request->setUrl(url);
        }
    }

    request->setRawHeader("Authorization", generateAuthorizationHeader(oauthParams));

    if (operation == QNetworkAccessManager::PostOperation
            || operation == QNetworkAccessManager::PutOperation)
        request->setHeader(QNetworkRequest::ContentTypeHeader,
                           QStringLiteral("application/x-www-form-urlencoded"));
}

/*!
    Generates a nonce.

    \b {See also}: \l {https://tools.ietf.org/html/rfc5849#section-3.3}
    {The OAuth 1.0 Protocol: Nonce and Timestamp}
*/
QByteArray QOAuth1::nonce()
{
    return QAbstractOAuth::generateRandomString(8);
}

/*!
    Generates an authorization header using \a oauthParams.

    \b {See also}: \l {https://tools.ietf.org/html/rfc5849#section-3.5.1}
    {The OAuth 1.0 Protocol: Authorization Header}
*/
QByteArray QOAuth1::generateAuthorizationHeader(const QVariantMap &oauthParams)
{
    // TODO Add realm parameter support
    bool first = true;
    QString ret(QStringLiteral("OAuth "));
    QVariantMap headers(oauthParams);
    for (auto it = headers.begin(), end = headers.end(); it != end; ++it) {
        if (first)
            first = false;
        else
            ret += QLatin1String(",");
        ret += it.key() +
               QLatin1String("=\"") +
               QString::fromUtf8(QUrl::toPercentEncoding(it.value().toString())) +
               QLatin1Char('\"');
    }
    return ret.toUtf8();
}

/*!
    Starts the Redirection-Based Authorization flow.

    \note For an out-of-band reply handler, a verifier string is
    received after the call to this function; pass that to
    continueGrantWithVerifier() to continue the grant process.

    \sa continueGrantWithVerifier()

    \b {See also}: \l {https://tools.ietf.org/html/rfc5849#section-2}
    {The OAuth 1.0 Protocol: Redirection-Based Authorization}
*/
void QOAuth1::grant()
{
    Q_D(QOAuth1);
    using Key = QOAuth1Private::OAuth1KeyString;

    if (d->temporaryCredentialsUrl.isEmpty()) {
        qCWarning(d->loggingCategory, "requestTokenUrl is empty");
        return;
    }
    if (d->tokenCredentialsUrl.isEmpty()) {
        qCWarning(d->loggingCategory, "authorizationGrantUrl is empty");
        return;
    }
    if (!d->token.isEmpty() && status() == Status::Granted) {
        qCWarning(d->loggingCategory, "Already authenticated");
        return;
    }

    QMetaObject::Connection connection;
    connection = connect(this, &QAbstractOAuth::statusChanged, [&](Status status) {
        Q_D(QOAuth1);

        if (status == Status::TemporaryCredentialsReceived) {
            if (d->authorizationUrl.isEmpty()) {
                // try upgrading token without verifier
                auto reply = requestTokenCredentials(QNetworkAccessManager::PostOperation,
                                                     d->tokenCredentialsUrl,
                                                     qMakePair(d->token, d->tokenSecret));
                connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
            } else {
                QVariantMap parameters;
                parameters.insert(Key::oauthToken, d->token);
                if (d->modifyParametersFunction)
                    d->modifyParametersFunction(Stage::RequestingAuthorization, &parameters);

                // https://tools.ietf.org/html/rfc5849#section-2.2
                resourceOwnerAuthorization(d->authorizationUrl, parameters);
            }
        } else if (status == Status::NotAuthenticated) {
            // Inherit class called QAbstractOAuth::setStatus(Status::NotAuthenticated);
            setTokenCredentials(QString(), QString());
            disconnect(connection);
        }
    });

    auto httpReplyHandler = qobject_cast<QOAuthHttpServerReplyHandler*>(replyHandler());
    if (httpReplyHandler) {
        connect(httpReplyHandler, &QOAuthHttpServerReplyHandler::callbackReceived, [&](
                const QVariantMap &values) {
            QString verifier = values.value(Key::oauthVerifier).toString();
            if (verifier.isEmpty()) {
                qCWarning(d->loggingCategory, "%s not found in the callback",
                          qPrintable(Key::oauthVerifier));
                return;
            }
            continueGrantWithVerifier(verifier);
        });
    }

    // requesting temporary credentials
    auto reply = requestTemporaryCredentials(QNetworkAccessManager::PostOperation,
                                             d->temporaryCredentialsUrl);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}

/*!
    Continues the Redirection-Based Authorization flow using
    \a verifier. Call this function when using an Out-of-band reply
    handler to supply the verifier provided by the web server.
*/
void QOAuth1::continueGrantWithVerifier(const QString &verifier)
{
    // https://tools.ietf.org/html/rfc5849#section-2.3
    Q_D(QOAuth1);

    QVariantMap parameters;
    parameters.insert(Key::oauthVerifier, verifier);
    auto reply = requestTokenCredentials(QNetworkAccessManager::PostOperation,
                                         d->tokenCredentialsUrl,
                                         qMakePair(d->token, d->tokenSecret),
                                         parameters);
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}

QT_END_NAMESPACE

#endif // QT_NO_HTTP
