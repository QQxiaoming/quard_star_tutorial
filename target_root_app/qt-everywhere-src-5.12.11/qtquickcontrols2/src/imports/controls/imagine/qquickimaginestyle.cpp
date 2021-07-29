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

#include "qquickimaginestyle_p.h"

#include <QtCore/qsettings.h>
#include <QtQuickControls2/private/qquickstyle_p.h>

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(QString, GlobalPath, (QLatin1String("qrc:/qt-project.org/imports/QtQuick/Controls.2/Imagine/images/")))

static const QString ensureSlash(const QString &path)
{
    const QChar slash = QLatin1Char('/');
    return path.endsWith(slash) ? path : path + slash;
}

QQuickImagineStyle::QQuickImagineStyle(QObject *parent)
    : QQuickAttachedObject(parent),
      m_path(*GlobalPath())
{
    init();
}

QQuickImagineStyle *QQuickImagineStyle::qmlAttachedProperties(QObject *object)
{
    return new QQuickImagineStyle(object);
}

QString QQuickImagineStyle::path() const
{
    return m_path;
}

void QQuickImagineStyle::setPath(const QString &path)
{
    m_explicitPath = true;
    if (m_path == path)
        return;

    m_path = path;
    propagatePath();

    emit pathChanged();
}

void QQuickImagineStyle::inheritPath(const QString &path)
{
    if (m_explicitPath || m_path == path)
        return;

    m_path = path;
    propagatePath();
    emit pathChanged();
}

void QQuickImagineStyle::propagatePath()
{
    const auto styles = attachedChildren();
    for (QQuickAttachedObject *child : styles) {
        QQuickImagineStyle *imagine = qobject_cast<QQuickImagineStyle *>(child);
        if (imagine)
            imagine->inheritPath(m_path);
    }
}

void QQuickImagineStyle::resetPath()
{
    if (!m_explicitPath)
        return;

    m_explicitPath = false;
    QQuickImagineStyle *imagine = qobject_cast<QQuickImagineStyle *>(attachedParent());
    inheritPath(imagine ? imagine->path() : *GlobalPath());
}

QUrl QQuickImagineStyle::url() const
{
    // Using ApplicationWindow as an example, its NinePatchImage url
    // was previously assigned like this:
    //
    // soruce: Imagine.path + "applicationwindow-background"
    //
    // If Imagine.path is set to ":/images" by the user, then the final URL would be:
    //
    // QUrl("file:///home/user/qt/qtbase/qml/QtQuick/Controls.2/Imagine/:/images/applicationwindow-background")
    //
    // To ensure that the correct URL is constructed, we do it ourselves here,
    // and then the control QML files use the "url" property instead.
    const QString path = ensureSlash(m_path);
    if (path.startsWith(QLatin1String("qrc")))
        return QUrl(path);

    if (path.startsWith(QLatin1String(":/")))
        return QUrl(QLatin1String("qrc") + path);

    return QUrl::fromLocalFile(path);
}

void QQuickImagineStyle::attachedParentChange(QQuickAttachedObject *newParent, QQuickAttachedObject *oldParent)
{
    Q_UNUSED(oldParent);
    QQuickImagineStyle *imagine = qobject_cast<QQuickImagineStyle *>(newParent);
    if (imagine)
        inheritPath(imagine->path());
}

static QByteArray resolveSetting(const QByteArray &env, const QSharedPointer<QSettings> &settings, const QString &name)
{
    QByteArray value = qgetenv(env);
#if QT_CONFIG(settings)
    if (value.isNull() && !settings.isNull())
        value = settings->value(name).toByteArray();
#endif
    return value;
}

void QQuickImagineStyle::init()
{
    static bool globalsInitialized = false;
    if (!globalsInitialized) {
        QSharedPointer<QSettings> settings = QQuickStylePrivate::settings(QStringLiteral("Imagine"));

        QString path = QString::fromUtf8(resolveSetting("QT_QUICK_CONTROLS_IMAGINE_PATH", settings, QStringLiteral("Path")));
        if (!path.isEmpty())
            *GlobalPath() = m_path = ensureSlash(path);

        globalsInitialized = true;
    }

    QQuickAttachedObject::init(); // TODO: lazy init?
}

QT_END_NAMESPACE
