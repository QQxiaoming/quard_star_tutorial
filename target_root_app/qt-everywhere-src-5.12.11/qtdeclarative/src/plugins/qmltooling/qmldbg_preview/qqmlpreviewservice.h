/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QML preview debug service.
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

#ifndef QQMLPREVIEWSERVICE_H
#define QQMLPREVIEWSERVICE_H

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

#include "qqmlpreviewhandler.h"
#include "qqmlpreviewfileengine.h"
#include <private/qqmldebugserviceinterfaces_p.h>

QT_BEGIN_NAMESPACE

class QQmlPreviewFileEngineHandler;
class QQmlPreviewHandler;
class QQmlPreviewServiceImpl : public QQmlDebugService
{
    Q_OBJECT

public:
    enum Command {
        File,
        Load,
        Request,
        Error,
        Rerun,
        Directory,
        ClearCache,
        Zoom,
        Fps,
        Language
    };

    static const QString s_key;

    QQmlPreviewServiceImpl(QObject *parent = nullptr);
    virtual ~QQmlPreviewServiceImpl();

    void messageReceived(const QByteArray &message) override;
    void engineAboutToBeAdded(QJSEngine *engine) override;
    void engineAboutToBeRemoved(QJSEngine *engine) override;
    void stateChanged(State state) override;

    void forwardRequest(const QString &file);
    void forwardError(const QString &error);
    void forwardFps(const QQmlPreviewHandler::FpsInfo &frames);

signals:
    void error(const QString &file);
    void file(const QString &file, const QByteArray &contents);
    void directory(const QString &file, const QStringList &entries);
    void load(const QUrl &url);
    void rerun();
    void clearCache();
    void zoom(qreal factor);
    void language(const QUrl &context, const QLocale &locale);

private:
    QScopedPointer<QQmlPreviewFileEngineHandler> m_fileEngine;
    QScopedPointer<QQmlPreviewFileLoader> m_loader;
    QQmlPreviewHandler m_handler;
    QUrl m_currentUrl;
};

QT_END_NAMESPACE

#endif // QQMLPREVIEWSERVICE_H
