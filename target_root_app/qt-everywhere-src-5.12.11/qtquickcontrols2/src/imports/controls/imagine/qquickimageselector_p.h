/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Controls 2 module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QQUICKIMAGESELECTOR_P_H
#define QQUICKIMAGESELECTOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qurl.h>
#include <QtCore/qobject.h>
#include <QtCore/qvariant.h>
#include <QtQml/qqmlproperty.h>
#include <QtQml/qqmlparserstatus.h>
#include <QtQml/private/qqmlpropertyvalueinterceptor_p.h>
#include <QtQml/qqmlproperty.h>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class QQuickImageSelector : public QObject, public QQmlParserStatus, public QQmlPropertyValueInterceptor
{
    Q_OBJECT
    Q_PROPERTY(QUrl source READ source NOTIFY sourceChanged FINAL)
    Q_PROPERTY(QString name READ name WRITE setName FINAL)
    Q_PROPERTY(QString path READ path WRITE setPath FINAL)
    Q_PROPERTY(QVariantList states READ states WRITE setStates FINAL)
    Q_PROPERTY(QString separator READ separator WRITE setSeparator FINAL)
    Q_PROPERTY(bool cache READ cache WRITE setCache FINAL)
    Q_INTERFACES(QQmlParserStatus QQmlPropertyValueInterceptor)

public:
    explicit QQuickImageSelector(QObject *parent = nullptr);

    QUrl source() const;
    void setSource(const QUrl &source);

    QString name() const;
    void setName(const QString &name);

    QString path() const;
    void setPath(const QString &path);

    QVariantList states() const;
    void setStates(const QVariantList &states);

    QString separator() const;
    void setSeparator(const QString &separator);

    bool cache() const;
    void setCache(bool cache);

    void write(const QVariant &value) override;
    void setTarget(const QQmlProperty &property) override;

Q_SIGNALS:
    void sourceChanged();

protected:
    void classBegin() override;
    void componentComplete() override;

    virtual QStringList fileExtensions() const;

    QString cacheKey() const;
    void updateSource();
    void setUrl(const QUrl &url);
    bool updateActiveStates();
    int calculateScore(const QStringList &states) const;

private:
    bool m_cache = false;
    bool m_complete = false;
    QUrl m_source;
    QString m_path;
    QString m_name;
    QString m_separator = QLatin1String("-");
    QVariantList m_allStates;
    QStringList m_activeStates;
    QQmlProperty m_property;
};

class QQuickNinePatchImageSelector : public QQuickImageSelector
{
    Q_OBJECT

public:
    explicit QQuickNinePatchImageSelector(QObject *parent = nullptr);

protected:
    QStringList fileExtensions() const override;
};

class QQuickAnimatedImageSelector : public QQuickImageSelector
{
    Q_OBJECT

public:
    explicit QQuickAnimatedImageSelector(QObject *parent = nullptr);

protected:
    QStringList fileExtensions() const override;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuickImageSelector)
QML_DECLARE_TYPE(QQuickAnimatedImageSelector)

#endif // QQUICKIMAGESELECTOR_P_H
