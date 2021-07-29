/***************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Copyright (C) 2013 BlackBerry Limited. All rights reserved.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Controls 2 module of the Qt Toolkit.
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

#include "qquickstyleselector_p.h"
#include "qquickstyleselector_p_p.h"

#include <QtCore/qfileinfo.h>
#include <QtCore/qlocale.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/private/qfileselector_p.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcQtQuickControlsStyle, "qt.quick.controls.style")

static QString ensureSlash(const QString &path)
{
    if (path.isEmpty() || path.endsWith(QLatin1Char('/')))
        return path;
    return path + QLatin1Char('/');
}

static QStringList prefixedPlatformSelectors(const QChar &prefix)
{
    QStringList selectors = QFileSelectorPrivate::platformSelectors();
    for (int i = 0; i < selectors.count(); ++i)
        selectors[i].prepend(prefix);
    return selectors;
}

static QStringList allSelectors()
{
    static const QStringList platformSelectors = prefixedPlatformSelectors(QLatin1Char('+'));
    QStringList selectors = platformSelectors;
    const QString locale = QLocale().name();
    if (!locale.isEmpty())
        selectors += QLatin1Char('+') + locale;
    return selectors;
}

QUrl QQuickStyleSelectorPrivate::select(const QString &filePath) const
{
    QFileInfo fi(filePath);
    // If file doesn't exist, don't select
    if (!fi.exists())
        return QUrl();

    const QString selected = QFileSelectorPrivate::selectionHelper(ensureSlash(fi.canonicalPath()),
                                                                   fi.fileName(), allSelectors(), QChar());

    if (selected.startsWith(QLatin1Char(':')))
        return QUrl(QLatin1String("qrc") + selected);

    return QUrl::fromLocalFile(selected.isEmpty() ? filePath : selected);
}

QQuickStyleSelector::QQuickStyleSelector() : d_ptr(new QQuickStyleSelectorPrivate)
{
}

QQuickStyleSelector::~QQuickStyleSelector()
{
}

QStringList QQuickStyleSelector::selectors() const
{
    Q_D(const QQuickStyleSelector);
    return d->selectors;
}

void QQuickStyleSelector::addSelector(const QString &selector)
{
    Q_D(QQuickStyleSelector);
    if (d->selectors.contains(selector))
        return;

    d->selectors += selector;
}

QStringList QQuickStyleSelector::paths() const
{
    Q_D(const QQuickStyleSelector);
    return d->paths;
}

void QQuickStyleSelector::setPaths(const QStringList &paths)
{
    Q_D(QQuickStyleSelector);
    d->paths = paths;
}

QUrl QQuickStyleSelector::select(const QString &fileName) const
{
    Q_D(const QQuickStyleSelector);
    // The lookup order is
    // 1) requested style (e.g. "MyStyle", included in d->selectors)
    // 2) fallback style (e.g. "Material", included in d->selectors)
    // 3) default style (empty selector, not in d->selectors)
    qCDebug(lcQtQuickControlsStyle) << "selecting" << fileName << "from" << d->paths << "with selectors" << d->selectors;

    int to = d->selectors.count() - 1;
    if (d->selectors.isEmpty() || !d->selectors.first().isEmpty())
        ++to; // lookup #3 unless #1 is also empty (redundant)

    // NOTE: last iteration intentionally out of bounds => empty selector
    for (int i = 0; i <= to; ++i) {
        const QString selector = d->selectors.value(i);
        for (const QString &path : d->paths) {
            const QUrl selectedUrl = d->select(ensureSlash(path) + selector + QLatin1Char('/') + fileName);
            if (selectedUrl.isValid()) {
                qCDebug(lcQtQuickControlsStyle) << "==>" << selectedUrl << "from" << path << "with selector" << selector;
                return selectedUrl;
            }
        }
    }

    return fileName;
}

QT_END_NAMESPACE
