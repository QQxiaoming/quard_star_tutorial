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

#ifndef QWEBENGINEURLSCHEME_H
#define QWEBENGINEURLSCHEME_H

#include <QtWebEngineCore/qtwebenginecoreglobal.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qobjectdefs.h>
#include <QtCore/qshareddata.h>

namespace QtWebEngineCore { class WebEngineContext; }

QT_BEGIN_NAMESPACE

class QWebEngineUrlSchemePrivate;

class QWEBENGINECORE_EXPORT QWebEngineUrlScheme {
    Q_GADGET
public:
    enum class Syntax {
        HostPortAndUserInformation,
        HostAndPort,
        Host,
        Path,
    };

    enum SpecialPort {
        PortUnspecified = -1
    };

    enum Flag {
        SecureScheme = 0x1,
        LocalScheme = 0x2,
        LocalAccessAllowed = 0x4,
        NoAccessAllowed = 0x8,
        ServiceWorkersAllowed = 0x10,
        ViewSourceAllowed = 0x20,
        ContentSecurityPolicyIgnored = 0x40,
    };
    Q_DECLARE_FLAGS(Flags, Flag)
    Q_FLAG(Flags)

    QWebEngineUrlScheme();
    explicit QWebEngineUrlScheme(const QByteArray &name);

    QWebEngineUrlScheme(const QWebEngineUrlScheme &that);
    QWebEngineUrlScheme &operator=(const QWebEngineUrlScheme &that);

    QWebEngineUrlScheme(QWebEngineUrlScheme &&that);
    QWebEngineUrlScheme &operator=(QWebEngineUrlScheme &&that);

    ~QWebEngineUrlScheme();

    bool operator==(const QWebEngineUrlScheme &that) const;
    bool operator!=(const QWebEngineUrlScheme &that) const { return !(*this == that); }

    QByteArray name() const;
    void setName(const QByteArray &newValue);

    Syntax syntax() const;
    void setSyntax(Syntax newValue);

    int defaultPort() const;
    void setDefaultPort(int newValue);

    Flags flags() const;
    void setFlags(Flags newValue);

    static void registerScheme(const QWebEngineUrlScheme &scheme);
    static QWebEngineUrlScheme schemeByName(const QByteArray &name);

private:
    friend QtWebEngineCore::WebEngineContext;
    static void lockSchemes();
    QWebEngineUrlScheme(QWebEngineUrlSchemePrivate *d);
    QSharedDataPointer<QWebEngineUrlSchemePrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QWebEngineUrlScheme::Flags)

QT_END_NAMESPACE

#endif // !QWEBENGINEURLSCHEME_H
