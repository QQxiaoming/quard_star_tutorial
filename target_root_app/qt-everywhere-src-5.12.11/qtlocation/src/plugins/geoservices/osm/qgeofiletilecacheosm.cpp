/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtLocation module of the Qt Toolkit.
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

#include "qgeofiletilecacheosm.h"
#include <QtLocation/private/qgeotilespec_p.h>
#include <QDir>
#include <QDirIterator>
#include <QPair>
#include <QDateTime>
#include <QtConcurrent>
#include <QThread>

QT_BEGIN_NAMESPACE

QGeoFileTileCacheOsm::QGeoFileTileCacheOsm(const QVector<QGeoTileProviderOsm *> &providers,
                                           const QString &offlineDirectory,
                                           const QString &directory,
                                           QObject *parent)
:   QGeoFileTileCache(directory, parent), m_offlineDirectory(offlineDirectory), m_offlineData(false), m_providers(providers)
{
    m_highDpi.resize(providers.size());
    if (!offlineDirectory.isEmpty()) {
        m_offlineDirectory = QDir(offlineDirectory);
        if (m_offlineDirectory.exists())
            m_offlineData = true;
    }
    for (int i = 0; i < providers.size(); i++) {
        providers[i]->setParent(this);
        m_highDpi[i] = providers[i]->isHighDpi();
        connect(providers[i], &QGeoTileProviderOsm::resolutionFinished, this, &QGeoFileTileCacheOsm::onProviderResolutionFinished);
        connect(providers[i], &QGeoTileProviderOsm::resolutionError, this, &QGeoFileTileCacheOsm::onProviderResolutionFinished);
    }
}

QGeoFileTileCacheOsm::~QGeoFileTileCacheOsm()
{
}

QSharedPointer<QGeoTileTexture> QGeoFileTileCacheOsm::get(const QGeoTileSpec &spec)
{
    QSharedPointer<QGeoTileTexture> tt = getFromMemory(spec);
    if (tt)
        return tt;
    if ((tt = getFromOfflineStorage(spec)))
        return tt;
    return getFromDisk(spec);
}

void QGeoFileTileCacheOsm::onProviderResolutionFinished(const QGeoTileProviderOsm *provider)
{
    clearObsoleteTiles(provider);
    Q_UNUSED(provider)
    for (int i = 0; i < m_providers.size(); i++) {
        if (m_providers[i]->isHighDpi() != m_highDpi[i]) { // e.g., HiDpi was requested but only LoDpi is available
            int mapId = m_providers[i]->mapType().mapId();
            m_highDpi[i] = m_providers[i]->isHighDpi();

            // reload cache for mapId i
            dropTiles(mapId);
            loadTiles(mapId);

            // send signal to clear scene in all maps created through this provider that use the reloaded tiles
            emit mapDataUpdated(mapId);
        }
    }
}

// On resolution error the provider is removed.
// This happens ONLY if there is no enabled hardcoded fallback for the mapId.
// Hardcoded fallbacks also have a timestamp, that can get updated with Qt releases.
void QGeoFileTileCacheOsm::onProviderResolutionError(const QGeoTileProviderOsm *provider, QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)
    clearObsoleteTiles(provider); // this still removes tiles who happen to be older than qgeotileproviderosm.cpp defaultTs
}

// init() is always called before the provider resolution starts
void QGeoFileTileCacheOsm::init()
{
    if (directory_.isEmpty())
        directory_ = baseLocationCacheDirectory();
    QDir::root().mkpath(directory_);

    // find max mapId
    int max = 0;
    for (auto p: m_providers)
        if (p->mapType().mapId() > max)
            max = p->mapType().mapId();
    // Create a mapId to maxTimestamp LUT..
    m_maxMapIdTimestamps.resize(max+1); // initializes to invalid QDateTime

    // .. by finding the newest file in each tileset (tileset = mapId).
    QDir dir(directory_);
    QStringList formats;
    formats << QLatin1String("*.*");
    QStringList files = dir.entryList(formats, QDir::Files);

    for (const QString &tileFileName : files) {
        QGeoTileSpec spec = filenameToTileSpec(tileFileName);
        if (spec.zoom() == -1)
            continue;
        QFileInfo fi(dir.filePath(tileFileName));
        if (fi.lastModified() > m_maxMapIdTimestamps[spec.mapId()])
            m_maxMapIdTimestamps[spec.mapId()] = fi.lastModified();
    }

    // Base class ::init()
    QGeoFileTileCache::init();

    for (QGeoTileProviderOsm * p: m_providers)
        clearObsoleteTiles(p);
}

QSharedPointer<QGeoTileTexture> QGeoFileTileCacheOsm::getFromOfflineStorage(const QGeoTileSpec &spec)
{
    if (!m_offlineData)
        return QSharedPointer<QGeoTileTexture>();

    int providerId = spec.mapId() - 1;
    if (providerId < 0 || providerId >= m_providers.size())
        return QSharedPointer<QGeoTileTexture>();

    const QString fileName = tileSpecToFilename(spec, QStringLiteral("*"), providerId);
    QStringList validTiles = m_offlineDirectory.entryList({fileName});
    if (!validTiles.size())
        return QSharedPointer<QGeoTileTexture>();

    QFile file(m_offlineDirectory.absoluteFilePath(validTiles.first()));
    if (!file.open(QIODevice::ReadOnly))
        return QSharedPointer<QGeoTileTexture>();
    QByteArray bytes = file.readAll();
    file.close();

    QImage image;
    if (!image.loadFromData(bytes)) {
        handleError(spec, QLatin1String("Problem with tile image"));
        return QSharedPointer<QGeoTileTexture>(0);
    }

    addToMemoryCache(spec, bytes, QString());
    return addToTextureCache(spec, image);
}

void QGeoFileTileCacheOsm::dropTiles(int mapId)
{
    QList<QGeoTileSpec> keys;
    keys = textureCache_.keys();
    for (const QGeoTileSpec &k : keys)
        if (k.mapId() == mapId)
            textureCache_.remove(k);

    keys = memoryCache_.keys();
    for (const QGeoTileSpec &k : keys)
        if (k.mapId() == mapId)
            memoryCache_.remove(k);

    keys = diskCache_.keys();
    for (const QGeoTileSpec &k : keys)
        if (k.mapId() == mapId)
            diskCache_.remove(k);
}

void QGeoFileTileCacheOsm::loadTiles(int mapId)
{
    QStringList formats;
    formats << QLatin1String("*.*");

    QDir dir(directory_);
    QStringList files = dir.entryList(formats, QDir::Files);

    for (int i = 0; i < files.size(); ++i) {
        QGeoTileSpec spec = filenameToTileSpec(files.at(i));
        if (spec.zoom() == -1 || spec.mapId() != mapId)
            continue;
        QString filename = dir.filePath(files.at(i));
        addToDiskCache(spec, filename);
    }
}

QString QGeoFileTileCacheOsm::tileSpecToFilename(const QGeoTileSpec &spec, const QString &format, const QString &directory) const
{
    int providerId = spec.mapId() - 1;
    if (providerId < 0 || providerId >= m_providers.size())
        return QString();

    QDir dir = QDir(directory);
    return dir.filePath(tileSpecToFilename(spec, format, providerId));
}

QString QGeoFileTileCacheOsm::tileSpecToFilename(const QGeoTileSpec &spec, const QString &format, int providerId) const
{
    QString filename = spec.plugin();
    filename += QLatin1String("-");
    filename += (m_providers[providerId]->isHighDpi()) ? QLatin1Char('h') : QLatin1Char('l');
    filename += QLatin1String("-");
    filename += QString::number(spec.mapId());
    filename += QLatin1String("-");
    filename += QString::number(spec.zoom());
    filename += QLatin1String("-");
    filename += QString::number(spec.x());
    filename += QLatin1String("-");
    filename += QString::number(spec.y());

    //Append version if real version number to ensure backwards compatibility and eviction of old tiles
    if (spec.version() != -1) {
        filename += QLatin1String("-");
        filename += QString::number(spec.version());
    }

    filename += QLatin1String(".");
    filename += format;
    return filename;
}

QGeoTileSpec QGeoFileTileCacheOsm::filenameToTileSpec(const QString &filename) const
{
    QGeoTileSpec emptySpec;

    QStringList parts = filename.split('.');

    if (parts.length() != 2)
        return emptySpec;

    QString name = parts.at(0);
    QStringList fields = name.split('-');

    int length = fields.length();
    if (length != 6 && length != 7)
        return emptySpec;

    QList<int> numbers;

    bool ok = false;
    for (int i = 2; i < length; ++i) {
        ok = false;
        int value = fields.at(i).toInt(&ok);
        if (!ok)
            return emptySpec;
        numbers.append(value);
    }

    if (numbers.at(0) > m_providers.size())
        return emptySpec;

    bool highDpi = m_providers[numbers.at(0) - 1]->isHighDpi();
    if (highDpi && fields.at(1) != QLatin1Char('h'))
        return emptySpec;
    else if (!highDpi && fields.at(1) != QLatin1Char('l'))
        return emptySpec;

    //File name without version, append default
    if (numbers.length() < 5)
        numbers.append(-1);

    return QGeoTileSpec(fields.at(0),
                    numbers.at(0),
                    numbers.at(1),
                    numbers.at(2),
                    numbers.at(3),
                    numbers.at(4));
}

void QGeoFileTileCacheOsm::clearObsoleteTiles(const QGeoTileProviderOsm *p)
{
    // process initialized providers, and connect the others

        if (p->isResolved()) {
            if (m_maxMapIdTimestamps[p->mapType().mapId()].isValid() &&  // there are tiles in the cache
                p->timestamp() > m_maxMapIdTimestamps[p->mapType().mapId()]) { // and they are older than the provider
                qInfo() << "provider for " << p->mapType().name() << " timestamp: " << p->timestamp()
                        << " -- data last modified: " << m_maxMapIdTimestamps[p->mapType().mapId()] << ". Clearing.";
                clearMapId(p->mapType().mapId());
                m_maxMapIdTimestamps[p->mapType().mapId()] = p->timestamp(); // don't do it again.
            }
        } else {
            connect(p, &QGeoTileProviderOsm::resolutionFinished,
                    this, &QGeoFileTileCacheOsm::onProviderResolutionFinished);
#if 0 // If resolution fails, better not try to remove anything. Beside, on error, resolutionFinished is also emitted.
            connect(p, &QGeoTileProviderOsm::resolutionError,
                    this, &QGeoFileTileCacheOsm::onProviderResolutionError);
#endif
        }
}

QT_END_NAMESPACE
