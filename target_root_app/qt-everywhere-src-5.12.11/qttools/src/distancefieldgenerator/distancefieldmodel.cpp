/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "distancefieldmodel.h"
#include "distancefieldmodelworker.h"

#include <QThread>
#include <QMetaEnum>

QT_BEGIN_NAMESPACE

DistanceFieldModel::DistanceFieldModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_glyphCount(0)
{
    int index = metaObject()->indexOfEnumerator("UnicodeRange");
    Q_ASSERT(index >= 0);

    m_rangeEnum = metaObject()->enumerator(index);

    m_workerThread.reset(new QThread);

    m_worker = new DistanceFieldModelWorker;
    m_worker->moveToThread(m_workerThread.data());
    connect(m_workerThread.data(), &QThread::finished,
            m_worker, &QObject::deleteLater);

    connect(m_worker, &DistanceFieldModelWorker::fontLoaded,
            this, &DistanceFieldModel::startGeneration);
    connect(m_worker, &DistanceFieldModelWorker::fontLoaded,
            this, &DistanceFieldModel::reserveSpace);
    connect(m_worker, &DistanceFieldModelWorker::distanceFieldGenerated,
            this, &DistanceFieldModel::addDistanceField);
    connect(m_worker, &DistanceFieldModelWorker::fontGenerated,
            this, &DistanceFieldModel::stopGeneration);
    connect(m_worker, &DistanceFieldModelWorker::distanceFieldGenerated,
            this, &DistanceFieldModel::distanceFieldGenerated);
    connect(m_worker, &DistanceFieldModelWorker::error,
            this, &DistanceFieldModel::error);

    m_workerThread->start();
}

DistanceFieldModel::~DistanceFieldModel()
{
    m_workerThread->quit();
    m_workerThread->wait();
}

QVariant DistanceFieldModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    return QVariant();
}

int DistanceFieldModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return m_glyphCount;
}

QVariant DistanceFieldModel::data(const QModelIndex &index, int role) const
{
    static QPixmap defaultImage;
    if (defaultImage.isNull()) {
        defaultImage = QPixmap(64, 64);
        defaultImage.fill(Qt::white);
    }

    if (!index.isValid())
        return QVariant();

    if (role == Qt::DecorationRole) {
        if (index.row() < m_distanceFields.size()) {
            return QPixmap::fromImage(m_distanceFields.at(index.row()).scaled(64, 64));
        } else {
            return defaultImage;
        }

    }

    return QVariant();
}

void DistanceFieldModel::setFont(const QString &fileName)
{
    QMetaObject::invokeMethod(m_worker,
                              [this, fileName] { m_worker->loadFont(fileName); },
                              Qt::QueuedConnection);
}

void DistanceFieldModel::reserveSpace(quint16 glyphCount,
                                      bool doubleResolution,
                                      qreal pixelSize)
{
    beginResetModel();
    m_glyphsPerUnicodeRange.clear();
    m_distanceFields.clear();
    m_glyphCount = glyphCount;
    if (glyphCount > 0)
        m_distanceFields.reserve(glyphCount);
    endResetModel();

    m_doubleGlyphResolution = doubleResolution;
    m_pixelSize = pixelSize;

    QMetaObject::invokeMethod(m_worker,
                              [this] { m_worker->generateOneDistanceField(); },
                              Qt::QueuedConnection);
}

DistanceFieldModel::UnicodeRange DistanceFieldModel::unicodeRangeForUcs4(quint32 ucs4) const
{
    int index = metaObject()->indexOfEnumerator("UnicodeRange");
    Q_ASSERT(index >= 0);

    QMetaEnum range = metaObject()->enumerator(index);
    for (int i = 0; i < range.keyCount() - 1; ++i) {
        int rangeStart = range.value(i);
        int rangeEnd = range.value(i + 1);
        if (quint32(rangeStart) <= ucs4 && quint32(rangeEnd) >= ucs4)
            return UnicodeRange(rangeStart);
    }

    return Other;
}

QList<DistanceFieldModel::UnicodeRange> DistanceFieldModel::unicodeRanges() const
{
    return m_glyphsPerUnicodeRange.uniqueKeys();
}

QList<glyph_t> DistanceFieldModel::glyphIndexesForUnicodeRange(UnicodeRange range) const
{
    return m_glyphsPerUnicodeRange.values(range);
}

QString DistanceFieldModel::nameForUnicodeRange(UnicodeRange range) const
{
    return QString::fromLatin1(m_rangeEnum.valueToKey(int(range)));
}

void DistanceFieldModel::addDistanceField(const QImage &distanceField,
                                          const QPainterPath &path,
                                          glyph_t glyphId,
                                          quint32 ucs4)
{
    if (glyphId >= quint16(m_distanceFields.size()))
        m_distanceFields.resize(glyphId + 1);
    m_distanceFields[glyphId] = distanceField;
    if (glyphId >= quint16(m_paths.size()))
        m_paths.resize(glyphId + 1);
    m_paths[glyphId] = path;

    if (ucs4 != 0) {
        UnicodeRange range = unicodeRangeForUcs4(ucs4);
        m_glyphsPerUnicodeRange.insertMulti(range, glyphId);
        m_glyphsPerUcs4.insert(ucs4, glyphId);
    }

    emit dataChanged(createIndex(glyphId, 0), createIndex(glyphId, 0));

    QMetaObject::invokeMethod(m_worker,
                             [this] { m_worker->generateOneDistanceField(); },
                             Qt::QueuedConnection);
}

glyph_t DistanceFieldModel::glyphIndexForUcs4(quint32 ucs4) const
{
    return m_glyphsPerUcs4.value(ucs4);
}

QT_END_NAMESPACE
