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

#include "qwebengineurlscheme.h"

#include "url/url_util_qt.h"

#include <QtDebug>

QT_BEGIN_NAMESPACE

ASSERT_ENUMS_MATCH(QWebEngineUrlScheme::Syntax::Path, url::SCHEME_WITHOUT_AUTHORITY)
ASSERT_ENUMS_MATCH(QWebEngineUrlScheme::Syntax::Host, url::SCHEME_WITH_HOST)
ASSERT_ENUMS_MATCH(QWebEngineUrlScheme::Syntax::HostAndPort, url::SCHEME_WITH_HOST_AND_PORT)
ASSERT_ENUMS_MATCH(QWebEngineUrlScheme::Syntax::HostPortAndUserInformation,
                   url::SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION)

ASSERT_ENUMS_MATCH(QWebEngineUrlScheme::PortUnspecified, url::PORT_UNSPECIFIED)

ASSERT_ENUMS_MATCH(QWebEngineUrlScheme::SecureScheme, url::CustomScheme::Secure)
ASSERT_ENUMS_MATCH(QWebEngineUrlScheme::LocalScheme, url::CustomScheme::Local)
ASSERT_ENUMS_MATCH(QWebEngineUrlScheme::LocalAccessAllowed, url::CustomScheme::LocalAccessAllowed)
ASSERT_ENUMS_MATCH(QWebEngineUrlScheme::NoAccessAllowed, url::CustomScheme::NoAccessAllowed)
ASSERT_ENUMS_MATCH(QWebEngineUrlScheme::ServiceWorkersAllowed, url::CustomScheme::ServiceWorkersAllowed)
ASSERT_ENUMS_MATCH(QWebEngineUrlScheme::ViewSourceAllowed, url::CustomScheme::ViewSourceAllowed)
ASSERT_ENUMS_MATCH(QWebEngineUrlScheme::ContentSecurityPolicyIgnored, url::CustomScheme::ContentSecurityPolicyIgnored)

static bool g_schemesLocked = false;

class QWebEngineUrlSchemePrivate : public QSharedData
                                 , public url::CustomScheme
{
public:
    QWebEngineUrlSchemePrivate() {}
    QWebEngineUrlSchemePrivate(const url::CustomScheme &cs) : url::CustomScheme(cs) {}
    static QSharedDataPointer<QWebEngineUrlSchemePrivate> defaultConstructed()
    {
        static QSharedDataPointer<QWebEngineUrlSchemePrivate> instance(new QWebEngineUrlSchemePrivate);
        return instance;
    }
};

/*!
  \class QWebEngineUrlScheme
  \inmodule QtWebEngineCore
  \since 5.12
  \brief The QWebEngineUrlScheme class configures a custom URL scheme.

  A web engine URL scheme describes a URL scheme from the web engine's
  perspective, specifying how URLs of this scheme should be parsed, and which
  security restrictions should be placed on resources originating from such
  URLs.

  Custom URL schemes must be configured early at application startup, before
  creating any \QWE classes. In general this means the schemes need to be configured before
  a QGuiApplication or QApplication instance is created.

  Every registered scheme configuration applies globally to all profiles.

  \code
  int main(int argc, char **argv)
  {
      QWebEngineUrlScheme scheme("myscheme");
      scheme.setSyntax(QWebEngineUrlScheme::Syntax::HostAndPort);
      scheme.setDefaultPort(2345);
      scheme.setFlags(QWebEngineUrlScheme::SecureScheme);
      QWebEngineUrlScheme::registerScheme(scheme);
      ...
  }
  \endcode

  To actually make use of the custom URL scheme, a \l QWebEngineUrlSchemeHandler
  must be created and registered in a profile.

  \sa QWebEngineUrlSchemeHandler
*/

/*!
  \enum QWebEngineUrlScheme::Syntax

  This enum type lists types of URL syntax.

  To apply the same-origin policy to a custom URL scheme, WebEngine must be able
  to compute the origin (host and port combination) of a URL. The \c {Host...}
  options indicate that the URL scheme conforms to the standard URL syntax (like
  \c http) and automatically enable the same-origin policy. The \c {Path}
  option indicates that the URL scheme uses a non-standard syntax and that the
  same-origin policy cannot be applied.

  \value HostPortAndUserInformation
  The authority component of a URL of this type has all of the standard
  elements: host, port, user name, and password. A URL without a port will use
  the \l defaultPort (which \e must not be \l PortUnspecified).

  \value HostAndPort
  The authority component of a URL of this type has only the host and port
  elements. A URL without a port will use the \l defaultPort (which \e must not
  be \l PortUnspecified).

  \value Host
  The authority component of a URL of this type has only the host part and no
  port. The \l defaultPort \e must be set to \l PortUnspecified.

  \value Path
  A URL of this type has no authority component at all. Everything after scheme
  name and separator character (:) will be preserved as is without validation
  or canonicalization. All URLs of such a scheme will be considered as having
  the same origin (unless the \c NoAccessAllowed flag is used).
*/

/*!
  \enum QWebEngineUrlScheme::SpecialPort

  This enum type defines special values for \l defaultPort.

  \value PortUnspecified
  Indicates that the URL scheme does not have a port element.
*/

/*!
  \enum QWebEngineUrlScheme::Flag

  This enum type specifies security options that should apply to a URL scheme.

  \value SecureScheme
  Indicates that the URL scheme is
  \l{https://www.w3.org/TR/powerful-features/#is-origin-trustworthy}{potentially
  trustworthy}. This flag should only be applied to URL schemes which ensure
  data authenticity, confidentiality, and integrity, either through encryption
  or other means. Examples of secure builtin schemes include \c https
  (authenticated and encrypted) and \c qrc (local resources only), whereas \c
  http is an example of an insecure scheme.

  \value LocalScheme
  Indicates that the URL scheme provides access to local resources. The purpose
  of this flag is to prevent network content from accessing local resources.
  Only schemes with the \c LocalAccessAllowed flag may load resources from a
  scheme with the \c Local flag. The only builtin schemes with this flag are \c
  file and \c qrc.

  \value LocalAccessAllowed
  Indicates that content from this scheme should be allowed to load resources
  from schemes with the \c Local flag.

  \value NoAccessAllowed
  Indicates that all content from this scheme should be forced to have unique
  opaque origins: no two resources will have the same origin.

  \value ServiceWorkersAllowed
  Indicates that the Service Workers API should be enabled.

  \value ViewSourceAllowed
  Indicates that the View Source feature should be enabled.

  \value ContentSecurityPolicyIgnored
  Indicates that accesses to this scheme should bypass all
  Content-Security-Policy checks.
*/

QWebEngineUrlScheme::QWebEngineUrlScheme(QWebEngineUrlSchemePrivate *d)
    : d(d)
{
}

/*!
  Constructs a web engine URL scheme with default values.
*/
QWebEngineUrlScheme::QWebEngineUrlScheme()
    : QWebEngineUrlScheme(QWebEngineUrlSchemePrivate::defaultConstructed())
{
}

/*!
  Constructs a web engine URL scheme with given \a name.
*/
QWebEngineUrlScheme::QWebEngineUrlScheme(const QByteArray &name)
    : QWebEngineUrlScheme()
{
    setName(name);
}

/*!
  Copies \a that.
*/
QWebEngineUrlScheme::QWebEngineUrlScheme(const QWebEngineUrlScheme &that) = default;

/*!
  Copies \a that.
*/
QWebEngineUrlScheme &QWebEngineUrlScheme::operator=(const QWebEngineUrlScheme &that) = default;

/*!
  Moves \a that.
*/
QWebEngineUrlScheme::QWebEngineUrlScheme(QWebEngineUrlScheme &&that) = default;

/*!
  Moves \a that.
*/
QWebEngineUrlScheme &QWebEngineUrlScheme::operator=(QWebEngineUrlScheme &&that) = default;

/*!
  Destructs this object.
*/
QWebEngineUrlScheme::~QWebEngineUrlScheme() = default;

/*!
  Returns \c true if this and \a that object are equal.
*/
bool QWebEngineUrlScheme::operator==(const QWebEngineUrlScheme &that) const
{
    return (d == that.d)
        || (d->name == that.d->name
            && d->type == that.d->type
            && d->default_port == that.d->default_port
            && d->flags == that.d->flags);
}

/*!
  \fn bool QWebEngineUrlScheme::operator!=(const QWebEngineUrlScheme &that) const

  Returns \c true if this and \a that object are not equal.
*/

/*!
  Returns the name of this URL scheme.

  The default value is an empty string.

  \sa setName()
*/
QByteArray QWebEngineUrlScheme::name() const
{
    return QByteArray::fromStdString(d->name);
}

/*!
  Sets the name of this URL scheme to \a newValue.

  \note The name is automatically converted to lower case.

  \sa name()
*/
void QWebEngineUrlScheme::setName(const QByteArray &newValue)
{
    d->name = newValue.toLower().toStdString();
}

/*!
  Returns the syntax type of this URL scheme.

  The default value is \c Path.

  \sa Syntax, setSyntax()
*/
QWebEngineUrlScheme::Syntax QWebEngineUrlScheme::syntax() const
{
    return static_cast<Syntax>(d->type);
}

/*!
  Sets the syntax type of this URL scheme to \a newValue.

  \sa Syntax, syntax()
*/
void QWebEngineUrlScheme::setSyntax(Syntax newValue)
{
    d->type = static_cast<url::SchemeType>(newValue);
}

/*!
  Returns the default port of this URL scheme.

  The default value is \c PortUnspecified.

  \sa setDefaultPort()
*/
int QWebEngineUrlScheme::defaultPort() const
{
    return d->default_port;
}

/*!
  Sets the default port of this URL scheme to \a newValue.

  \sa defaultPort()
*/
void QWebEngineUrlScheme::setDefaultPort(int newValue)
{
    d->default_port = newValue;
}

/*!
  Returns the flags for this URL scheme.

  The default value is an empty set of flags.

  \sa Flags, setFlags()
*/
QWebEngineUrlScheme::Flags QWebEngineUrlScheme::flags() const
{
    return Flags(d->flags);
}

/*!
  Sets the flags for this URL scheme to \a newValue.

  \sa Flags, flags()
*/
void QWebEngineUrlScheme::setFlags(Flags newValue)
{
    d->flags = newValue;
}

/*!
  Registers \a scheme with the web engine's URL parser and security model.

  It is recommended that all custom URL schemes are first registered with this
  function at application startup, even if the default options are to be used.

  \warning This function must be called early at application startup, before
  creating any WebEngine classes. Late calls will be ignored.

  \sa schemeByName()
*/
void QWebEngineUrlScheme::registerScheme(const QWebEngineUrlScheme &scheme)
{
    if (scheme.d->name.empty()) {
        qWarning() << "QWebEngineUrlScheme::registerScheme: Scheme name cannot be empty";
        return;
    }

    bool needsPort = scheme.d->has_port_component();
    bool hasPort = scheme.d->default_port != url::PORT_UNSPECIFIED;
    if (needsPort && !hasPort) {
        qWarning() << "QWebEngineUrlScheme::registerScheme: Scheme" << scheme.name() << "needs a default port";
        return;
    }

    if (url::CustomScheme::FindScheme(scheme.d->name)) {
        qWarning() << "QWebEngineUrlScheme::registerScheme: Scheme" << scheme.name() << "already registered";
        return;
    }

    if (url::IsStandard(scheme.d->name.data(), url::Component(0, scheme.d->name.size()))) {
        qWarning() << "QWebEngineUrlScheme::registerScheme: Scheme" << scheme.name() << "is a standard scheme";
        return;
    }

    if (g_schemesLocked) {
        qWarning() << "QWebEngineUrlScheme::registerScheme: Too late to register scheme" << scheme.name();
        return;
    }

    url::CustomScheme::AddScheme(*scheme.d);
}

/*!
  Returns the web engine URL scheme with the given \a name or the
  default-constructed scheme.

  \sa registerScheme()
*/
QWebEngineUrlScheme QWebEngineUrlScheme::schemeByName(const QByteArray &name)
{
    base::StringPiece namePiece{name.data(), static_cast<size_t>(name.size())};
    if (const url::CustomScheme *cs = url::CustomScheme::FindScheme(namePiece))
        return QWebEngineUrlScheme(new QWebEngineUrlSchemePrivate(*cs));
    return QWebEngineUrlScheme();
}

void QWebEngineUrlScheme::lockSchemes()
{
    g_schemesLocked = true;
}

QT_END_NAMESPACE
