/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
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

#include "playlistmodel.h"

#include <QFileInfo>
#include <QUrl>
#include <QMediaPlaylist>

PlaylistModel::PlaylistModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

PlaylistModel::~PlaylistModel()
{
}

int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    return m_playlist && !parent.isValid() ? m_playlist->mediaCount() : 0;
}

int PlaylistModel::columnCount(const QModelIndex &parent) const
{
    return !parent.isValid() ? ColumnCount : 0;
}

QModelIndex PlaylistModel::index(int row, int column, const QModelIndex &parent) const
{
    return m_playlist && !parent.isValid()
            && row >= 0 && row < m_playlist->mediaCount()
            && column >= 0 && column < ColumnCount
        ? createIndex(row, column)
        : QModelIndex();
}

QModelIndex PlaylistModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);

    return QModelIndex();
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && role == Qt::DisplayRole) {
        QVariant value = m_data[index];
        if (!value.isValid() && index.column() == Title) {
            QUrl location = m_playlist->media(index.row()).canonicalUrl();
            return QFileInfo(location.path()).fileName();
        }

        return value;
    }
    return QVariant();
}

QMediaPlaylist *PlaylistModel::playlist() const
{
    return m_playlist.data();
}

void PlaylistModel::setPlaylist(QMediaPlaylist *playlist)
{
    if (m_playlist) {
        disconnect(m_playlist.data(), &QMediaPlaylist::mediaAboutToBeInserted, this, &PlaylistModel::beginInsertItems);
        disconnect(m_playlist.data(), &QMediaPlaylist::mediaInserted, this, &PlaylistModel::endInsertItems);
        disconnect(m_playlist.data(), &QMediaPlaylist::mediaAboutToBeRemoved, this, &PlaylistModel::beginRemoveItems);
        disconnect(m_playlist.data(), &QMediaPlaylist::mediaRemoved, this, &PlaylistModel::endRemoveItems);
        disconnect(m_playlist.data(), &QMediaPlaylist::mediaChanged, this, &PlaylistModel::changeItems);
    }

    beginResetModel();
    m_playlist.reset(playlist);

    if (m_playlist) {
        connect(m_playlist.data(), &QMediaPlaylist::mediaAboutToBeInserted, this, &PlaylistModel::beginInsertItems);
        connect(m_playlist.data(), &QMediaPlaylist::mediaInserted, this, &PlaylistModel::endInsertItems);
        connect(m_playlist.data(), &QMediaPlaylist::mediaAboutToBeRemoved, this, &PlaylistModel::beginRemoveItems);
        connect(m_playlist.data(), &QMediaPlaylist::mediaRemoved, this, &PlaylistModel::endRemoveItems);
        connect(m_playlist.data(), &QMediaPlaylist::mediaChanged, this, &PlaylistModel::changeItems);
    }

    endResetModel();
}

bool PlaylistModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(role);
    m_data[index] = value;
    emit dataChanged(index, index);
    return true;
}

void PlaylistModel::beginInsertItems(int start, int end)
{
    m_data.clear();
    beginInsertRows(QModelIndex(), start, end);
}

void PlaylistModel::endInsertItems()
{
    endInsertRows();
}

void PlaylistModel::beginRemoveItems(int start, int end)
{
    m_data.clear();
    beginRemoveRows(QModelIndex(), start, end);
}

void PlaylistModel::endRemoveItems()
{
    endInsertRows();
}

void PlaylistModel::changeItems(int start, int end)
{
    m_data.clear();
    emit dataChanged(index(start,0), index(end,ColumnCount));
}
