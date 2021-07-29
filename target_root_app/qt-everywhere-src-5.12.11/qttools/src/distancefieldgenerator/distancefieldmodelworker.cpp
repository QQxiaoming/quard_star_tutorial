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

#include "distancefieldmodelworker.h"

#include "distancefieldmodel.h"
#include <qendian.h>
#include <QtGui/private/qdistancefield_p.h>

QT_BEGIN_NAMESPACE

#   pragma pack(1)
struct MaxpHeader
{
    quint32 version;
    quint16 numGlyphs;
};

struct CmapHeader {
    quint16 version;
    quint16 numTables;
};

struct CmapEncodingRecord {
    quint16 platformId;
    quint16 encodingId;
    quint32 offset;
};

struct CmapSubtable
{
    quint16 format;
    quint16 length;
    quint16 language;
};

struct CmapSubtable0 : public CmapSubtable
{
    quint8 glyphIdArray[256];
};

struct CmapSubtable4 : public CmapSubtable
{
    quint16 segCountX2;
    quint16 searchRange;
    quint16 entrySelector;
    quint16 rangeShift;
};

struct CmapSubtable6 : public CmapSubtable
{
    quint16 firstCode;
    quint16 entryCount;
};

struct CmapSubtable10
{
    quint32 format;
    quint32 length;
    quint32 language;
    quint32 startCharCode;
    quint32 numChars;
};

struct CmapSubtable12
{
    quint16 format;
    quint16 reserved;
    quint32 length;
    quint32 language;
    quint32 numGroups;
};

struct SequentialMapGroup
{
    quint32 startCharCode;
    quint32 endCharCode;
    quint32 glyphIndex;
};

#   pragma pack()

DistanceFieldModelWorker::DistanceFieldModelWorker(QObject *parent)
    : QObject(parent)
    , m_glyphCount(0)
    , m_nextGlyphId(0)
    , m_doubleGlyphResolution(false)
{
}

template <typename T>
static void readCmapSubtable(DistanceFieldModelWorker *worker, const QByteArray &cmap, quint32 tableOffset, quint16 format)
{
    if (uint(cmap.size()) < tableOffset + sizeof(T)) {
        emit worker->error(QObject::tr("End of file when reading subtable of format '%1'").arg(format));
        return;
    }

    const T *subtable = reinterpret_cast<const T *>(cmap.constData() + tableOffset);
    quint16 length = qFromBigEndian(subtable->length);
    if (uint(cmap.size()) < tableOffset + length) {
        emit worker->error(QObject::tr("Corrupt data found when reading subtable of format '%1'. Table offset: %2. Length: %3. Cmap length: %4.")
                           .arg(format).arg(tableOffset).arg(length).arg(cmap.size()));
        return;
    }

    const void *end = cmap.constData() + tableOffset + length;
    worker->readCmapSubtable(subtable, end);
}

void DistanceFieldModelWorker::readCmapSubtable(const CmapSubtable0 *subtable, const void *end)
{
    Q_UNUSED(end); // Already checked for length
    for (int i = 0; i < 256; ++i)
        m_cmapping[glyph_t(subtable->glyphIdArray[i])] = i;
}

void DistanceFieldModelWorker::readCmapSubtable(const CmapSubtable4 *subtable, const void *end)
{
    quint16 segCount = qFromBigEndian(subtable->segCountX2) / 2;
    const quint16 *endCodes = reinterpret_cast<const quint16 *>(subtable + 1);
    const quint16 *startCodes = endCodes + segCount + 1; // endCodes[segCount] + reservedPad
    const qint16 *idDeltas = reinterpret_cast<const qint16 *>(startCodes + segCount);
    const quint16 *idRangeOffsets = reinterpret_cast<const quint16 *>(idDeltas + segCount);
    const quint16 *glyphIdArray = idRangeOffsets + segCount;
    if (glyphIdArray > end) {
        emit error(tr("End of cmap table reached when parsing subtable format '4'"));
        return;
    }

    for (int i = 0; i < segCount - 1; ++i) { // Last entry in arrays is the sentinel
        quint16 startCode = qFromBigEndian(startCodes[i]);
        quint16 endCode = qFromBigEndian(endCodes[i]);
        quint16 rangeOffset = qFromBigEndian(idRangeOffsets[i]);

        for (quint16 c = startCode; c <= endCode; ++c) {
            if (rangeOffset != 0) {
                const quint16 *glyphIndex = (idRangeOffsets + i) + (c - startCode) + rangeOffset / 2;
                if (glyphIndex + 1 > end) {
                    emit error(tr("End of cmap, subtable format '4', reached when fetching character '%1' in range [%2, %3]").arg(c).arg(startCode).arg(endCode));
                    return;
                }

                m_cmapping[glyph_t(qFromBigEndian(*glyphIndex))] = quint32(c);
            } else {
                quint16 idDelta = qFromBigEndian(idDeltas[i]);
                m_cmapping[glyph_t((idDelta + c) % 65536)] = quint32(c);
            }
        }

    }
}

void DistanceFieldModelWorker::readCmapSubtable(const CmapSubtable6 *subtable, const void *end)
{
    quint16 entryCount = qFromBigEndian(subtable->entryCount);
    const quint16 *glyphIndexes = reinterpret_cast<const quint16 *>(subtable + 1);
    if (glyphIndexes + entryCount > end) {
        emit error(tr("End of cmap reached while parsing subtable format '6'"));
        return;
    }

    quint16 firstCode = qFromBigEndian(subtable->firstCode);
    for (quint16 i = 0; i < entryCount; ++i)
        m_cmapping[glyph_t(qFromBigEndian(glyphIndexes[i]))] = firstCode + i;
}

void DistanceFieldModelWorker::readCmapSubtable(const CmapSubtable10 *subtable, const void *end)
{
    quint32 numChars = qFromBigEndian(subtable->numChars);
    const quint16 *glyphs = reinterpret_cast<const quint16 *>(subtable + 1);
    if (glyphs + numChars > end) {
        emit error(tr("End of cmap reached while parsing subtable of format '10'"));
        return;
    }

    quint32 startCharCode = qFromBigEndian(subtable->startCharCode);
    for (quint32 i = 0; i < numChars; ++i)
        m_cmapping[glyph_t(qFromBigEndian(glyphs[i]))] = startCharCode + i;
}

void DistanceFieldModelWorker::readCmapSubtable(const CmapSubtable12 *subtable, const void *end)
{
    quint32 numGroups = qFromBigEndian(subtable->numGroups);
    const SequentialMapGroup *sequentialMapGroups = reinterpret_cast<const SequentialMapGroup *>(subtable + 1);
    if (sequentialMapGroups + numGroups > end) {
        emit error(tr("End of cmap reached while parsing subtable of format '12'"));
        return;
    }

    for (quint32 i = 0; i < numGroups; ++i) {
        quint32 startCharCode = qFromBigEndian(sequentialMapGroups[i].startCharCode);
        quint32 endCharCode = qFromBigEndian(sequentialMapGroups[i].endCharCode);
        quint32 startGlyphIndex = qFromBigEndian(sequentialMapGroups[i].glyphIndex);

        for (quint32 j = 0; j < endCharCode - startCharCode + 1; ++j)
            m_cmapping[glyph_t(startGlyphIndex + j)] = startCharCode + j;
    }
}

void DistanceFieldModelWorker::readCmap()
{
    if (m_font.isValid()) {
        QByteArray cmap = m_font.fontTable("cmap");
        if (uint(cmap.size()) < sizeof(CmapHeader)) {
            emit error(tr("Invalid cmap table. No header."));
            return;
        }

        const CmapHeader *header = reinterpret_cast<const CmapHeader *>(cmap.constData());
        quint16 numTables = qFromBigEndian(header->numTables);

        if (uint(cmap.size()) < sizeof(CmapHeader) + numTables * sizeof(CmapEncodingRecord)) {
            emit error(tr("Invalid cmap table. No space for %1 encoding records.").arg(numTables));
            return;
        }

        // Support the same encodings as macOS (and same order of prefernece), since this should
        // cover most fonts
        static quint32 encodingPreferenceOrder[] =
        {
            quint32(0) << 16 | 4, // Unicode 2.0 +
            quint32(0) << 16 | 3, // Unicode 2.0 BMP
            quint32(0) << 16 | 1, // Unicode 1.1
            quint32(3) << 16 | 10, // Windows, UCS-4
            quint32(3) << 16 | 1, // Windows, UCS-2
            quint32(0)
        };

        QHash<quint32, const CmapEncodingRecord *> encodingRecords;
        {
            const CmapEncodingRecord *encodingRecord = reinterpret_cast<const CmapEncodingRecord *>(cmap.constData() + sizeof(CmapHeader));
            while (numTables-- > 0) {
                quint32 encoding = quint32(qFromBigEndian(encodingRecord->platformId)) << 16 | qFromBigEndian(encodingRecord->encodingId);
                encodingRecords[encoding] = encodingRecord++;
            }
        }

        // Find the first subtable we support in order of preference
        for (int i = 0; encodingPreferenceOrder[i] != 0; ++i) {
            const CmapEncodingRecord *encodingRecord = encodingRecords.value(encodingPreferenceOrder[i], nullptr);
            if (encodingRecord != nullptr) {
                quint32 offset = qFromBigEndian(encodingRecord->offset);
                if (uint(cmap.size()) < offset + sizeof(quint16)) {
                    emit error(tr("Invalid offset '%1' in cmap").arg(offset));
                    return;
                }

                quint16 format = qFromBigEndian(*reinterpret_cast<const quint16 *>(cmap.constData() + offset));
                switch (format) {
                case 0:
                    ::readCmapSubtable<CmapSubtable0>(this, cmap, offset, format);
                    return;
                case 4:
                    ::readCmapSubtable<CmapSubtable4>(this, cmap, offset, format);
                    return;
                case 6:
                    ::readCmapSubtable<CmapSubtable6>(this, cmap, offset, format);
                    return;
                case 10:
                    ::readCmapSubtable<CmapSubtable10>(this, cmap, offset, format);
                    return;
                case 12:
                    ::readCmapSubtable<CmapSubtable12>(this, cmap, offset, format);
                    return;
                default:
                    qWarning() << tr("Unsupported cmap subtable format '%1'").arg(format);
                };
            }
        }

        emit error(tr("No suitable cmap subtable found"));
    }
}

void DistanceFieldModelWorker::readGlyphCount()
{
    m_nextGlyphId = 0;
    m_glyphCount = 0;
    if (m_font.isValid()) {
        QByteArray maxp = m_font.fontTable("maxp");
        if (uint(maxp.size()) >= sizeof(MaxpHeader)) {
            const MaxpHeader *header = reinterpret_cast<const MaxpHeader *>(maxp.constData());
            m_glyphCount = qFromBigEndian(header->numGlyphs);
        }
    }

    m_doubleGlyphResolution = qt_fontHasNarrowOutlines(m_font) && m_glyphCount < QT_DISTANCEFIELD_HIGHGLYPHCOUNT();
}

void DistanceFieldModelWorker::loadFont(const QString &fileName)
{
    m_font = QRawFont(fileName, 64);
    if (!m_font.isValid())
        emit error(tr("File '%1' is not a valid font file.").arg(fileName));

    readGlyphCount();
    readCmap();

    qreal pixelSize = QT_DISTANCEFIELD_BASEFONTSIZE(m_doubleGlyphResolution) * QT_DISTANCEFIELD_SCALE(m_doubleGlyphResolution);
    m_font.setPixelSize(pixelSize);

    emit fontLoaded(m_glyphCount,
                    m_doubleGlyphResolution,
                    pixelSize);
}

void DistanceFieldModelWorker::generateOneDistanceField()
{
    Q_ASSERT(m_nextGlyphId <= m_glyphCount);

    if (m_nextGlyphId == m_glyphCount) {
        emit fontGenerated();
        return;
    }

    QPainterPath path = m_font.pathForGlyph(m_nextGlyphId);
    QDistanceField distanceField(path, m_nextGlyphId, m_doubleGlyphResolution);
    emit distanceFieldGenerated(distanceField.toImage(QImage::Format_Alpha8),
                                path,
                                m_nextGlyphId,
                                m_cmapping.value(m_nextGlyphId));

    m_nextGlyphId++;
}

QT_END_NAMESPACE
