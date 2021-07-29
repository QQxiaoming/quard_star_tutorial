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

#include "base/callback.h"

#include "ssl_host_state_delegate_qt.h"

#include "type_conversion.h"

namespace QtWebEngineCore {

// Mirrors implementation in aw_ssl_host_state_delegate.cc

CertPolicy::CertPolicy()
{
}

CertPolicy::~CertPolicy()
{
}

// For an allowance, we consider a given |cert| to be a match to a saved
// allowed cert if the |error| is an exact match to or subset of the errors
// in the saved CertStatus.
bool CertPolicy::Check(const net::X509Certificate &cert, int error) const
{
    net::SHA256HashValue fingerprint = cert.CalculateChainFingerprint256();
    auto allowed_iter = m_allowed.find(fingerprint);
    if ((allowed_iter != m_allowed.end()) && (allowed_iter->second & error) && ((allowed_iter->second & error) == error))
        return true;
    return false;
}

void CertPolicy::Allow(const net::X509Certificate& cert, int error)
{
    net::SHA256HashValue fingerprint = cert.CalculateChainFingerprint256();
    m_allowed[fingerprint] |= error;
}

SSLHostStateDelegateQt::SSLHostStateDelegateQt()
{
}

SSLHostStateDelegateQt::~SSLHostStateDelegateQt()
{
}

void SSLHostStateDelegateQt::AllowCert(const std::string &host, const net::X509Certificate &cert, int error)
{
    m_certPolicyforHost[host].Allow(cert, error);
}

// Clear all allow preferences.
void SSLHostStateDelegateQt::Clear(const base::Callback<bool(const std::string&)>& host_filter)
{
    if (host_filter.is_null()) {
        m_certPolicyforHost.clear();
        return;
    }

    for (auto it = m_certPolicyforHost.begin(); it != m_certPolicyforHost.end();) {
        auto next_it = std::next(it);

        if (host_filter.Run(it->first))
            m_certPolicyforHost.erase(it);

        it = next_it;
    }
}

// Queries whether |cert| is allowed for |host| and |error|. Returns true in
// |expired_previous_decision| if a previous user decision expired immediately
// prior to this query, otherwise false.
content::SSLHostStateDelegate::CertJudgment SSLHostStateDelegateQt::QueryPolicy(
                                                       const std::string &host, const net::X509Certificate &cert,
                                                       int error, bool */*expired_previous_decision*/)
{
    return m_certPolicyforHost[host].Check(cert, error) ? SSLHostStateDelegate::ALLOWED : SSLHostStateDelegate::DENIED;
}

// Records that a host has run insecure content.
void SSLHostStateDelegateQt::HostRanInsecureContent(const std::string &host, int pid, InsecureContentType content_type)
{
}

// Returns whether the specified host ran insecure content.
bool SSLHostStateDelegateQt::DidHostRunInsecureContent(const std::string &host, int pid, InsecureContentType content_type) const
{
    return false;
}

// Revokes all SSL certificate error allow exceptions made by the user for
// |host|.
void SSLHostStateDelegateQt::RevokeUserAllowExceptions(const std::string &host)
{
    m_certPolicyforHost.erase(host);
}

// Returns whether the user has allowed a certificate error exception for
// |host|. This does not mean that *all* certificate errors are allowed, just
// that there exists an exception. To see if a particular certificate and
// error combination exception is allowed, use QueryPolicy().
bool SSLHostStateDelegateQt::HasAllowException(const std::string &host) const
{
    auto policy_iterator = m_certPolicyforHost.find(host);
    return policy_iterator != m_certPolicyforHost.end() &&
           policy_iterator->second.HasAllowException();
}


} // namespace QtWebEngineCore
