/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "client_cert_select_controller.h"

#include <base/bind.h>
#include <content/public/browser/client_certificate_delegate.h>
#include <net/cert/x509_certificate.h>
#include <net/ssl/client_cert_identity.h>
#include <net/ssl/ssl_cert_request_info.h>
#include <net/ssl/ssl_info.h>

#include "type_conversion.h"

QT_BEGIN_NAMESPACE

using namespace QtWebEngineCore;

ClientCertSelectController::ClientCertSelectController(net::SSLCertRequestInfo *certRequestInfo,
                                                       std::vector<std::unique_ptr<net::ClientCertIdentity>> clientCerts,
                                                       std::unique_ptr<content::ClientCertificateDelegate> delegate)
            : m_clientCerts(std::move(clientCerts))
            , m_delegate(std::move(delegate))
            , m_selected(false)
{
    m_hostAndPort.setHost(QString::fromStdString(certRequestInfo->host_and_port.HostForURL()));
    m_hostAndPort.setPort(certRequestInfo->host_and_port.port());
}

ClientCertSelectController::~ClientCertSelectController()
{
    // Continue without a client certificate, for instance if the app has not
    // implemented support for client certificate selection.
    if (!m_selected)
        m_delegate->ContinueWithCertificate(nullptr, nullptr);
}

#if !defined(QT_NO_SSL) || QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)

void ClientCertSelectController::selectNone()
{
    if (m_selected) {
        qWarning() << "ClientCertSelectController::selectNone() certicate already selected";
        return;
    }
    m_selected = true;
    m_delegate->ContinueWithCertificate(nullptr, nullptr);
}

void ClientCertSelectController::select(const QSslCertificate &certificate)
{
    if (m_selected) {
        qWarning() << "ClientCertSelectController::select() certicate already selected";
        return;
    }
    QByteArray derCertificate = certificate.toDer();
    scoped_refptr<net::X509Certificate> selectedCert =
            net::X509Certificate::CreateFromBytes(derCertificate.constData(), derCertificate.length());
    for (auto &certInfo : m_clientCerts) {
        scoped_refptr<net::X509Certificate> cert = certInfo->certificate();
        if (cert->EqualsExcludingChain(selectedCert.get())) {
            m_selected = true;
            net::ClientCertIdentity::SelfOwningAcquirePrivateKey(
                        std::move(certInfo),
                        base::Bind(&content::ClientCertificateDelegate::ContinueWithCertificate,
                                   base::Passed(std::move(m_delegate)), std::move(cert)));
            return;
        }
    }
    qWarning() << "ClientCertSelectController::select() - selected client certificate not recognized."
               << "    Selected certificate needs to be one of the offered";
}

QVector<QSslCertificate> ClientCertSelectController::certificates() const
{
    QVector<QSslCertificate> out;
    for (auto &cert : m_clientCerts) {
        std::vector<std::string> pem_encoded;
        if (cert->certificate()->GetPEMEncodedChain(&pem_encoded))
            out.append(QSslCertificate(QByteArray::fromStdString(pem_encoded.front())));
    }
    return out;
}

#endif // !defined(QT_NO_SSL) || QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)

QT_END_NAMESPACE
