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

#include "qqmlpreviewservice.h"

#include <QtCore/qpointer.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlcomponent.h>
#include <QtQuick/qquickwindow.h>
#include <QtQuick/qquickitem.h>
#include <QtGui/qguiapplication.h>

#include <private/qquickpixmapcache_p.h>
#include <private/qqmldebugconnector_p.h>
#include <private/qversionedpacket_p.h>

QT_BEGIN_NAMESPACE

const QString QQmlPreviewServiceImpl::s_key = QStringLiteral("QmlPreview");
using QQmlDebugPacket = QVersionedPacket<QQmlDebugConnector>;

QQmlPreviewServiceImpl::QQmlPreviewServiceImpl(QObject *parent) :
    QQmlDebugService(s_key, 1.0f, parent)
{
    m_loader.reset(new QQmlPreviewFileLoader(this));
    connect(this, &QQmlPreviewServiceImpl::load,
            m_loader.data(), &QQmlPreviewFileLoader::whitelist, Qt::DirectConnection);
    connect(this, &QQmlPreviewServiceImpl::load, &m_handler, &QQmlPreviewHandler::loadUrl);
    connect(this, &QQmlPreviewServiceImpl::rerun, &m_handler, &QQmlPreviewHandler::rerun);
    connect(this, &QQmlPreviewServiceImpl::zoom, &m_handler, &QQmlPreviewHandler::zoom);
    connect(this, &QQmlPreviewServiceImpl::language, &m_handler, &QQmlPreviewHandler::language);
    connect(&m_handler, &QQmlPreviewHandler::error, this, &QQmlPreviewServiceImpl::forwardError,
            Qt::DirectConnection);
    connect(&m_handler, &QQmlPreviewHandler::fps, this, &QQmlPreviewServiceImpl::forwardFps,
            Qt::DirectConnection);
}

QQmlPreviewServiceImpl::~QQmlPreviewServiceImpl()
{
}

void QQmlPreviewServiceImpl::messageReceived(const QByteArray &data)
{
    QQmlDebugPacket packet(data);
    qint8 command;

    packet >> command;
    switch (command) {
    case File: {
        QString path;
        QByteArray contents;
        packet >> path >> contents;
        emit file(path, contents);

        // Replace the whole scene with the first file successfully loaded over the debug
        // connection. This is an OK approximation of the root component, and if the client wants
        // something specific, it will send an explicit Load anyway.
        if (m_currentUrl.isEmpty() && path.endsWith(".qml")) {
            if (path.startsWith(':'))
                m_currentUrl = QUrl("qrc" + path);
            else
                m_currentUrl = QUrl::fromLocalFile(path);
            emit load(m_currentUrl);
        }
        break;
    }
    case Directory: {
        QString path;
        QStringList entries;
        packet >> path >> entries;
        emit directory(path, entries);
        break;
    }
    case Load: {
        QUrl url;
        packet >> url;
        if (url.isEmpty())
            url = m_currentUrl;
        else
            m_currentUrl = url;
        emit load(url);
        break;
    }
    case Error: {
        QString file;
        packet >> file;
        emit error(file);
        break;
    }
    case Rerun:
        emit rerun();
        break;
    case ClearCache:
        emit clearCache();
        break;
    case Zoom: {
        float factor;
        packet >> factor;
        emit zoom(static_cast<qreal>(factor));
        break;
    }
    case Language: {
        QUrl context;
        QString locale;
        packet >> context >> locale;
        emit language(context.isEmpty() ? m_currentUrl : context,
                      locale.isEmpty() ? QLocale() : QLocale(locale));
        break;
    }
    default:
        forwardError(QString::fromLatin1("Invalid command: %1").arg(command));
        break;
    }
}

void QQmlPreviewServiceImpl::engineAboutToBeAdded(QJSEngine *engine)
{
    if (QQmlEngine *qmlEngine = qobject_cast<QQmlEngine *>(engine))
        m_handler.addEngine(qmlEngine);
    emit attachedToEngine(engine);
}

void QQmlPreviewServiceImpl::engineAboutToBeRemoved(QJSEngine *engine)
{
    if (QQmlEngine *qmlEngine = qobject_cast<QQmlEngine *>(engine))
        m_handler.removeEngine(qmlEngine);
    emit detachedFromEngine(engine);
}

void QQmlPreviewServiceImpl::stateChanged(QQmlDebugService::State state)
{
    m_fileEngine.reset(state == Enabled ? new QQmlPreviewFileEngineHandler(m_loader.data())
                                        : nullptr);
}

void QQmlPreviewServiceImpl::forwardRequest(const QString &file)
{
    QQmlDebugPacket packet;
    packet << static_cast<qint8>(Request) << file;
    emit messageToClient(name(), packet.data());
}

void QQmlPreviewServiceImpl::forwardError(const QString &error)
{
    QQmlDebugPacket packet;
    packet << static_cast<qint8>(Error) << error;
    emit messageToClient(name(), packet.data());
}

void QQmlPreviewServiceImpl::forwardFps(const QQmlPreviewHandler::FpsInfo &frames)
{
    QQmlDebugPacket packet;
    packet << static_cast<qint8>(Fps)
           << frames.numSyncs << frames.minSync << frames.maxSync << frames.totalSync
           << frames.numRenders << frames.minRender << frames.maxRender << frames.totalRender;
    emit messageToClient(name(), packet.data());
}

QT_END_NAMESPACE
