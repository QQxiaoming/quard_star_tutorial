/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QAxFactory>
#include <QCheckBox>
#include <QRadioButton>
#include <QPushButton>
#include <QToolButton>
#include <QPixmap>
#include <functional>

//! [0]
class ActiveQtFactory : public QAxFactory
{
public:
    ActiveQtFactory(const QUuid &lib, const QUuid &app)
        : QAxFactory(lib, app)
    {}

    QStringList featureList() const
    {
        return m_activeElements.keys();
    }

    QObject *createObject(const QString &key)
    {
        auto it = m_activeElements.find(key);
        if (it != m_activeElements.end())
            return it->create();
        return nullptr;
    }

    const QMetaObject *metaObject(const QString &key) const
    {
        auto it = m_activeElements.find(key);
        if (it != m_activeElements.end())
            return it->metaObject;
        return nullptr;
    }

    QUuid classID(const QString &key) const
    {
        auto it = m_activeElements.find(key);
        if (it != m_activeElements.end())
            return it->classID;
        return QUuid();
    }

    QUuid interfaceID(const QString &key) const
    {
        auto it = m_activeElements.find(key);
        if (it != m_activeElements.end())
            return it->interfaceID;
        return QUuid();
    }

    QUuid eventsID(const QString &key) const
    {
        auto it = m_activeElements.find(key);
        if (it != m_activeElements.end())
            return it->eventsID;
        return QUuid();
    }

private:

    struct ActiveElement {
       QUuid classID;
       QUuid interfaceID;
       QUuid eventsID;
       const QMetaObject *metaObject;
       std::function<QObject *()> create;
    };

    const QHash<QString, ActiveElement> m_activeElements {
        {
            QStringLiteral("QCheckBox"), {
                QUuid("{6E795DE9-872D-43CF-A831-496EF9D86C68}"),
                QUuid("{4FD39DD7-2DE0-43C1-A8C2-27C51A052810}"),
                QUuid("{FDB6FFBE-56A3-4E90-8F4D-198488418B3A}"),
                &QCheckBox::staticMetaObject,
                []() { return new QCheckBox; }
            }
        },
        {
            QStringLiteral("QRadioButton"), {
                QUuid("{AFCF78C8-446C-409A-93B3-BA2959039189}"),
                QUuid("{7CC8AE30-206C-48A3-A009-B0A088026C2F}"),
                QUuid("{73EE4860-684C-4A66-BF63-9B9EFFA0CBE5}"),
                &QRadioButton::staticMetaObject,
                []() { return new QRadioButton; }
            }
        },
        {
            QStringLiteral("QPushButton"), {
                QUuid("{2B262458-A4B6-468B-B7D4-CF5FEE0A7092}"),
                QUuid("{06831CC9-59B6-436A-9578-6D53E5AD03D3}"),
                QUuid("{3CC3F17F-EA59-4B58-BBD3-842D467131DD}"),
                &QPushButton::staticMetaObject,
                []() { return new QPushButton; }
            }
        },
        {
            QStringLiteral("QToolButton"), {
                QUuid("{7c0ffe7a-60c3-4666-bde2-5cf2b54390a1}"),
                QUuid("{6726080f-d63d-4950-a366-9bf33e5cdf84}"),
                QUuid("{f4d421fd-4ead-4fd9-8a25-440766939639}"),
                &QToolButton::staticMetaObject,
                []() { return new QToolButton; }
            }
        },
    };

};
//! [0] //! [1]

QAXFACTORY_EXPORT(ActiveQtFactory, "{3B756301-0075-4E40-8BE8-5A81DE2426B7}", "{AB068077-4924-406a-BBAF-42D91C8727DD}")
//! [1]
