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

#include "cookie_monster_delegate_qt.h"

#include "base/bind.h"
#include "base/memory/ptr_util.h"
#include "content/public/browser/browser_thread.h"
#include "net/cookies/cookie_util.h"

#include "api/qwebenginecookiestore.h"
#include "api/qwebenginecookiestore_p.h"
#include "type_conversion.h"

namespace QtWebEngineCore {

static GURL sourceUrlForCookie(const QNetworkCookie &cookie) {
    QString urlFragment = QStringLiteral("%1%2").arg(cookie.domain()).arg(cookie.path());
    return net::cookie_util::CookieOriginToURL(urlFragment.toStdString(), /* is_https */ cookie.isSecure());
}

CookieMonsterDelegateQt::CookieMonsterDelegateQt()
    : m_client(0)
    , m_cookieMonster(nullptr)
{
}

CookieMonsterDelegateQt::~CookieMonsterDelegateQt()
{

}

void CookieMonsterDelegateQt::AddStore(net::CookieStore *store)
{
    std::unique_ptr<net::CookieChangeSubscription> sub =
        store->GetChangeDispatcher().AddCallbackForAllChanges(
            base::Bind(&CookieMonsterDelegateQt::OnCookieChanged,
                        // this object's destruction will deregister the subscription.
                        base::Unretained(this)));

    m_subscriptions.push_back(std::move(sub));
}

bool CookieMonsterDelegateQt::hasCookieMonster()
{
    return m_cookieMonster;
}

void CookieMonsterDelegateQt::getAllCookies(quint64 callbackId)
{
    net::CookieMonster::GetCookieListCallback callback =
        base::BindOnce(&CookieMonsterDelegateQt::GetAllCookiesCallbackOnIOThread, this, callbackId);

    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
                                     base::BindOnce(&CookieMonsterDelegateQt::GetAllCookiesOnIOThread, this, std::move(callback)));
}

void CookieMonsterDelegateQt::GetAllCookiesOnIOThread(net::CookieMonster::GetCookieListCallback callback)
{
    if (m_cookieMonster)
        m_cookieMonster->GetAllCookiesAsync(std::move(callback));
}

void CookieMonsterDelegateQt::setCookie(quint64 callbackId, const QNetworkCookie &cookie, const QUrl &origin)
{
    Q_ASSERT(hasCookieMonster());
    Q_ASSERT(m_client);

    net::CookieStore::SetCookiesCallback callback;
    if (callbackId != CallbackDirectory::NoCallbackId)
        callback = base::BindOnce(&CookieMonsterDelegateQt::SetCookieCallbackOnIOThread, this, callbackId);

    GURL gurl = origin.isEmpty() ? sourceUrlForCookie(cookie) : toGurl(origin);

    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
                                     base::BindOnce(&CookieMonsterDelegateQt::SetCookieOnIOThread, this,
                                                    gurl, cookie.toRawForm().toStdString(), std::move(callback)));
}

void CookieMonsterDelegateQt::SetCookieOnIOThread(
        const GURL& url, const std::string& cookie_line,
        net::CookieMonster::SetCookiesCallback callback)
{
    net::CookieOptions options;
    options.set_include_httponly();

    if (m_cookieMonster)
        m_cookieMonster->SetCookieWithOptionsAsync(url, cookie_line, options, std::move(callback));
}

void CookieMonsterDelegateQt::deleteCookie(const QNetworkCookie &cookie, const QUrl &origin)
{
    Q_ASSERT(hasCookieMonster());
    Q_ASSERT(m_client);

    GURL gurl = origin.isEmpty() ? sourceUrlForCookie(cookie) : toGurl(origin);

    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
                                     base::BindOnce(&CookieMonsterDelegateQt::DeleteCookieOnIOThread, this,
                                                    gurl, cookie.name().toStdString()));
}

void CookieMonsterDelegateQt::DeleteCookieOnIOThread(const GURL& url, const std::string& cookie_name)
{
    if (m_cookieMonster)
        m_cookieMonster->DeleteCookieAsync(url, cookie_name, base::Closure());
}

void CookieMonsterDelegateQt::deleteSessionCookies(quint64 callbackId)
{
    Q_ASSERT(hasCookieMonster());
    Q_ASSERT(m_client);

    net::CookieMonster::DeleteCallback callback =
        base::BindOnce(&CookieMonsterDelegateQt::DeleteCookiesCallbackOnIOThread, this, callbackId);
    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
                                     base::BindOnce(&CookieMonsterDelegateQt::DeleteSessionCookiesOnIOThread, this, std::move(callback)));
}

void CookieMonsterDelegateQt::DeleteSessionCookiesOnIOThread(net::CookieMonster::DeleteCallback callback)
{
    if (m_cookieMonster)
        m_cookieMonster->DeleteSessionCookiesAsync(std::move(callback));
}

void CookieMonsterDelegateQt::deleteAllCookies(quint64 callbackId)
{
    Q_ASSERT(hasCookieMonster());
    Q_ASSERT(m_client);

    net::CookieMonster::DeleteCallback callback =
        base::BindOnce(&CookieMonsterDelegateQt::DeleteCookiesCallbackOnIOThread, this, callbackId);
    content::BrowserThread::PostTask(content::BrowserThread::IO, FROM_HERE,
                                     base::BindOnce(&CookieMonsterDelegateQt::DeleteAllOnIOThread, this, std::move(callback)));
}

void CookieMonsterDelegateQt::DeleteAllOnIOThread(net::CookieMonster::DeleteCallback callback)
{
    if (m_cookieMonster)
        m_cookieMonster->DeleteAllAsync(std::move(callback));
}

void CookieMonsterDelegateQt::setCookieMonster(net::CookieMonster* monster)
{
    if (monster == m_cookieMonster)
        return;

    m_subscriptions.clear();
    if (monster)
        AddStore(monster);

    m_cookieMonster = monster;

    if (!m_client)
        return;

    if (monster)
        m_client->d_func()->processPendingUserCookies();
    else
        m_client->d_func()->rejectPendingUserCookies();
}

void CookieMonsterDelegateQt::setClient(QWebEngineCookieStore *client)
{
    m_client = client;

    if (!m_client)
        return;

    m_client->d_func()->delegate = this;

    if (hasCookieMonster())
        m_client->d_func()->processPendingUserCookies();
}

bool CookieMonsterDelegateQt::canSetCookie(const QUrl &firstPartyUrl, const QByteArray &/*cookieLine*/, const QUrl &url) const
{
    if (!m_client)
        return true;

    return m_client->d_func()->canAccessCookies(firstPartyUrl, url);
}

bool CookieMonsterDelegateQt::canGetCookies(const QUrl &firstPartyUrl, const QUrl &url) const
{
    if (!m_client)
        return true;

    return m_client->d_func()->canAccessCookies(firstPartyUrl, url);
}

void CookieMonsterDelegateQt::OnCookieChanged(const net::CanonicalCookie& cookie, net::CookieChangeCause cause)
{
    if (!m_client)
        return;
    m_client->d_func()->onCookieChanged(toQt(cookie), cause != net::CookieChangeCause::INSERTED);
}

void CookieMonsterDelegateQt::GetAllCookiesCallbackOnIOThread(qint64 callbackId, const net::CookieList &cookies)
{
    QByteArray rawCookies;
    for (auto &&cookie : cookies)
        rawCookies += toQt(cookie).toRawForm() % QByteArrayLiteral("\n");

    content::BrowserThread::PostTask(
        content::BrowserThread::UI,
        FROM_HERE,
        base::BindOnce(&CookieMonsterDelegateQt::GetAllCookiesCallbackOnUIThread, this, callbackId, rawCookies));
}

void CookieMonsterDelegateQt::SetCookieCallbackOnIOThread(qint64 callbackId, bool success)
{
    content::BrowserThread::PostTask(
        content::BrowserThread::UI,
        FROM_HERE,
        base::BindOnce(&CookieMonsterDelegateQt::SetCookieCallbackOnUIThread, this, callbackId, success));
}

void CookieMonsterDelegateQt::DeleteCookiesCallbackOnIOThread(qint64 callbackId, uint numCookies)
{
    content::BrowserThread::PostTask(
        content::BrowserThread::UI,
        FROM_HERE,
        base::BindOnce(&CookieMonsterDelegateQt::DeleteCookiesCallbackOnUIThread, this, callbackId, numCookies));
}

void CookieMonsterDelegateQt::GetAllCookiesCallbackOnUIThread(qint64 callbackId, const QByteArray &cookies)
{
    if (m_client)
        m_client->d_func()->onGetAllCallbackResult(callbackId, cookies);
}

void CookieMonsterDelegateQt::SetCookieCallbackOnUIThread(qint64 callbackId, bool success)
{
    if (m_client)
        m_client->d_func()->onSetCallbackResult(callbackId, success);
}

void CookieMonsterDelegateQt::DeleteCookiesCallbackOnUIThread(qint64 callbackId, uint numCookies)
{
    if (m_client)
        m_client->d_func()->onDeleteCallbackResult(callbackId, numCookies);
}
}
