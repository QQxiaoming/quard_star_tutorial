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

#include <qabstractoauth2.h>
#include <private/qabstractoauth2_p.h>

#include <QtCore/qurl.h>
#include <QtCore/qurlquery.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qmessageauthenticationcode.h>

#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qhttpmultipart.h>

QT_BEGIN_NAMESPACE

/*!
    \class QAbstractOAuth2
    \inmodule QtNetworkAuth
    \ingroup oauth
    \brief The QAbstractOAuth2 class is the base of all
    implementations of OAuth 2 authentication methods.
    \since 5.8

    The class defines the basic interface of the OAuth 2
    authentication classes. By inheriting this class, you
    can create custom authentication methods using the OAuth 2
    standard for different web services.

    A description of how OAuth 2 works can be found in:
    \l {https://tools.ietf.org/html/rfc6749}{The OAuth 2.0
    Authorization Framework}
*/
/*!
    \property QAbstractOAuth2::scope
    \brief This property holds the desired scope which defines the
    permissions requested by the client.
*/

/*!
    \property QAbstractOAuth2::userAgent
    This property holds the User-Agent header used to create the
    network requests.

    The default value is "QtOAuth/1.0 (+https://www.qt.io)".
*/

/*!
    \property QAbstractOAuth2::clientIdentifierSharedKey
    This property holds the client shared key used as a password if
    the server requires authentication to request the token.
*/

/*!
    \property QAbstractOAuth2::state
    This property holds the string sent to the server during
    authentication. The state is used to identify and validate the
    request when the callback is received.
*/

/*!
    \property QAbstractOAuth2::expiration
    This property holds the expiration time of the current access
    token.
*/

/*!
    \fn QAbstractOAuth2::error(const QString &error, const QString &errorDescription, const QUrl &uri)

    Signal emitted when the server responds to the request with an
    error: \a error is the name of the error; \a errorDescription describes
    the error and \a uri is an optional URI containing more
    information about the error.
*/

/*!
    \fn QAbstractOAuth2::authorizationCallbackReceived(const QVariantMap &data)

    Signal emitted when the reply server receives the authorization
    callback from the server: \a data contains the values received
    from the server.
*/

using Key = QAbstractOAuth2Private::OAuth2KeyString;
const QString Key::accessToken =        QStringLiteral("access_token");
const QString Key::apiKey =             QStringLiteral("api_key");
const QString Key::clientIdentifier =   QStringLiteral("client_id");
const QString Key::clientSharedSecret = QStringLiteral("client_secret");
const QString Key::code =               QStringLiteral("code");
const QString Key::error =              QStringLiteral("error");
const QString Key::errorDescription =   QStringLiteral("error_description");
const QString Key::errorUri =           QStringLiteral("error_uri");
const QString Key::expiresIn =          QStringLiteral("expires_in");
const QString Key::grantType =          QStringLiteral("grant_type");
const QString Key::redirectUri =        QStringLiteral("redirect_uri");
const QString Key::refreshToken =       QStringLiteral("refresh_token");
const QString Key::responseType =       QStringLiteral("response_type");
const QString Key::scope =              QStringLiteral("scope");
const QString Key::state =              QStringLiteral("state");
const QString Key::tokenType =          QStringLiteral("token_type");

QAbstractOAuth2Private::QAbstractOAuth2Private(const QPair<QString, QString> &clientCredentials,
                                               const QUrl &authorizationUrl,
                                               QNetworkAccessManager *manager) :
    QAbstractOAuthPrivate("qt.networkauth.oauth2",
                          authorizationUrl,
                          clientCredentials.first,
                          manager),
    clientIdentifierSharedKey(clientCredentials.second)
{}

QAbstractOAuth2Private::~QAbstractOAuth2Private()
{}

QString QAbstractOAuth2Private::generateRandomState()
{
    return QString::fromUtf8(QAbstractOAuthPrivate::generateRandomString(8));
}

QNetworkRequest QAbstractOAuth2Private::createRequest(QUrl url, const QVariantMap *parameters)
{
    QUrlQuery query(url.query());

    QNetworkRequest request;
    if (parameters) {
        for (auto it = parameters->begin(), end = parameters->end(); it != end; ++it)
            query.addQueryItem(it.key(), it.value().toString());
        url.setQuery(query);
    } else { // POST, PUT request
        addContentTypeHeaders(&request);
    }

    request.setUrl(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, userAgent);
    const QString bearer = bearerFormat.arg(token);
    request.setRawHeader("Authorization", bearer.toUtf8());
    return request;
}

/*!
    Constructs a QAbstractOAuth2 object using \a parent as parent.
*/
QAbstractOAuth2::QAbstractOAuth2(QObject *parent) :
    QAbstractOAuth2(nullptr, parent)
{}

/*!
    Constructs a QAbstractOAuth2 object using \a parent as parent and
    sets \a manager as the network access manager.
*/
QAbstractOAuth2::QAbstractOAuth2(QNetworkAccessManager *manager, QObject *parent) :
    QAbstractOAuth(*new QAbstractOAuth2Private(qMakePair(QString(), QString()),
                                               QUrl(),
                                               manager),
                   parent)
{}

QAbstractOAuth2::QAbstractOAuth2(QAbstractOAuth2Private &dd, QObject *parent) :
    QAbstractOAuth(dd, parent)
{}

void QAbstractOAuth2::setResponseType(const QString &responseType)
{
    Q_D(QAbstractOAuth2);
    if (d->responseType != responseType) {
        d->responseType = responseType;
        Q_EMIT responseTypeChanged(responseType);
    }
}

/*!
    Destroys the QAbstractOAuth2 instance.
*/
QAbstractOAuth2::~QAbstractOAuth2()
{}

/*!
    The returned URL is based on \a url, combining it with the given
    \a parameters and the access token.
*/
QUrl QAbstractOAuth2::createAuthenticatedUrl(const QUrl &url, const QVariantMap &parameters)
{
    Q_D(const QAbstractOAuth2);
    if (Q_UNLIKELY(d->token.isEmpty())) {
        qCWarning(d->loggingCategory, "Empty access token");
        return QUrl();
    }
    QUrl ret = url;
    QUrlQuery query(ret.query());
    query.addQueryItem(Key::accessToken, d->token);
    for (auto it = parameters.begin(), end = parameters.end(); it != end ;++it)
        query.addQueryItem(it.key(), it.value().toString());
    ret.setQuery(query);
    return ret;
}

/*!
    Sends an authenticated HEAD request and returns a new
    QNetworkReply. The \a url and \a parameters are used to create
    the request.

    \b {See also}: \l {https://tools.ietf.org/html/rfc2616#section-9.4}
    {Hypertext Transfer Protocol -- HTTP/1.1: HEAD}
*/
QNetworkReply *QAbstractOAuth2::head(const QUrl &url, const QVariantMap &parameters)
{
    Q_D(QAbstractOAuth2);
    QNetworkReply *reply = d->networkAccessManager()->head(d->createRequest(url, &parameters));
    connect(reply, &QNetworkReply::finished, [this, reply]() { emit finished(reply); });
    return reply;
}

/*!
    Sends an authenticated GET request and returns a new
    QNetworkReply. The \a url and \a parameters are used to create
    the request.

    \b {See also}: \l {https://tools.ietf.org/html/rfc2616#section-9.3}
    {Hypertext Transfer Protocol -- HTTP/1.1: GET}
*/
QNetworkReply *QAbstractOAuth2::get(const QUrl &url, const QVariantMap &parameters)
{
    Q_D(QAbstractOAuth2);
    QNetworkReply *reply = d->networkAccessManager()->get(d->createRequest(url, &parameters));
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
QNetworkReply *QAbstractOAuth2::post(const QUrl &url, const QVariantMap &parameters)
{
    Q_D(QAbstractOAuth2);
    const auto data = d->convertParameters(parameters);
    return post(url, data);
}

/*!
    \since 5.10

    \overload

    Sends an authenticated POST request and returns a new
    QNetworkReply. The \a url and \a data are used to create
    the request.

    \sa post(), {https://tools.ietf.org/html/rfc2616#section-9.6}
    {Hypertext Transfer Protocol -- HTTP/1.1: POST}
*/
QNetworkReply *QAbstractOAuth2::post(const QUrl &url, const QByteArray &data)
{
    Q_D(QAbstractOAuth2);
    QNetworkReply *reply = d->networkAccessManager()->post(d->createRequest(url), data);
    connect(reply, &QNetworkReply::finished, [this, reply]() { emit finished(reply); });
    return reply;
}

/*!
    \since 5.10

    \overload

    Sends an authenticated POST request and returns a new
    QNetworkReply. The \a url and \a multiPart are used to create
    the request.

    \sa post(), QHttpMultiPart, {https://tools.ietf.org/html/rfc2616#section-9.6}
    {Hypertext Transfer Protocol -- HTTP/1.1: POST}
*/
QNetworkReply *QAbstractOAuth2::post(const QUrl &url, QHttpMultiPart *multiPart)
{
    Q_D(QAbstractOAuth2);
    QNetworkReply *reply = d->networkAccessManager()->post(d->createRequest(url), multiPart);
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
QNetworkReply *QAbstractOAuth2::put(const QUrl &url, const QVariantMap &parameters)
{
    Q_D(QAbstractOAuth2);
    const auto data = d->convertParameters(parameters);
    return put(url, data);
}

/*!
    \since 5.10

    \overload

    Sends an authenticated PUT request and returns a new
    QNetworkReply. The \a url and \a data are used to create
    the request.

    \sa put(), {https://tools.ietf.org/html/rfc2616#section-9.6}
    {Hypertext Transfer Protocol -- HTTP/1.1: PUT}
*/
QNetworkReply *QAbstractOAuth2::put(const QUrl &url, const QByteArray &data)
{
    Q_D(QAbstractOAuth2);
    QNetworkReply *reply = d->networkAccessManager()->put(d->createRequest(url), data);
    connect(reply, &QNetworkReply::finished, std::bind(&QAbstractOAuth::finished, this, reply));
    return reply;
}

/*!
    \since 5.10

    \overload

    Sends an authenticated PUT request and returns a new
    QNetworkReply. The \a url and \a multiPart are used to create
    the request.

    \sa put(), QHttpMultiPart, {https://tools.ietf.org/html/rfc2616#section-9.6}
    {Hypertext Transfer Protocol -- HTTP/1.1: PUT}
*/
QNetworkReply *QAbstractOAuth2::put(const QUrl &url, QHttpMultiPart *multiPart)
{
    Q_D(QAbstractOAuth2);
    QNetworkReply *reply = d->networkAccessManager()->put(d->createRequest(url), multiPart);
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
QNetworkReply *QAbstractOAuth2::deleteResource(const QUrl &url, const QVariantMap &parameters)
{
    Q_D(QAbstractOAuth2);
    QNetworkReply *reply = d->networkAccessManager()->deleteResource(
                d->createRequest(url, &parameters));
    connect(reply, &QNetworkReply::finished, [this, reply]() { emit finished(reply); });
    return reply;
}

QString QAbstractOAuth2::scope() const
{
    Q_D(const QAbstractOAuth2);
    return d->scope;
}

void QAbstractOAuth2::setScope(const QString &scope)
{
    Q_D(QAbstractOAuth2);
    if (d->scope != scope) {
        d->scope = scope;
        Q_EMIT scopeChanged(scope);
    }
}

QString QAbstractOAuth2::userAgent() const
{
    Q_D(const QAbstractOAuth2);
    return d->userAgent;
}

void QAbstractOAuth2::setUserAgent(const QString &userAgent)
{
    Q_D(QAbstractOAuth2);
    if (d->userAgent != userAgent) {
        d->userAgent = userAgent;
        Q_EMIT userAgentChanged(userAgent);
    }
}

/*!
    Returns the \l {https://tools.ietf.org/html/rfc6749#section-3.1.1}
    {response_type} used.
*/
QString QAbstractOAuth2::responseType() const
{
    Q_D(const QAbstractOAuth2);
    return d->responseType;
}

QString QAbstractOAuth2::clientIdentifierSharedKey() const
{
    Q_D(const QAbstractOAuth2);
    return d->clientIdentifierSharedKey;
}

void QAbstractOAuth2::setClientIdentifierSharedKey(const QString &clientIdentifierSharedKey)
{
    Q_D(QAbstractOAuth2);
    if (d->clientIdentifierSharedKey != clientIdentifierSharedKey) {
        d->clientIdentifierSharedKey = clientIdentifierSharedKey;
        Q_EMIT clientIdentifierSharedKeyChanged(clientIdentifierSharedKey);
    }
}

QString QAbstractOAuth2::state() const
{
    Q_D(const QAbstractOAuth2);
    return d->state;
}

void QAbstractOAuth2::setState(const QString &state)
{
    Q_D(QAbstractOAuth2);
    if (state != d->state) {
        d->state = state;
        Q_EMIT stateChanged(state);
    }
}

QDateTime QAbstractOAuth2::expirationAt() const
{
    Q_D(const QAbstractOAuth2);
    return d->expiresAt;
}

/*!
    \brief Gets the current refresh token.

    Refresh tokens usually have longer lifespans than access tokens,
    so it makes sense to save them for later use.

    Returns the current refresh token or an empty string, if
    there is no refresh token available.
*/
QString QAbstractOAuth2::refreshToken() const
{
    Q_D(const QAbstractOAuth2);
    return  d->refreshToken;
}

/*!
   \brief Sets the new refresh token \a refreshToken to be used.

    A custom refresh token can be used to refresh the access token via this method and then
    the access token can be refreshed via QOAuth2AuthorizationCodeFlow::refreshAccessToken().

*/
void QAbstractOAuth2::setRefreshToken(const QString &refreshToken)
{
    Q_D(QAbstractOAuth2);
    if (d->refreshToken != refreshToken) {
        d->refreshToken = refreshToken;
        Q_EMIT refreshTokenChanged(refreshToken);
    }
}

QT_END_NAMESPACE

#endif // QT_NO_HTTP
